#include "Messages.h"
#include "sms/sms_const.h"
#include "util/smstext.h"
#include "misscall/callproc.hpp"
#include "TaskProcessor.h"

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

using smsc::util::templates::AdapterException;

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

void addBanner(std::string& message, const string& banner)
{
  if ( banner.empty() ) return;
  message += " " + banner;
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
MessageFormatter::formatMessage(const AbntAddr& called_abnt,
                                const vector<MCEvent>& mc_events,
                                MCEventOut* for_send,
                                const std::string& mciSmeAddress,
                                int timeOffset,
                                bool originatingAddressIsMciSmeAddress)
{
  if (mc_events.size() == 0) return false;

  const std::string& toAbnt = called_abnt.getText();
  ContextEnvironment ctx;
  std::string unknownCaller = formatter->getUnknownCaller();
  size_t mc_events_count = mc_events.size();

  int total = 0;

  int i=0;
  std::string formattedEvents;
  for (; i < mc_events_count; i++)
  {
    AbntAddr from(&(mc_events[i].caller));

    std::string fromAbnt = (from.getText().empty() ? unknownCaller : from.getText());
    if( mc_events[i].missCallFlags & misscall::ANTI_AON_FOR_CALLER ) fromAbnt = unknownCaller;

    uint16_t callCount = mc_events[i].callCount;

    InformGetAdapter info_adapter(toAbnt, fromAbnt, callCount, mc_events[i].dt + timeOffset);
    std::string formattedEventInfo;
    if ( callCount > 1 )
      formatter->getMultiFormatter()->format(formattedEventInfo, info_adapter, ctx);
    else
      formatter->getSingleFormatter()->format(formattedEventInfo, info_adapter, ctx);

    MessageGetAdapter msg_adapter(toAbnt, formattedEvents + formattedEventInfo,
                                  total + callCount);
    std::string report_msg_for_client;
    formatter->getMessageFormatter()->format(report_msg_for_client, msg_adapter, ctx);

    unsigned int hibit = smsc::util::hasHighBit(report_msg_for_client.c_str(), report_msg_for_client.length())?1:0;
    if ( report_msg_for_client.size() > MAX_MSG_LENS[hibit] ) {
      return true;
    }
    formattedEvents += formattedEventInfo;
    total += callCount;
    for_send->msg.message = report_msg_for_client;
    for_send->srcEvents.push_back(mc_events[i]);

    if ( !originatingAddressIsMciSmeAddress ) {
      // in this case MCISme doesn't group messages in order to send
      // result message on behalf of MCISme (i.e. original address is MCISme address)
      for_send->msg.calling_abonent = (fromAbnt == unknownCaller ? mciSmeAddress : fromAbnt);
      return true;
    } else
      for_send->msg.calling_abonent = _taskProcessor.getAddress();
  }

  return true;
}

}}
