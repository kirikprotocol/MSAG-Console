#ifndef __SMSC_SYSTEM_STATE_MACHINE_HPP__
#define __SMSC_SYSTEM_STATE_MACHINE_HPP__

#include "core/threads/ThreadedTask.hpp"
#include "store/StoreManager.h"
#include "system/event_queue.h"
#include "util/templates/Formatters.h"
#include <string>

namespace smsc{
namespace system{

class smsc::system::Smsc;

using namespace smsc::util::templates;

class StateMachine:public smsc::core::threads::ThreadedTask{
public:
  StateMachine(EventQueue& q,
               smsc::store::MessageStore* st,
               smsc::system::Smsc *app):
               eq(q),
               store(st),
               smsc(app)
               {}
  virtual ~StateMachine()
  {
  }

  int Execute();

  struct FormatData{
    const char* scheme;
    const char* locale;
    const char* addr;
    time_t date;
    const char* msgId;
    const char* err;
    int lastResult;
    int lastResultGsm;
    const char* msc;
    void setLastResult(int errcode)
    {
      lastResult=errcode;
      if (errcode >= 1152) lastResultGsm= errcode- 1152;
      else if(errcode == 1025)  lastResultGsm=134;
      else if(errcode == 1026)  lastResultGsm=132;
      else if(errcode == 1029)  lastResultGsm=135;
      else if(errcode == 1027)  lastResultGsm=131;
      else if(errcode == 1030)  lastResultGsm=136;
      else lastResultGsm = 137;
    }
  };

  time_t rescheduleSms(SMS& sms);

  const char *taskName(){return "StateMachine";}

  time_t maxValidTime;
  Address scAddress;

protected:

  EventQueue& eq;
  smsc::store::MessageStore* store;
  smsc::system::Smsc *smsc;

  StateType submit(Tuple& t);
  StateType forward(Tuple& t);
  StateType deliveryResp(Tuple& t);
  StateType alert(Tuple& t);
  StateType replace(Tuple& t);
  StateType query(Tuple& t);
  StateType cancel(Tuple& t);

  void formatDeliver(const FormatData&,std::string& out);
  void formatFailed(const FormatData&,std::string& out);
  void formatNotify(const FormatData&,std::string& out);

  void sendFailureReport(SMS& sms,MsgIdType msgId,int state,const char* reason);
  void sendNotifyReport(SMS& sms,MsgIdType msgId,const char* reason);

  void processDirectives(SMS& sms,smsc::profiler::Profile& p);
};

};//system
};//smsc

#endif
