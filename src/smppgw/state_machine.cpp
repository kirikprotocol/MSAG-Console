#include "util/debug.h"
#include "smppgw/smsc.hpp"
#include "smppgw/state_machine.hpp"
#include <exception>
#include "system/common/rescheduler.hpp"
#include "profiler/profiler.hpp"
#include "util/recoder/recode_dll.h"
#include "core/buffers/Hash.hpp"
#include "util/smstext.h"
#include "logger/Logger.h"
#include "util/regexp/RegExp.hpp"
#include "core/synchronization/Mutex.hpp"
#include "system/status.h"
#include "resourcemanager/ResourceManager.hpp"
#include "util/udh.hpp"
#include <utility>
#include <list>
#include <map>
#include <set>
#include "smppgw/gwsme.hpp"


// строчка по русски, что б сработал autodetect :)

namespace smsc{
namespace smppgw{

using namespace smsc::smeman;
using namespace smsc::sms;
using namespace smsc::system::StateTypeValue;
using namespace smsc::smpp;
using namespace util;
using namespace smsc::resourcemanager;
using std::exception;
using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
using namespace smsc::core::synchronization;

using smsc::system::Task;

using smsc::router::TrafficRules;

using smsc::system::StateType;
using smsc::system::Tuple;

class TransactionMonitor{
public:

  enum TrSmsStatus{
    trDeniedBase=0x100000,
    trDeniedByRule,
    trDeniedByLimit,
    trDeniedByUssdLimit,

    trInvalid=0x1000,

    trSmeBase=0x200000,
    trSmeGenerated,

    trSmeAnswerEnd,

    trSmeUssdInit,
    trSmeUssdCont,
    trSmeUssdEnd,
    trSmeUssdInvalid=trSmeBase|trInvalid,

    trScBase=0x400000,
    trScRequest,

    trScUssdInit,
    trScUssdCont,
    trScUssdEnd,
    trScInvalid=trScBase|trInvalid
  };

  static const char* getStatusName(TrSmsStatus st)
  {
    switch(st)
    {
#define TMON_NAME_TO_STR(x) case x:return #x;
    TMON_NAME_TO_STR(trDeniedByRule)
    TMON_NAME_TO_STR(trDeniedByLimit)
    TMON_NAME_TO_STR(trDeniedByUssdLimit)

    TMON_NAME_TO_STR(trSmeGenerated)

    TMON_NAME_TO_STR(trSmeAnswerEnd)

    TMON_NAME_TO_STR(trSmeUssdInit)
    TMON_NAME_TO_STR(trSmeUssdCont)
    TMON_NAME_TO_STR(trSmeUssdEnd)
    TMON_NAME_TO_STR(trSmeUssdInvalid)

    TMON_NAME_TO_STR(trScRequest)

    TMON_NAME_TO_STR(trScUssdInit)
    TMON_NAME_TO_STR(trScUssdCont)
    TMON_NAME_TO_STR(trScUssdEnd)
    TMON_NAME_TO_STR(trScInvalid)
    }
    return "Unknown";
  }

