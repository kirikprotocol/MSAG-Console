#ifndef __SMSC_SYSTEM_STATE_MACHINE_HPP__
#define __SMSC_SYSTEM_STATE_MACHINE_HPP__

#include "system/smsc.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "store/StoreManager.h"
#include "system/event_queue.h"
#include "util/templates/Formatters.h"
#include <string>
#include "profiler/profiler.hpp"
#include "core/buffers/Hash.hpp"
#include <list>
#include "util/regexp/RegExp.hpp"
#include <exception>

namespace smsc{
namespace system{

class smsc::system::Smsc;
using smsc::profiler::Profile;

using namespace smsc::util::templates;

class RegExpCompilationException:public std::exception{
public:
  const char* what()const throw()
  {
    return "failed to compile directive processing regexp in state machine";
  }
};

class StateMachine:public smsc::core::threads::ThreadedTask{
public:
  StateMachine(EventQueue& q,
               smsc::store::MessageStore* st,
               smsc::system::Smsc *app);
  virtual ~StateMachine()
  {
  }

  int Execute();

  struct FormatData{
    const char* scheme;
    const char* locale;
    const char* addr;
    const char* ddest;
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

  void processDirectives(SMS& sms,Profile& p,Profile& srcprof);

  void setReceiptInfo(const std::string& st,int pid,std::string sid)
  {
    serviceType=st;
    protocolId=pid;
    smscSmeId=sid;
  }

  static void AddDirectiveAlias(const char* dir,const char* alias)
  {
    directiveAliases[dir].push_back(alias);
  }

protected:

  EventQueue& eq;
  smsc::store::MessageStore* store;
  smsc::system::Smsc *smsc;

  smsc::logger::Logger* smsLog;

  std::string serviceType;
  int protocolId;
  std::string smscSmeId;

  static Hash<std::list<std::string> > directiveAliases;

  smsc::util::regexp::RegExp dreAck;
  smsc::util::regexp::RegExp dreNoAck;
  smsc::util::regexp::RegExp dreHide;
  smsc::util::regexp::RegExp dreUnhide;
  smsc::util::regexp::RegExp dreFlash;
  smsc::util::regexp::RegExp dreDef;
  smsc::util::regexp::RegExp dreTemplate;
  smsc::util::regexp::RegExp dreTemplateParam;
  smsc::util::regexp::RegExp dreUnknown;

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

  void changeSmsStateToEnroute(SMS& sms,SMSId id,const Descriptor& d,uint32_t failureCause,time_t nextTryTime,bool skipAttempt=false);

  void submitReceipt(SMS& sms,int type);

  void submitResp(Tuple&,SMS*,int);

  void finalizeSms(SMSId id,SMS& sms);

  StateType DivertProcessing(Tuple& t,SMS& sms);

};

}//system
}//smsc

#endif
