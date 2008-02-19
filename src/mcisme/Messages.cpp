
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

const int MAX_MSG_LATIN_LEN = 600; //160;
const int MAX_MSG_RUS_LEN = 256;
const int MAX_MSG_LENS[2] = {MAX_MSG_LATIN_LEN, MAX_MSG_RUS_LEN};

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

void MessageFormatter::formatMessage(const AbntAddr& abnt, const vector<MCEvent>& mc_events, uint8_t start_from, vector<MCEventOut>& for_send, const std::string& smscAddress, int timeOffset/*=0*/)
{
  if (mc_events.size() <= 0) return;

  std::string toAbnt = abnt.getText();
  ContextEnvironment ctx;
  const std::string unknownCaller = formatter->getUnknownCaller();
  OutputFormatter*  multiFormatter = formatter->getMultiFormatter();
  OutputFormatter*  singleFormatter = formatter->getSingleFormatter();
  size_t	mc_events_count = mc_events.size();
  int hibit=0;

  if(formatter->isGroupping())
  {
//    Hash<bool>	callers;
    int total = 0;
//    for (int i = start_from; i < mc_events_count; i++)
    {
      int	count = 0;
      int i = start_from;
      time_t convertedTime = 0;
      AbntAddr from(&(mc_events[i].caller));
      const char* fromStr = (from.getText().length() > 0) ? from.getText().c_str():UNKNOWN_CALLER;
      if (fromStr == UNKNOWN_CALLER || (strcmp(fromStr, UNKNOWN_CALLER) == 0))
        fromStr = unknownCaller.c_str();
//      if(!callers.Exists(fromStr))
      {
//        callers.Insert(fromStr, true);
        convertedTime = mc_events[i].dt + timeOffset; //timeOffset*3600;
        count++;

        std::vector<MCEvent> srcEventLst;
        srcEventLst.push_back(mc_events[i]);

        for(int j = i + 1; j < mc_events_count; j++)
        {
          AbntAddr test(&(mc_events[j].caller));
          const char* testStr = (test.getText().length() > 0) ? test.getText().c_str():UNKNOWN_CALLER;
          if (testStr == UNKNOWN_CALLER || (strcmp(testStr, UNKNOWN_CALLER) == 0))
            testStr = unknownCaller.c_str();
          if(0 == strcmp(testStr, fromStr))
          {
            time_t t = mc_events[j].dt + timeOffset; //*3600;
            if(t > convertedTime) 
              convertedTime = t;
            srcEventLst.push_back(mc_events[j]);
            count++;
          }
        }
        total += count;

        InformGetAdapter info_adapter(toAbnt, fromStr, count, convertedTime);
        std::string rows = "";
        if( count > 1 ) multiFormatter->format(rows, info_adapter, ctx);
        else singleFormatter->format(rows, info_adapter, ctx);

        OutputFormatter*  messageFormatter = formatter->getMessageFormatter();
        MessageGetAdapter msg_adapter(toAbnt, rows, total);
        std::string report_msg_for_client;
        messageFormatter->format(report_msg_for_client, msg_adapter, ctx);
        hibit = hasHighBit(report_msg_for_client.c_str(), report_msg_for_client.length()); 

        std::string messageOriginatingAddress;
        if ( !strcmp(fromStr, unknownCaller.c_str()) )
          messageOriginatingAddress = smscAddress;
        else
          messageOriginatingAddress = fromStr;
        if(report_msg_for_client.length() < MAX_MSG_LENS[hibit]) {
          MCEventOut outEvent(messageOriginatingAddress, report_msg_for_client);
          outEvent.srcEvents = srcEventLst;
          for_send.push_back(outEvent);
        } else {
          if(i == start_from)
          {
            // обрезать смску.
            report_msg_for_client.resize(MAX_MSG_LENS[hibit]);
            MCEventOut outEvent(messageOriginatingAddress, report_msg_for_client);
            outEvent.srcEvents.assign(1, mc_events[i]);
            for_send.push_back(outEvent);
          }
//          break;
        }
      }
    }
//    callers.Empty();
  }
  else
  {
    for (int i = start_from; i < mc_events_count; i++)
    {
      AbntAddr from(&(mc_events[i].caller));
      const char* fromStr = (from.getText().length() > 0) ? from.getText().c_str():UNKNOWN_CALLER;
      if (fromStr == UNKNOWN_CALLER || (strcmp(fromStr, UNKNOWN_CALLER) == 0))
        fromStr = unknownCaller.c_str();
      time_t convertedTime = mc_events[i].dt + timeOffset;//*3600;

      InformGetAdapter info_adapter(toAbnt, fromStr, 1, convertedTime);
      std::string rows = "";
      singleFormatter->format(rows, info_adapter, ctx);

      OutputFormatter*  messageFormatter = formatter->getMessageFormatter();
      MessageGetAdapter msg_adapter(toAbnt, rows, i+1);
      std::string report_msg_for_client;
      messageFormatter->format(report_msg_for_client, msg_adapter, ctx);
      hibit = hasHighBit(report_msg_for_client.c_str(), report_msg_for_client.length()); 
      std::string messageOriginatingAddress;
      if ( !strcmp(fromStr, unknownCaller.c_str()) )
        messageOriginatingAddress = smscAddress;
      else
        messageOriginatingAddress = fromStr;
      MCEventOut outEvent(messageOriginatingAddress, report_msg_for_client);
      outEvent.srcEvents.push_back(mc_events[i]);
      if(report_msg_for_client.length() < MAX_MSG_LENS[hibit])
        for_send.push_back(outEvent);
      else
      {
        if(i == start_from)
        {
          // обрезать смску.
          report_msg_for_client.resize(MAX_MSG_LENS[hibit]);
          MCEventOut outEvent(messageOriginatingAddress, report_msg_for_client);
          outEvent.srcEvents.assign(1, mc_events[i]);
          for_send.push_back(outEvent);
        }
        break;
      }
    }
  }
}