  TrSmsStatus getS2CSmsStatus(SMS& sms,TrafficRules& r)
  {
    MutexGuard mg(mtx);

    using namespace smsc::smpp::UssdServiceOpValue;

    TransactionMap::iterator i=trMap.find(makeKey(sms));

    if(i==trMap.end())
    {
      __trace__("TMon: getS2C - not found in trMap");
      if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
      {
        int sop=sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP);
        if(sop==USSR_REQUEST)
        {
          if(r.allowInitiateUssdSession)
            return trSmeUssdInit;
          else
            return trDeniedByRule;
        }
        return trSmeUssdInvalid;
      }
    }else
    {
      __trace2__("TMon: getS2C - found in trMap: ussd=%s, dir=%s",i->second.ussd?"YES":"NO",i->second.dir==TransactionInfo::C2S?"C2S":"S2C");
      if(!i->second.ussd)
      {
        if(r.allowAnswer)
          return trSmeAnswerEnd;
        else
          return trDeniedByRule;
      }
      int sop=sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP);
      if(sop==PSSR_RESPONSE)
      {
        if(r.allowPssrResp)
        {
          if(i->second.dir==TransactionInfo::C2S)
          {
            return trScUssdEnd;
          }else
          {
            return trSmeUssdInvalid;
          }
        }else
        {
          return trDeniedByRule;
        }
      }
      if(sop==USSN_REQUEST)
      {
        if(i->second.dir==TransactionInfo::S2C)
        {
          if(r.siUssdSessionLimit!=0 && i->second.trLen>=r.siUssdSessionLimit)
          {
            return trDeniedByUssdLimit;
          }else
          {
            i->second.trLen++;
            return trSmeUssdCont;
          }
        }else
        {
          return trScUssdCont;
        }
      }
      if(sop==USSR_REQUEST)
      {
        if(r.allowUssrRequest)
        {
          if(i->second.dir==TransactionInfo::S2C)
          {
            __trace__("wtf?????");
            return trSmeUssdCont;
          }else
          {
            return trScUssdCont;
          }
        }else
        {
          return trDeniedByRule;
        }
      }
      return trSmeUssdInvalid;
    }
    if(r.limitType==TrafficRules::limitDenied)return trDeniedByRule;
    if(r.limitType!=TrafficRules::limitNoLimit)
    {
      if(getServiceCount(sms.getSourceSmeId(),r.limitType)>=r.sendLimit)
      {
        return trDeniedByLimit;
      }
    }
    return trSmeGenerated;
  }

  TrSmsStatus getC2SSmsStatus(SMS& sms,TrafficRules& r)
  {
    MutexGuard mg(mtx);
    using namespace smsc::smpp::UssdServiceOpValue;
    if(!sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
    {
      if(r.allowIncom)
      {
        return trScRequest;
      }else
      {
        return trDeniedByRule;
      }
    }
    int sop=sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP);
    TransactionMap::iterator i=trMap.find(makeKey(sms));
    if(i==trMap.end())
    {
      if(!r.allowIncom)
      {
        return trDeniedByRule;
      }
      if(sop==PSSR_INDICATION)
      {
        if(r.allowPssrResp || r.allowUssrRequest)
        {
          return trScUssdInit;
        }else
        {
          return trDeniedByRule;
        }
      }
    }else
    {
      if(sop==USSN_CONFIRM)
      {
        if(i->second.dir==TransactionInfo::S2C)
        {
          return trSmeUssdEnd;
        }else
        {
          return trScUssdCont;
        }
      }
      if(sop==USSR_CONFIRM)
      {
        if(i->second.dir==TransactionInfo::C2S)
        {
          if(r.miUssdSessionLimit!=0 && i->second.trLen==r.miUssdSessionLimit)
          {
            return trDeniedByUssdLimit;
          }else
          {
            i->second.trLen++;
            return trScUssdCont;
          }
        }else
        {
          return trSmeUssdCont;
        }
      }
    }
    return trScInvalid;
  }

  TrSmsStatus RegisterS2C(SMS& sms,time_t timeOut,TrafficRules& r)
  {
    TrSmsStatus st=getS2CSmsStatus(sms,r);
    if((st&trDeniedBase) || (st&trInvalid))return st;
    if(st==trSmeUssdInit)
    {
      __trace__("TMon: init ussd S2C");
      MutexGuard mg(mtx);
      TransactionInfo ti(true,TransactionInfo::S2C);
      pair<TransactionMap::iterator,bool> pit=trMap.insert(std::make_pair(makeKey(sms),ti));
      timeList.push_back(make_pair(time(NULL)+timeOut,pit.first));
      TimeList::iterator lit=--timeList.end();
      backMapping.insert(std::make_pair(makeKey(sms),lit));
    }
    if(st==trSmeUssdEnd || st==trSmeAnswerEnd)
    {
      MutexGuard mg(mtx);
      TransactionMap::iterator it=trMap.find(makeKey(sms));
      if(it==trMap.end())
      {
        __warning__("fuck\n");
        return st;
      }
      trMap.erase(it);
      BackMapping::iterator bit=backMapping.find(makeKey(sms));
      timeList.erase(bit->second);
      backMapping.erase(bit);
    }
    return st;
  }

