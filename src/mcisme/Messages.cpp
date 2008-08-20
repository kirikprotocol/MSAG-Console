
#include "Messages.h"
#include <sms/sms_const.h>
#include <util/smstext.h>

namespace smsc {
namespace mcisme {

int Message::maxRowsPerMessage = 5;

const char* MSG_TEMPLATE_PARAM_ABONENT = "abonent";
const char* MSG_TEMPLATE_PARAM_CALLER  = "caller";
const char* MSG_TEMPLATE_PARAM_TOTAL   = "total";
const char* MSG_TEMPLATE_PARAM_COUNT   = "count";
const char* MSG_TEMPLATE_PARAM_DATE    = "date";
const char* MSG_TEMPLATE_PARAM_ROWS    = "rows";

static const char*  UNKNOWN_CALLER = "XXX";

const unsigned int MAX_MSG_LATIN_LEN = 600; //160;
const unsigned int MAX_MSG_RUS_LEN = 256;
const unsigned int MAX_MSG_LENS[2] = {MAX_MSG_LATIN_LEN, MAX_MSG_RUS_LEN};

void keyIsNotSupported(const char* key) throw(AdapterException) {
  throw AdapterException(Exception("Argument '%s' is not supported by message formatter", key));
}
void typeIsNotSupported(const char* type) throw(AdapterException) {
  throw AdapterException(Exception("Type '%s' is not supported by message formatter", type));
}
char* convertStrToLowerCase(const char* str)
{
  if (!str) return 0;
  char* low = new char[strlen(str)+1]; int cur = 0;
  while (*str) low[cur++] = (char)tolower(*str++);
  low[cur] = '\0';
  return low;
}
const char* findMessageTemplateKey(const char* key) throw(AdapterException)
{
  if (!key || !key[0]) throw AdapterException(Exception("Argument key is null"));
  std::auto_ptr<char> lkeyGuard(convertStrToLowerCase(key));
  const char* lkey = lkeyGuard.get();

  if      (!strcmp(MSG_TEMPLATE_PARAM_ABONENT, lkey)) return MSG_TEMPLATE_PARAM_ABONENT;
  else if (!strcmp(MSG_TEMPLATE_PARAM_CALLER,  lkey)) return MSG_TEMPLATE_PARAM_CALLER;
  else if (!strcmp(MSG_TEMPLATE_PARAM_TOTAL,   lkey)) return MSG_TEMPLATE_PARAM_TOTAL;
  else if (!strcmp(MSG_TEMPLATE_PARAM_COUNT,   lkey)) return MSG_TEMPLATE_PARAM_COUNT;
  else if (!strcmp(MSG_TEMPLATE_PARAM_DATE,    lkey)) return MSG_TEMPLATE_PARAM_DATE;
  else if (!strcmp(MSG_TEMPLATE_PARAM_ROWS,    lkey)) return MSG_TEMPLATE_PARAM_ROWS;
  else keyIsNotSupported(key); // Forces exception throwing

  return 0;
}

bool MessageFormatter::canAdd(const MissedCallEvent& event)
{
  if (!formatter->isGroupping()) return (events.Count() < Message::maxRowsPerMessage);

  const char* fromStr = (event.from.length() > 0) ? event.from.c_str():UNKNOWN_CALLER;
  return ((counters.Exists(fromStr)) ? true:(counters.GetCount() < Message::maxRowsPerMessage));
}
void MessageFormatter::addEvent(const MissedCallEvent& event)
{
  int eventsCount = events.Count();
  if (eventsCount > 0)
  {
    for (int i=0; i<eventsCount; i++) 
      if (event.time < events[i].time) { events.Insert(i, event); break; }
  }
  if (events.Count() == eventsCount) events.Push(event);

  if (formatter->isGroupping())
  {
    const char* fromStr = (event.from.length() > 0) ? event.from.c_str():UNKNOWN_CALLER;
    uint32_t* recPtr = counters.GetPtr(fromStr);
    if (recPtr) (*recPtr)++; else counters.Insert(fromStr, 1);
  }
}
void MessageFormatter::addEvent(const AbntAddr& to, const MCEvent& event)
{
  MissedCallEvent	mce;
  AbntAddr from(&(event.caller));

  mce.from = from.getText();
  mce.to = to.getText();
  mce.time = event.dt;
  mce.cause = 0;

  //	printf("MessageFormatter: addEvent %s missed call from %s\n", mce.to.c_str(), mce.from.c_str());

  addEvent(mce);
}
bool MessageFormatter::isLastFromCaller(int index)
{
  if (events.Count() <= 0 || index < 0 || index >= events.Count()) return false;
  else if (index == events.Count()-1) return true;

  const char* checkStr = (events[index].from.length() > 0) ? events[index].from.c_str():UNKNOWN_CALLER;
  for (int i=index+1; i<events.Count(); i++)
  {
    const char* fromStr = (events[i].from.length() > 0) ? events[i].from.c_str():UNKNOWN_CALLER;
    if (!strcmp(fromStr, checkStr)) return false;
  }
  return true;
}

bool
MessageFormatter::formatMessage(const AbntAddr& abnt,
                                const vector<MCEvent>& mc_events,
                                MCEventOut* for_send,
                                const std::string& mciSmeAddress,
                                int timeOffset,
                                bool originatingAddressIsMciSmeAddress)
{
  if (mc_events.size() <= 0) return false;

  const std::string& toAbnt = abnt.getText();
  ContextEnvironment ctx;
  const std::string unknownCaller = formatter->getUnknownCaller();
  size_t            mc_events_count = mc_events.size();

  int total = 0;

  int i=0;
  for (; i < mc_events_count; i++)
  {
    smsc_log_debug(logger, "MessageFormatter::formatMessage::: process next event: event idx=%d", i);
    AbntAddr from(&(mc_events[i].caller));

    const std::string& fromAbnt = (from.getText().empty() ? unknownCaller : from.getText());

    const std::string& report_msg_for_client =
      produceMessageForAbonent(toAbnt, fromAbnt, mc_events[i].callCount, mc_events[i].dt + timeOffset, &total, &ctx);

    unsigned int hibit = hasHighBit(report_msg_for_client.c_str(), report_msg_for_client.length()); 

    std::string messageOriginatingAddress;
    if ( fromAbnt == unknownCaller || originatingAddressIsMciSmeAddress )
      messageOriginatingAddress = mciSmeAddress;
    else
      messageOriginatingAddress = fromAbnt;

    if ( createMCEventOut(for_send, mc_events[i], originatingAddressIsMciSmeAddress, messageOriginatingAddress, report_msg_for_client, MAX_MSG_LENS[hibit]) ) break;
  }

  for_send->lastCallingTime = mc_events[i].dt;

  return true;
}

bool
MessageFormatter::createMCEventOut(MCEventOut* forSend,
                                   const MCEvent& eventFromStorage,
                                   bool originatingAddressIsMciSmeAddress,
                                   const std::string& messageOriginatingAddress,
                                   const std::string& reportMsgForClient,
                                   unsigned int maxMsgLen)
{
  if ( originatingAddressIsMciSmeAddress ) {
    if ( forSend->msg.length() + reportMsgForClient.length() < maxMsgLen ) {
      forSend->msg += reportMsgForClient;
      forSend->srcEvents.push_back(eventFromStorage);
      return false; // continue accumulate messages for abonent
    } else {
      if ( forSend->msg.empty() ) {
        const std::string& messageToClient = ( reportMsgForClient.length() < maxMsgLen ) ? reportMsgForClient : reportMsgForClient.substr(0, maxMsgLen);
        MCEventOut outEvent(messageOriginatingAddress, messageToClient);
        outEvent.srcEvents.push_back(eventFromStorage);
        *forSend = outEvent;
      }
      return true;
    }
  } else {
    const std::string& messageToClient = ( reportMsgForClient.length() < maxMsgLen ) ? reportMsgForClient : reportMsgForClient.substr(0, maxMsgLen);
    MCEventOut outEvent(messageOriginatingAddress, messageToClient);
    outEvent.srcEvents.push_back(eventFromStorage);
    *forSend = outEvent;

    return true;
  }
}

std::string
MessageFormatter::produceMessageForAbonent(const std::string& toAbnt,
                                           const std::string& fromAbnt,
                                           uint16_t callCount,
                                           time_t convertedTime,
                                           int* total,
                                           ContextEnvironment* ctx
                                           )
{
  smsc_log_debug(logger, "MessageFormatter::produceMessageForAbonent::: create InformGetAdapter, toAbnt=[%s], fromAbnt=[%s], callCount=%d, convertedTime=%x", toAbnt.c_str(), fromAbnt.c_str(), callCount, convertedTime);
  InformGetAdapter info_adapter(toAbnt, fromAbnt, callCount, convertedTime);
  std::string rows = "";
  if ( callCount > 1 )
    formatter->getMultiFormatter()->format(rows, info_adapter, *ctx);
  else
    formatter->getSingleFormatter()->format(rows, info_adapter, *ctx);

  smsc_log_debug(logger, "MessageFormatter::produceMessageForAbonent::: OutputFormatter::format returned rows=[%s]", rows.c_str());
  *total += callCount;

  OutputFormatter*  messageFormatter = formatter->getMessageFormatter();
  smsc_log_debug(logger, "MessageFormatter::produceMessageForAbonent::: create MessageGetAdapter: toAbnt=[%s], rows=[%s], total=[%d]", toAbnt.c_str(), rows.c_str(), *total);

  MessageGetAdapter msg_adapter(toAbnt, rows, *total);
  std::string report_msg_for_client;
  messageFormatter->format(report_msg_for_client, msg_adapter, *ctx);

  smsc_log_debug(logger, "MessageFormatter::produceMessageForAbonent::: result message for abonent report_msg_for_client=[%s]", report_msg_for_client.c_str());

  return report_msg_for_client;;
}

void MessageFormatter::addBanner(Message& message, const string& banner)
{
  if( (message.abonent == "+79139139704") || (message.abonent == "+79137141001") || (message.abonent == "+79139419998") || (message.abonent == "+79139064438")) message.message += "Privet ot MikeR!"; else
	
    message.message += " " + banner;
  int hibit = hasHighBit(message.message.c_str(), message.message.length()); 
  if(message.message.length() > MAX_MSG_LENS[hibit])
    message.message.resize(MAX_MSG_LENS[hibit]);
}

}
}