void MessageFormatter::formatMessage(Message& message, const AbntAddr& abnt, const vector<MCEvent>& mc_events, uint8_t start_from, vector<MCEvent>& for_send, int timeOffset/*=0*/)
{
  message.message = "";
  if (mc_events.size() <= 0) return;

  std::string test_msg;
  std::string rows = "";
  std::string toAbnt = abnt.getText();
  ContextEnvironment ctx;
  const std::string unknownCaller = formatter->getUnknownCaller();
  OutputFormatter*  multiFormatter = formatter->getMultiFormatter();
  OutputFormatter*  singleFormatter = formatter->getSingleFormatter();
  size_t	mc_events_count = mc_events.size();
  int hibit=0;

  if(formatter->isGroupping())
  {
    Hash<bool>	callers;
    int total = 0;
    for (int i = start_from; i < mc_events_count; i++)
    {
      int	count = 0;
      time_t convertedTime = 0;
      AbntAddr from(&(mc_events[i].caller));
      const char* fromStr = (from.getText().length() > 0) ? from.getText().c_str():UNKNOWN_CALLER;
      if (fromStr == UNKNOWN_CALLER || (strcmp(fromStr, UNKNOWN_CALLER) == 0))
        fromStr = unknownCaller.c_str();
      if(!callers.Exists(fromStr))
      {
        callers.Insert(fromStr, true);
        convertedTime = mc_events[i].dt + timeOffset; //timeOffset*3600;
        for_send.push_back(mc_events[i]);
        count++;

        for(int j = i + 1; j < mc_events_count; j++)
        {
          AbntAddr test(&(mc_events[j].caller));
          const char* testStr = (test.getText().length() > 0) ? test.getText().c_str():UNKNOWN_CALLER;
          if (testStr == UNKNOWN_CALLER || (strcmp(testStr, UNKNOWN_CALLER) == 0))
            testStr = unknownCaller.c_str();
          if(0 == strcmp(testStr, fromStr))
          {
            time_t t = mc_events[j].dt + timeOffset; //*3600;
            if(t > convertedTime) 
              convertedTime = t;
            for_send.push_back(mc_events[j]);
            count++;
          }
        }
        total += count;
        InformGetAdapter info_adapter(toAbnt, fromStr, count, convertedTime);
        if( count > 1 ) multiFormatter->format(rows, info_adapter, ctx);
        else singleFormatter->format(rows, info_adapter, ctx);

        OutputFormatter*  messageFormatter = formatter->getMessageFormatter();
        MessageGetAdapter msg_adapter(toAbnt, rows, total);
        messageFormatter->format(test_msg, msg_adapter, ctx);
        hibit = hasHighBit(test_msg.c_str(), test_msg.length()); 
        if(test_msg.length() < MAX_MSG_LENS[hibit])
        {
          message.message = test_msg;
          test_msg="";
        }
        else
        {
          if(i == start_from)
          {	
            // обрезать смску.
            message.message = test_msg;
            message.message.resize(MAX_MSG_LENS[hibit]);
          }
          else
            for_send.erase(for_send.end()-count, for_send.end());
          break;
        }
      }
    }
    callers.Empty();
  }
  else
  {
    for (int i = start_from; i < mc_events_count; i++)
    {
      AbntAddr from(&(mc_events[i].caller));
      const char* fromStr = (from.getText().length() > 0) ? from.getText().c_str():UNKNOWN_CALLER;
      if (fromStr == UNKNOWN_CALLER || (strcmp(fromStr, UNKNOWN_CALLER) == 0))
        fromStr = unknownCaller.c_str();
      time_t convertedTime = mc_events[i].dt + timeOffset;//*3600;

      InformGetAdapter info_adapter(toAbnt, fromStr, 1, convertedTime);
      singleFormatter->format(rows, info_adapter, ctx);

      OutputFormatter*  messageFormatter = formatter->getMessageFormatter();
      MessageGetAdapter msg_adapter(toAbnt, rows, i+1);
      messageFormatter->format(test_msg, msg_adapter, ctx);
      hibit = hasHighBit(test_msg.c_str(), test_msg.length()); 
      if(test_msg.length() < MAX_MSG_LENS[hibit])
      {
        message.message = test_msg;
        test_msg="";
        for_send.push_back(mc_events[i]);
      }
      else
      {
        if(i == start_from)
        {	
          // обрезать смску.
          message.message = test_msg;
          message.message.resize(MAX_MSG_LENS[hibit]);
          for_send.push_back(mc_events[i]);
        }
        break;
      }
    }
  }
}


void MessageFormatter::addBanner(Message& message, const string& banner)
{
  if( (message.abonent == "+79139139704") || (message.abonent == "+79137141001") || (message.abonent == "+79139419998") || (message.abonent == "+79139064438")) message.message += "Privet ot MikeR!"; else
	
    message.message += " " + banner;
  int hibit = hasHighBit(message.message.c_str(), message.message.length()); 
  if(message.message.length() > MAX_MSG_LENS[hibit])
    message.message.resize(MAX_MSG_LENS[hibit]);
}

}}