  TrSmsStatus RegisterC2S(SMS& sms,time_t timeOut,TrafficRules& r)
  {
    TrSmsStatus st=getC2SSmsStatus(sms,r);
    if((st&trDeniedBase) || (st&trInvalid))return st;
    if(st==trScUssdInit || st==trScRequest)
    {
      __trace__("TMon: init ussd C2S");
      MutexGuard mg(mtx);
      TransactionInfo ti(st==trScUssdInit,TransactionInfo::C2S);
      pair<TransactionMap::iterator,bool> pit=trMap.insert(std::make_pair(makeKey(sms),ti));
      timeList.push_back(make_pair(time(NULL)+timeOut,pit.first));
      TimeList::iterator lit=--timeList.end();
      backMapping.insert(std::make_pair(makeKey(sms),lit));
    }
    if(st==trScUssdEnd)
    {
      MutexGuard mg(mtx);
      TransactionMap::iterator it=trMap.find(makeKey(sms));
      if(it==trMap.end())
      {
        __warning__("fuck\n");
        return st;
      }
      trMap.erase(it);
      BackMapping::iterator bit=backMapping.find(makeKey(sms));
      timeList.erase(bit->second);
      backMapping.erase(bit);
    }
    return st;
  }

  uint64_t getServiceCount(const char* smeId,int limitType)
  {
    //TODO: implement
    return 0;
  }

  void incServiceCount(const char* smeId)
  {
    //TODO: implement
  }

protected:

  struct TransactionInfo
  {
    enum TrDir{
      C2S,S2C
    };
    int trLen;
    TrDir dir;
    bool ussd;
    TransactionInfo(bool u,TrDir d):trLen(0),ussd(u),dir(d)
    {
    }
  };

  Mutex mtx;

  struct TransactionKey{
    Address oa;
    Address da;
    int mr;
    TransactionKey(const Address& o,const Address& d,int m):mr(m)
    {
      if(o<d)
      {
        oa=o;
        da=d;
      }else
      {
        oa=d;
        da=o;
      }
    }
    bool operator<(const TransactionKey& cmp)const
    {
      return oa<cmp.oa || (oa==cmp.oa && da<cmp.da) ||
             (oa==cmp.oa && da==cmp.da && mr<cmp.mr);
    }
  };

  typedef std::map<TransactionKey,TransactionInfo> TransactionMap;
  typedef std::pair<time_t,TransactionMap::iterator> TimeIterPair;
  typedef std::list<TimeIterPair> TimeList;
  typedef std::map<TransactionKey,TimeList::iterator> BackMapping;

  TransactionKey makeKey(SMS& sms)
  {
    return TransactionKey
           (
             sms.getOriginatingAddress(),
             sms.getDestinationAddress(),
             (int)sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE)
           );
  }

  TransactionMap trMap;
  TimeList timeList;
  BackMapping backMapping;

};

static TransactionMonitor tmon;

class RespRegistry{
public:

