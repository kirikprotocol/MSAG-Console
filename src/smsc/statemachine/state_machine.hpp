#ifndef __SMSC_STATEMACHINE_STATE_MACHINE_HPP__
#define __SMSC_STATEMACHINE_STATE_MACHINE_HPP__

#include <exception>
#include <string>
#include <list>

#include "core/buffers/Hash.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "smsc/event_queue.h"
#include "smsc/profiler/profiler.hpp"
#include "util/templates/Formatters.h"
#include "util/regexp/RegExp.hpp"
#include "store/MessageStore.h"
#include "smsc/router/Router.hpp"

namespace smsc{

class Smsc;

namespace statemachine{

using smsc::profiler::Profile;

using namespace smsc::util::templates;

class RegExpCompilationException:public std::exception{
public:
  const char* what()const throw()
  {
    return "failed to compile directive processing regexp in state machine";
  }
};

inline FixedLengthString<64> AddrPair(const char* s1,const Address& a1,const char* s2,const Address& a2)
{
  char buf[64];
  char buf1[32];
  char buf2[32];
  if(a1==a2)
  {
    a1.toString(buf1,sizeof(buf1));
    sprintf(buf,"%s=%s",s1,buf1);
  }else
  {
    a1.toString(buf1,sizeof(buf1));
    a2.toString(buf2,sizeof(buf2));
    sprintf(buf,"%s=%s;%s=%s",s1,buf1,s2,buf2);
  }
  return buf;
}


class StateMachine:public smsc::core::threads::ThreadedTask{
public:
  StateMachine(EventQueue& q,
               smsc::store::MessageStore* st,
               smsc::Smsc *app);
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
    time_t submitDate;
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
#ifdef SMSEXTRA
  bool createCopyOnNickUsage;
#endif

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

  enum SmsCreationState{
    scsCreate,
    scsDoNotCreate,
    scsReplace
  };

protected:

  EventQueue& eq;
  smsc::store::MessageStore* store;
  smsc::Smsc *smsc;

  smsc::logger::Logger* smsLog;
  smsc::logger::Logger* perfLog;

  std::string serviceType;
  int protocolId;
  SmeSystemId smscSmeId;

  static bool StateMachine::checkSourceAddress(const std::string& pattern,const Address& src);


  static Hash<std::list<std::string> > directiveAliases;

  smsc::util::regexp::RegExp dreAck;
  smsc::util::regexp::RegExp dreNoAck;
  smsc::util::regexp::RegExp dreHide;
  smsc::util::regexp::RegExp dreUnhide;
  smsc::util::regexp::RegExp dreFlash;
  smsc::util::regexp::RegExp dreDef;
  smsc::util::regexp::RegExp dreTemplate;
  smsc::util::regexp::RegExp dreTemplateParam;
  smsc::util::regexp::RegExp dreNoTrans;
  smsc::util::regexp::RegExp dreUnknown;

  smsc::util::regexp::RegExp smeAckRx;

  StateType submit(Tuple& t);
  StateType forward(Tuple& t);
  StateType forwardChargeResp(Tuple& t);
  StateType deliveryResp(Tuple& t);
  StateType alert(Tuple& t);
  StateType replace(Tuple& t);
  StateType query(Tuple& t);
  StateType cancel(Tuple& t);
  StateType submitChargeResp(Tuple& t);

  void formatDeliver(const FormatData&,std::string& out);
  void formatFailed(const FormatData&,std::string& out);
  void formatNotify(const FormatData&,std::string& out);

  void sendFailureReport(SMS& sms,MsgIdType msgId,int state,const char* reason);
  void sendNotifyReport(SMS& sms,MsgIdType msgId,const char* reason);

  void changeSmsStateToEnroute(SMS& sms,SMSId id,const Descriptor& d,uint32_t failureCause,time_t nextTryTime,bool skipAttempt=false);

  void submitReceipt(SMS& sms,int type);

  void submitResp(Tuple&,SMS*,int);

  void finalizeSms(SMSId id,SMS& sms);

  void fullReport(SMSId msgid,SMS& sms);

  StateType DivertProcessing(Tuple& t,SMS& sms);

  void onSubmitOk(SMSId id,SMS& sms);
  void onSubmitFail(SMSId id,SMS& sms);
  void onDeliveryOk(SMSId id,SMS& sms);
  void onDeliveryFail(SMSId id,SMS& sms);
  void onUndeliverable(SMSId id,SMS& sms);
  void onForwardOk(SMSId id,SMS& sms);

  struct SbmContext
  {
    SbmContext(Tuple& argT):t(argT)
    {
      sms=0;
      fromMap=false;
      toMap=false;
      fromDistrList=false;
      noDestChange=false;
      has_route=false;
      isForwardTo=false;
      generateDeliver=false;
      needToSendResp=false;
      dialogId=-1;
      createSms=scsCreate;
    }
    SMS* sms;
    smsc::router::RoutingResult rr;
    SmeProxy* src_proxy;
    Tuple& t;
    bool fromMap;
    bool toMap;
    bool fromDistrList;
    Profile srcprof;
    Profile profile;
    bool noDestChange;
    Address dst;
    bool has_route;
    StateType rvstate;
    bool isForwardTo;
    bool generateDeliver;
    bool needToSendResp;
    bool noPartitionSms;
    SmsCreationState createSms;
    int dialogId;
  };

  bool processMerge(SbmContext& c);

  void prepareSmsDc(SMS& sms,bool defaultDcLatin1);

#ifdef SMSEXTRA
  bool ExtraProcessing(SbmContext& c);
#endif
};

}//system
}//smsc

#endif