  void Register(int seq,int uid,int seqold,SmeProxy* sme,int to)
  {
    MutexGuard g(mtx);
    __trace2__("RR: register %d/%d",seq,uid);
    time_t now=time(NULL);
    while(timeList.size()>0 && timeList.front().first<=now)
    {
      __trace2__("RR: response timed out:%d/%d",timeList.front().second->first.first,timeList.front().second->first.second);
      backMapping.erase(backMapping.find(timeList.front().second->first));
      mapping.erase(timeList.front().second);
      timeList.pop_front();
    }
    Mapping::iterator it=mapping.insert(std::pair<const SeqUidPair,SeqSmePair>(SeqUidPair(seq,uid),SeqSmePair(seqold,sme))).first;
    TimeList::iterator lit=timeList.insert(timeList.end(),make_pair(time(NULL)+to,it));
    backMapping.insert(make_pair(make_pair(seq,uid),lit));
  }

  bool Get(int seq,int uid,int& oldseq,SmeProxy*& sme)
  {
    MutexGuard g(mtx);
    __trace2__("RR: get %d/%d",seq,uid);
    Mapping::iterator it=mapping.find(make_pair(seq,uid));
    if(it==mapping.end())return false;
    oldseq=it->second.first;
    sme=it->second.second;
    BackMapping::iterator bit=backMapping.find(make_pair(seq,uid));
    timeList.erase(bit->second);
    backMapping.erase(bit);
    mapping.erase(it);
    return true;
  }

protected:
  Mutex mtx;
  typedef std::pair<int,SmeProxy*> SeqSmePair;
  typedef std::pair<int,int> SeqUidPair;
  typedef std::map<SeqUidPair,SeqSmePair> Mapping;
  typedef std::list<std::pair<time_t,Mapping::iterator> > TimeList;
  typedef std::map<SeqUidPair,TimeList::iterator> BackMapping;
  Mapping mapping;
  BackMapping backMapping;
  TimeList timeList;
};

static RespRegistry submitRegistry;
static RespRegistry deliverRegistry;

StateMachine::StateMachine(smsc::system::EventQueue& q,
               Smsc *app):
               eq(q),
               smsc(app)

{
  smsLog = smsc::logger::Logger::getInstance("sms.trace");
}


int StateMachine::Execute()
{
  smsc::system::Tuple t;
  StateType st;
  for(;;)
  {
    eq.selectAndDequeue(t,&isStopping);
    if(isStopping)break;
    try{
      switch(t.command->cmdid)
      {
        case SUBMIT:st=submit(t);break;
        case SUBMIT_RESP:st=submitResp(t);break;
        case DELIVERY:st=delivery(t);break;
        case DELIVERY_RESP:st=deliveryResp(t);break;
        case ALERT:st=alert(t);break;
        case REPLACE:st=replace(t);break;
        case QUERY:st=query(t);break;
        case CANCEL:st=cancel(t);break;
        default:
          __warning2__("UNKNOWN COMMAND:%d",t.command->cmdid);
          st=ERROR_STATE;
          break;
      }
      __trace2__("change state for %lld to %d",t.msgId,st);
      eq.changeState(t.msgId,st);
    }catch(exception& e)
    {
      __warning2__("StateMachine::exception %s",e.what());
    }
    catch(...)
    {
      __warning__("StateMachine::unknown exception");
    }
  }
  __trace__("exit state machine");
  return 0;
}

StateType StateMachine::submit(Tuple& t)
{
  RouteInfo ri;
  SMS& sms=*t.command->get_sms();
  SmeProxy *src_proxy=t.command.getProxy();
  SmeProxy *dst_proxy;
  uint32_t dialogId =  t.command->get_dialogId();
  int dst_index;
  bool routeFound=smsc->routeSms(sms.getOriginatingAddress(),sms.getDestinationAddress(),dst_index,dst_proxy,&ri,src_proxy->getSmeIndex());
  if(!routeFound)
  {
    warn2(smsLog,"SBM: no route %s->%s",sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str());
    SmscCommand resp=SmscCommand::makeSubmitSmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::NOROUTE,
                       sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                     );
    src_proxy->putCommand(resp);
    smsc->getStatistics()->updateCounter(stat::Counters::cntRejected,src_proxy->getSystemId(),0,smsc::system::Status::NOROUTE);
    return ERROR_STATE;
  }
  if(!dst_proxy)
  {
    SmscCommand resp=SmscCommand::makeSubmitSmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::RX_T_APPN,
                       sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                     );
    src_proxy->putCommand(resp);
    return ERROR_STATE;
  }

  if(!sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
  {
    int newmr=smsc->getNextMR(sms.getDestinationAddress());
    debug2(smsLog,"submit: set mr to %d",newmr);
    sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,newmr);
  }

  TransactionMonitor::TrSmsStatus st=tmon.RegisterS2C(sms,32,ri.trafRules);

  smsc_log_info(smsLog,"SBM: trstate for sms from %s to %s=%s",sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str(),TransactionMonitor::getStatusName(st));

  if((st&TransactionMonitor::trDeniedBase) || (st&TransactionMonitor::trInvalid))
  {
    SmscCommand resp=SmscCommand::makeSubmitSmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::RX_P_APPN,
                       sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                     );
    src_proxy->putCommand(resp);
    return ERROR_STATE;
  }

  int newdid=dst_proxy->getNextSequenceNumber();

  submitRegistry.Register(newdid,dst_index,dialogId,src_proxy,dst_proxy->getPreferredTimeout());

  t.command->set_dialogId(newdid);

  bool ok=false;

  try{
    dst_proxy->putCommand(t.command);
    ok=true;
  }catch(exception& e)
  {
    warn2(smsLog,"SBM: failed to put command to dest proxy:%s",e.what());
  }catch(...)
  {
    warn2(smsLog,"SBM: failed to put command to dest proxy:%s","unknown");
  }

  if(!ok)
  {
    SmscCommand resp=SmscCommand::makeSubmitSmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::RX_T_APPN,
                       sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                     );
    src_proxy->putCommand(resp);
    return ERROR_STATE;
  }

  return ENROUTE_STATE;
}

StateType StateMachine::submitResp(Tuple& t)
{
  debug1(smsLog,"submit resp");
  SmeProxy* src_proxy=t.command.getProxy();
  SmeProxy* dst_proxy;
  int dlgId;

  if(submitRegistry.Get(t.command->get_dialogId(),src_proxy->getSmeIndex(),dlgId,dst_proxy))
  {
    debug1(smsLog,"record for submit resp found");
    t.command->set_dialogId(dlgId);
    int st=t.command->get_resp()->get_status();
    uint64_t id=0;
    if(st==0)
    {
      const char* msgid=t.command->get_resp()->get_messageId();
      sscanf(msgid,"%lld",&id);
      id<<=8;
    }
    using smsc::smeman::SmeRecord;
    SmeRecord* smerec=dynamic_cast<SmeRecord*>(src_proxy);

    if(!smerec)
    {
      warn1(smsLog,"SBMRESP: incorrect command direction (submit response to to gatewaysme)");
      return ERROR_STATE;
    }
    GatewaySme *gwsme=0;
    MutexGuard g(smerec->mutex);
    if(!smerec->deleted)
    {
      gwsme=dynamic_cast<GatewaySme*>(smerec->proxy);
    }
    if(gwsme)
    {
      id|=gwsme->getPrefix();
      char buf[64];
      sprintf(buf,"%lld",id);
      t.command->get_resp()->set_messageId(buf);
      try{
        dst_proxy->putCommand(t.command);
      }catch(...)
      {
        warn2(smsLog,"SBMRESP: failed to put command to %s",dst_proxy->getSystemId());
      }
    }else
    {
      warn1(smsLog,"SBMRESP: incorrect command direction (submit response to to gatewaysme)");
      return ERROR_STATE;
    }
  }else
  {
    debug1(smsLog,"record for submit resp not found");
  }

  return DELIVERED_STATE;
}


StateType StateMachine::delivery(smsc::system::Tuple& t)
{
  RouteInfo ri;
  SMS& sms=*t.command->get_sms();
  SmeProxy *src_proxy=t.command.getProxy();
  SmeProxy *dst_proxy;
  uint32_t dialogId =  t.command->get_dialogId();
  int dst_index;
  bool routeFound=smsc->routeSms(sms.getOriginatingAddress(),sms.getDestinationAddress(),dst_index,dst_proxy,&ri,src_proxy->getSmeIndex());
  if(!routeFound)
  {
    debug2(smsLog,"DLV: no route %s->%s",sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str());
    SmscCommand resp=SmscCommand::makeDeliverySmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::RX_R_APPN
                     );
    src_proxy->putCommand(resp);
    return ERROR_STATE;
  }

  if(!dst_proxy)
  {
    SmscCommand resp=SmscCommand::makeDeliverySmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::RX_T_APPN
                     );
    src_proxy->putCommand(resp);
    return ERROR_STATE;
  }

  if(!sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
  {
    int newmr=smsc->getNextMR(sms.getOriginatingAddress());
    debug2(smsLog,"DLV: set mr to %d",newmr);
    sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,newmr);
  }else
  {
    debug2(smsLog,"DLV: already have mr=%d",sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
  }

  TransactionMonitor::TrSmsStatus st=tmon.RegisterC2S(sms,32,ri.trafRules);

  smsc_log_info(smsLog,"DLV: trstate for sms from %s to %s=%s",sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str(),TransactionMonitor::getStatusName(st));

  if((st&TransactionMonitor::trDeniedBase) || (st&TransactionMonitor::trInvalid))
  {
    SmscCommand resp=SmscCommand::makeDeliverySmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::RX_P_APPN
                     );
    src_proxy->putCommand(resp);
    return ERROR_STATE;
  }

  int newdid=dst_proxy->getNextSequenceNumber();

  //deliverRegistry.Register(newdid,dst_index,dialogId,src_proxy,dst_proxy->getPreferredTimeout());
  deliverRegistry.Register(newdid,dst_index,dialogId,src_proxy,dst_proxy->getPreferredTimeout());

  t.command->set_dialogId(newdid);


  try{
    dst_proxy->putCommand(t.command);
    Task task(dst_proxy->getUniqueId(),newdid,0);
    smsc->tasks.createTask(task,dst_proxy->getPreferredTimeout());
  }catch(exception& e)
  {
    warn2(smsLog,"DLV: failed to put command to dest proxy:%s",e.what());
    return ERROR_STATE;
  }catch(...)
  {
    warn2(smsLog,"DLV: failed to put command to dest proxy:%s","unknown");
    return ERROR_STATE;
  }

  return ENROUTE_STATE;
}


StateType StateMachine::deliveryResp(Tuple& t)
{
  debug1(smsLog,"submit resp");
  SmeProxy* src_proxy=t.command.getProxy();
  SmeProxy* dst_proxy;
  int dlgId;

  if(deliverRegistry.Get(t.command->get_dialogId(),src_proxy->getSmeIndex(),dlgId,dst_proxy))
  {
    debug1(smsLog,"record for delivery resp found");
    t.command->set_dialogId(dlgId);
    t.command->get_resp()->set_messageId("0");
    try{
      dst_proxy->putCommand(t.command);
    }catch(...)
    {
      warn2(smsLog,"DLVRESP: failed to put command to %s",dst_proxy->getSystemId());
    }
  }else
  {
    debug1(smsLog,"record for delivery resp not found");
  }

  return DELIVERED_STATE;
}

StateType StateMachine::alert(Tuple& t)
{
  return ENROUTE_STATE;
}

StateType StateMachine::replace(Tuple& t)
{
  return ENROUTE_STATE;
}

StateType StateMachine::query(Tuple& t)
{
  return ENROUTE_STATE;
}

StateType StateMachine::cancel(Tuple& t)
{
  return ENROUTE_STATE;
}


}//system
}//smsc
