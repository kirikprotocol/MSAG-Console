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
#include "smppgw/billing/bill.hpp"


// строчка по русски, что б сработал autodetect :)

namespace smsc{
namespace smppgw{

using namespace smsc::smeman;
using namespace smsc::sms;
using namespace smsc::smpp;
using namespace util;
using namespace smsc::resourcemanager;
using std::exception;
using smsc::util::regexp::RegExp;
using smsc::util::regexp::SMatch;
using namespace smsc::core::synchronization;

using smsc::system::Task;

using smsc::router::TrafficRules;

using billing::GetBillingInterface;

smsc::db::DataSource* StateMachine::dataSource;

class TransactionMonitor{
public:

  enum TrSmsStatus{
    trDeniedBase=0x100000,
    trDeniedByRule,
    trDeniedByLimit,
    trDeniedByUssdLimit,
    trDeniedByBilling,
    trTransactionInProgress,

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
    TMON_NAME_TO_STR(trDeniedByBilling)
    TMON_NAME_TO_STR(trTransactionInProgress)

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
      if(getRouteCount(sms.getRouteId(),r.limitType)>=r.sendLimit)
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

  std::pair<TrSmsStatus,billing::TransactionIdType> RegisterS2C(SMS& sms,time_t timeOut,TrafficRules& r)
  {
    TrSmsStatus st=getS2CSmsStatus(sms,r);
    if((st&trDeniedBase) || (st&trInvalid))
    {
      return std::make_pair(st,billing::InvalidTransactionId);
    }
    if(st==trSmeUssdInit)
    {
      __trace__("TMon: init ussd S2C");
      MutexGuard mg(mtx);

      if(trMap.find(makeKey(sms))!=trMap.end())
      {
        return std::make_pair(trTransactionInProgress,billing::InvalidTransactionId);
      }


      TransactionInfo ti(true,TransactionInfo::S2C);
      ti.sysId=sms.getSourceSmeId();
      ti.routeId=sms.getRouteId();
      ti.trId=billing::GetBillingInterface()->BeginTransaction(sms,true);
      if(ti.trId==billing::InvalidTransactionId)
      {
        __trace__("TMon: billing denied transaction");
        return std::make_pair(trDeniedByBilling,billing::InvalidTransactionId);
      }
      pair<TransactionMap::iterator,bool> pit=trMap.insert(std::make_pair(makeKey(sms),ti));
      timeList.push_back(make_pair(time(NULL)+timeOut,pit.first));
      TimeList::iterator lit=--timeList.end();
      backMapping.insert(std::make_pair(makeKey(sms),lit));
      return std::make_pair(st,ti.trId);
    }
    if(st==trSmeUssdEnd || st==trSmeAnswerEnd)
    {
      MutexGuard mg(mtx);
      TransactionMap::iterator it=trMap.find(makeKey(sms));
      if(it==trMap.end())
      {
        __warning__("trmon: fuck, transaction not found :-/\n");
        return std::make_pair(st,billing::InvalidTransactionId);
      }

      GetBillingInterface()->CommitTransaction(it->second.trId);

      trMap.erase(it);
      BackMapping::iterator bit=backMapping.find(makeKey(sms));
      timeList.erase(bit->second);
      backMapping.erase(bit);
    }
    if(st==trSmeGenerated)
    {
      if(r.limitType!=TrafficRules::limitNoLimit)
      {
        MutexGuard mg(mtx);
        incRouteCount(sms.getRouteId());
      }
      billing::TransactionIdType trId=billing::GetBillingInterface()->BeginTransaction(sms,true);
      if(trId==billing::InvalidTransactionId)
      {
        return std::make_pair(trDeniedByBilling,billing::InvalidTransactionId);
      }
      return std::make_pair(st,trId);
    }
    return std::make_pair(st,billing::InvalidTransactionId);
  }

  TrSmsStatus RegisterC2S(SMS& sms,time_t timeOut,TrafficRules& r)
  {
    TrSmsStatus st=getC2SSmsStatus(sms,r);
    if((st&trDeniedBase) || (st&trInvalid))return st;
    if(st==trScUssdInit || st==trScRequest)
    {
      __trace__("TMon: init ussd C2S");
      MutexGuard mg(mtx);

      if(trMap.find(makeKey(sms))!=trMap.end())
      {
        return trTransactionInProgress;
      }

      TransactionInfo ti(st==trScUssdInit,TransactionInfo::C2S);
      ti.sysId=sms.getDestinationSmeId();
      ti.routeId=sms.getRouteId();
      ti.trId=GetBillingInterface()->BeginTransaction(sms,false);
      if(ti.trId==billing::InvalidTransactionId)
      {
        __trace__("TMon: billing denied transaction");
        return trDeniedByBilling;
      }
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
        __warning__("trmon: fuck, transaction not found :-/\n");
        return st;
      }
      trMap.erase(it);
      BackMapping::iterator bit=backMapping.find(makeKey(sms));
      timeList.erase(bit->second);
      backMapping.erase(bit);
    }
    return st;
  }

  void ProcessExpired(stat::IStatistics* stat)
  {
    MutexGuard mg(mtx);
    time_t now=time(NULL);
    int cnt=0;
    while(timeList.size()>0 && timeList.front().first<=now)
    {
      TransactionInfo& ti=timeList.front().second->second;
      if(ti.ussd)
      {
        if(ti.dir==TransactionInfo::S2C)
        {
          stat->updateCounter(stat::Counters::cntUssdTrFromSmeFailed,ti.sysId.c_str(),ti.routeId.c_str(),smsc::system::Status::TRANSACTIONTIMEDOUT);
        }else
        {
          stat->updateCounter(stat::Counters::cntUssdTrFromScFailed,ti.sysId.c_str(),ti.routeId.c_str(),smsc::system::Status::TRANSACTIONTIMEDOUT);
        }
      }else
      {
        stat->updateCounter(stat::Counters::cntSmsTrFailed,ti.sysId.c_str(),ti.routeId.c_str(),smsc::system::Status::TRANSACTIONTIMEDOUT);
      }
      if(ti.trId!=billing::InvalidTransactionId)
      {
        GetBillingInterface()->RollbackTransaction(ti.trId);
      }
      backMapping.erase(timeList.front().second->first);
      trMap.erase(timeList.front().second);
      timeList.pop_front();
      cnt++;
    }
    if(cnt>0)
    {
      __trace2__("TrMon: %d transactions expired",cnt);
    }
  }

  uint64_t getRouteCount(const char* rtId,int limitType)
  {
    TimeSlotCounter<> **pcnt=countersHash.GetPtr(rtId);
    if(!pcnt)
    {
      TimeSlotCounter<> *cnt;
      time_t t=time(NULL);
      int mps=1000; //max per slot
      switch(limitType)
      {
        case TrafficRules::limitDenied:
        case TrafficRules::limitNoLimit:return 0;
        case TrafficRules::limitPerHour:
          cnt=new TimeSlotCounter<>(60*60,1000);
          t-=60*60;
          break;
        case TrafficRules::limitPerDay:
          cnt=new TimeSlotCounter<>(60*60*24,60000);
          t-=60*60*24;
          break;
        case TrafficRules::limitPerWeek:
          cnt=new TimeSlotCounter<>(60*60*24*7,600000);
          t-=60*60*24*7;
          break;
        case TrafficRules::limitPerMonth:
          cnt=new TimeSlotCounter<>(60*60*24*30,600000);
          t-=60*60*24*30;
        break;
      }
      tm tmCT;
      localtime_r(&t, &tmCT);
      uint32_t period= (tmCT.tm_year+1900)*1000000+(tmCT.tm_mon+1)*10000+
                       (tmCT.tm_mday)*100+tmCT.tm_hour;
      int counterValue=0;

      smsc::db::Connection* connection=StateMachine::dataSource->getConnection();
      if(connection)
      {
        try{
          using std::auto_ptr;
          using namespace smsc::db;
          auto_ptr<Statement> getCntStatement;
          const char* sql="select sum(accepted) from smppgw_stat_route where routeId=:rtid and period>=:per";
          getCntStatement=auto_ptr<Statement>(connection->createStatement(sql));
          getCntStatement->setString(1,rtId);
          getCntStatement->setInt32(2,period);
          auto_ptr<ResultSet> rs(getCntStatement->executeQuery());
          if(rs.get())
          {
            if(rs->fetchNext())
            {
              counterValue=rs->getInt32(1);
            }
          }
        }catch(...)
        {
          connection->rollback();
        }
        StateMachine::dataSource->freeConnection(connection);
      }
      countersHash.Insert(rtId,cnt);
      cnt->IncEven(counterValue);
      return counterValue;
    }else
    {
      return (*pcnt)->Get();
    }
  }

  void incRouteCount(const char* rtId)
  {
    TimeSlotCounter<> **cnt=countersHash.GetPtr(rtId);
    if(cnt)(*cnt)->Inc();
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
    std::string sysId;
    std::string routeId;
    smsc::smppgw::billing::TransactionIdType trId;
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

  Hash<TimeSlotCounter<>*> countersHash;

};

static TransactionMonitor tmon;

class RespRegistry{
public:

  struct RegistryData{
    SmeProxy* sme;
    int seq;
    std::string smeId;
    std::string routeId;
    billing::TransactionIdType trId;
    RegistryData():sme(0),seq(0),trId(billing::InvalidTransactionId){}
    RegistryData(SmeProxy* smeVal,int seqVal,const std::string& smeIdVal,const std::string& routeIdVal,billing::TransactionIdType trIdVal=billing::InvalidTransactionId):
      sme(smeVal),seq(seqVal),smeId(smeIdVal),routeId(routeIdVal),trId(trIdVal)
    {
    }
  };

  void Register(int seq,int uid,const RegistryData& data,int to)
  {
    MutexGuard g(mtx);
    __trace2__("RR: register %d/%d",seq,uid);
    time_t now=time(NULL);
    while(timeList.size()>0 && timeList.front().first<=now)
    {
      __trace2__("RR: response timed out:%d/%d",timeList.front().second->first.first,timeList.front().second->first.second);
      if(timeList.front().second->second.trId!=billing::InvalidTransactionId)
      {
        GetBillingInterface()->RollbackTransaction(timeList.front().second->second.trId);
      }
      backMapping.erase(timeList.front().second->first);
      mapping.erase(timeList.front().second);
      timeList.pop_front();
    }
    Mapping::iterator it=mapping.insert(std::pair<const SeqUidPair,RegistryData>(SeqUidPair(seq,uid),data)).first;
    TimeList::iterator lit=timeList.insert(timeList.end(),make_pair(time(NULL)+to,it));
    backMapping.insert(make_pair(make_pair(seq,uid),lit));
  }

  bool Get(int seq,int uid,RegistryData& data)
  {
    MutexGuard g(mtx);
    __trace2__("RR: get %d/%d",seq,uid);
    Mapping::iterator it=mapping.find(make_pair(seq,uid));
    if(it==mapping.end())return false;
    data=it->second;
    BackMapping::iterator bit=backMapping.find(make_pair(seq,uid));
    timeList.erase(bit->second);
    backMapping.erase(bit);
    mapping.erase(it);
    return true;
  }

protected:
  Mutex mtx;
  typedef std::pair<int,int> SeqUidPair;
  typedef std::map<SeqUidPair,RegistryData> Mapping;
  typedef std::list<std::pair<time_t,Mapping::iterator> > TimeList;
  typedef std::map<SeqUidPair,TimeList::iterator> BackMapping;
  Mapping mapping;
  BackMapping backMapping;
  TimeList timeList;
};

static RespRegistry submitRegistry;
static RespRegistry deliverRegistry;
static RespRegistry replaceRegistry;
static RespRegistry queryRegistry;
static RespRegistry cancelRegistry;

StateMachine::StateMachine(EventQueue& q,
               Smsc *app):
               eq(q),
               smsc(app)

{
  smsLog = smsc::logger::Logger::getInstance("sms.trace");
}


void StateMachine::KillExpiredTrans()
{
  //__trace__("process expired transactions");
  tmon.ProcessExpired(smsc->getStatistics());
}


int StateMachine::Execute()
{
  SmscCommand cmd;
  for(;;)
  {
    eq.selectAndDequeue(cmd,&isStopping);
    if(isStopping)break;
    try{
      switch(cmd->cmdid)
      {
        case SUBMIT:submit(cmd);break;
        case SUBMIT_RESP:submitResp(cmd);break;
        case DELIVERY:delivery(cmd);break;
        case DELIVERY_RESP:deliveryResp(cmd);break;
        case ALERT:alert(cmd);break;
        case REPLACE:replace(cmd);break;
        case REPLACE_RESP:replaceResp(cmd);break;
        case QUERY:query(cmd);break;
        case QUERY_RESP:queryResp(cmd);break;
        case CANCEL:cancel(cmd);break;
        case CANCEL_RESP:cancelResp(cmd);break;
        case KILLEXPIREDTRANSACTIONS:KillExpiredTrans();UNDELIVERABLE_STATE;break;
        default:
          __warning2__("UNKNOWN COMMAND:%d",cmd->cmdid);
          break;
      }
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

void StateMachine::submit(SmscCommand& cmd)
{
  RouteInfo ri;
  SMS& sms=*cmd->get_sms();
  SmeProxy *src_proxy=cmd.getProxy();
  SmeProxy *dst_proxy;
  uint32_t dialogId =  cmd->get_dialogId();
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
    return;
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
    return;
  }

  if(!sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
  {
    int newmr=smsc->getNextMR(sms.getDestinationAddress());
    debug2(smsLog,"submit: set mr to %d",newmr);
    sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,newmr);
  }

  sms.setRouteId(ri.routeId.c_str());
  sms.setSourceSmeId(src_proxy->getSystemId());
  sms.setDestinationSmeId(dst_proxy->getSystemId());

  std::pair<TransactionMonitor::TrSmsStatus,billing::TransactionIdType> trpair=tmon.RegisterS2C(sms,32,ri.trafRules);
  TransactionMonitor::TrSmsStatus st=trpair.first;

  smsc_log_info(smsLog,"SBM: trstate for sms from %s to %s=%s",sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str(),TransactionMonitor::getStatusName(st));

  if((st&TransactionMonitor::trDeniedBase) || (st&TransactionMonitor::trInvalid))
  {
    smsc->getStatistics()->updateCounter(stat::Counters::cntRejected,src_proxy->getSystemId(),ri.routeId.c_str(),smsc::system::Status::RX_R_APPN);
    SmscCommand resp=SmscCommand::makeSubmitSmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::RX_R_APPN,
                       sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                     );
    src_proxy->putCommand(resp);
    return;
  }

  if(st==TransactionMonitor::trSmeAnswerEnd)
  {
    smsc->getStatistics()->updateCounter(stat::Counters::cntSmsTrOk,src_proxy->getSystemId(),0,0);
  }else if(st==TransactionMonitor::trScUssdEnd)
  {
    smsc->getStatistics()->updateCounter(stat::Counters::cntUssdTrFromScOk,src_proxy->getSystemId(),0,0);
  }

  int newdid=dst_proxy->getNextSequenceNumber();

  submitRegistry.Register
  (
    newdid,
    dst_index,
    RespRegistry::RegistryData
    (
      src_proxy,
      dialogId,
      src_proxy->getSystemId(),
      ri.routeId.c_str(),
      st==TransactionMonitor::trSmeGenerated?trpair.second:billing::InvalidTransactionId
    ),
    dst_proxy->getPreferredTimeout()
  );

  cmd->set_dialogId(newdid);

  bool ok=false;

  try{
    dst_proxy->putCommand(cmd);
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
    smsc->getStatistics()->updateCounter(stat::Counters::cntRejected,src_proxy->getSystemId(),ri.routeId.c_str(),smsc::system::Status::RX_T_APPN);
    SmscCommand resp=SmscCommand::makeSubmitSmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::RX_T_APPN,
                       sms.getIntProperty(Tag::SMPP_DATA_SM)!=0
                     );
    src_proxy->putCommand(resp);
    return;
  }

  //smsc->getStatistics()->updateCounter(stat::Counters::cntAccepted,src_proxy->getSystemId(),ri.routeId.c_str(),smsc::system::Status::OK);

  return;
}

void StateMachine::submitResp(SmscCommand& cmd)
{
  debug1(smsLog,"submit resp");
  SmeProxy* src_proxy=cmd.getProxy();
  SmeProxy* dst_proxy;
  int dlgId;

  RespRegistry::RegistryData rd;

  if(submitRegistry.Get(cmd->get_dialogId(),src_proxy->getSmeIndex(),rd))
  {
    dlgId=rd.seq;
    dst_proxy=rd.sme;
    debug1(smsLog,"record for submit resp found");
    cmd->set_dialogId(dlgId);
    int st=cmd->get_resp()->get_status();
    uint64_t id=0;
    if(st==0)
    {
      const char* msgid=cmd->get_resp()->get_messageId();
      sscanf(msgid,"%lld",&id);
      id<<=8;
    }

    if(rd.trId!=billing::InvalidTransactionId)
    {
      GetBillingInterface()->CommitTransaction(rd.trId);
    }

    using smsc::smeman::SmeRecord;
    SmeRecord* smerec=dynamic_cast<SmeRecord*>(src_proxy);

    if(!smerec)
    {
      warn1(smsLog,"SBMRESP: incorrect command direction (submit response to to gatewaysme)");
      smsc->getStatistics()->updateCounter(stat::Counters::cntRejected,rd.smeId.c_str(),rd.routeId.c_str(),smsc::system::Status::RX_R_APPN);
      return;
    }
    GatewaySme *gwsme=0;
    MutexGuard g(smerec->mutex);
    if(!smerec->deleted)
    {
      gwsme=dynamic_cast<GatewaySme*>(smerec->proxy);
    }
    if(gwsme)
    {
      id|=gwsme->getPrefix()&0xFF;
      char buf[64];
      sprintf(buf,"%lld",id);
      cmd->get_resp()->set_messageId(buf);
      try{
        dst_proxy->putCommand(cmd);
        smsc->getStatistics()->updateCounter(st==0?stat::Counters::cntAccepted:stat::Counters::cntRejected,rd.smeId.c_str(),rd.routeId.c_str(),st);
      }catch(...)
      {
        warn2(smsLog,"SBMRESP: failed to put command to %s",dst_proxy->getSystemId());
        smsc->getStatistics()->updateCounter(stat::Counters::cntRejected,rd.smeId.c_str(),rd.routeId.c_str(),smsc::system::Status::RX_R_APPN);
      }
    }else
    {
      warn1(smsLog,"SBMRESP: incorrect command direction (submit response to to gatewaysme)");
      smsc->getStatistics()->updateCounter(stat::Counters::cntRejected,rd.smeId.c_str(),rd.routeId.c_str(),smsc::system::Status::RX_R_APPN);
      return;
    }
  }else
  {
    debug1(smsLog,"record for submit resp not found");
  }

  return;
}


void StateMachine::delivery(SmscCommand& cmd)
{
  RouteInfo ri;
  SMS& sms=*cmd->get_sms();
  SmeProxy *src_proxy=cmd.getProxy();
  SmeProxy *dst_proxy;
  uint32_t dialogId =  cmd->get_dialogId();
  int dst_index;
  bool routeFound=smsc->routeSms(sms.getOriginatingAddress(),sms.getDestinationAddress(),dst_index,dst_proxy,&ri,src_proxy->getSmeIndex());
  if(!routeFound)
  {
    debug2(smsLog,"DLV: no route %s->%s",sms.getOriginatingAddress().toString().c_str(),sms.getDestinationAddress().toString().c_str());
    SmscCommand resp=SmscCommand::makeDeliverySmResp
                     (
                       "0",
                       dialogId,
                       smsc::system::Status::RX_P_APPN
                     );
    try{
      src_proxy->putCommand(resp);
    }catch(...)
    {
    }
    smsc->getStatistics()->updateCounter(stat::Counters::cntPerm,src_proxy->getSystemId(),0,smsc::system::Status::RX_P_APPN);
    return;
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
    smsc->getStatistics()->updateCounter(stat::Counters::cntTemp,src_proxy->getSystemId(),0,smsc::system::Status::RX_T_APPN);
    return;
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

  sms.setRouteId(ri.routeId.c_str());
  sms.setSourceSmeId(src_proxy->getSystemId());
  sms.setDestinationSmeId(dst_proxy->getSystemId());


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
    return;
  }

  if(st==TransactionMonitor::trSmeUssdEnd)
  {
    smsc->getStatistics()->updateCounter(stat::Counters::cntUssdTrFromSmeOk,src_proxy->getSystemId(),ri.routeId.c_str(),smsc::system::Status::OK);
  }

  int newdid=dst_proxy->getNextSequenceNumber();

  //deliverRegistry.Register(newdid,dst_index,dialogId,src_proxy,dst_proxy->getPreferredTimeout());
  deliverRegistry.Register(newdid,dst_index,RespRegistry::RegistryData(src_proxy,dialogId,src_proxy->getSystemId(),ri.routeId.c_str()),dst_proxy->getPreferredTimeout());

  cmd->set_dialogId(newdid);


  try{
    dst_proxy->putCommand(cmd);
    Task task(dst_proxy->getUniqueId(),newdid,0);
    smsc->tasks.createTask(task,dst_proxy->getPreferredTimeout());
  }catch(exception& e)
  {
    warn2(smsLog,"DLV: failed to put command to dest proxy:%s",e.what());
    return;
  }catch(...)
  {
    warn2(smsLog,"DLV: failed to put command to dest proxy:%s","unknown");
    return;
  }

  return;
}


void StateMachine::deliveryResp(SmscCommand& cmd)
{
  debug1(smsLog,"submit resp");
  SmeProxy* src_proxy=cmd.getProxy();
  SmeProxy* dst_proxy;
  int dlgId;

  RespRegistry::RegistryData rd;
  if(deliverRegistry.Get(cmd->get_dialogId(),src_proxy->getSmeIndex(),rd))
  {
    dlgId=rd.seq;
    dst_proxy=rd.sme;
    int st=cmd->get_resp()->get_status();
    smsc->getStatistics()->updateCounter(st==0?stat::Counters::cntDelivered:
      smsc::system::Status::isErrorPermanent(st)?stat::Counters::cntPerm:stat::Counters::cntTemp
      ,dst_proxy->getSystemId(),rd.routeId.c_str(),st);
    debug1(smsLog,"record for delivery resp found");
    cmd->set_dialogId(dlgId);
    cmd->get_resp()->set_messageId("0");
    try{
      dst_proxy->putCommand(cmd);
    }catch(...)
    {
      warn2(smsLog,"DLVRESP: failed to put command to %s",dst_proxy->getSystemId());
    }
  }else
  {
    debug1(smsLog,"record for delivery resp not found");
  }

  return;
}

void StateMachine::alert(SmscCommand& cmd)
{
  return;
}

void StateMachine::replace(SmscCommand& cmd)
{
  uint64_t msgId=cmd->get_replaceSm().getMessageId();
  debug2(smsLog,"REPLACE: msgId=%lld",msgId);
  uint8_t uid=(uint8_t)(msgId&0xff);
  GatewaySme *gwsme=smsc->getGwSme(uid);
  if(gwsme==0)
  {
    warn2(smsLog,"REPLACE: invalid uid=%d!",(int)uid);
    return;
  }

  msgId>>=8;
  sprintf(cmd->get_replaceSm().messageId.get(),"%lld",msgId);
  try{
    int newdid=gwsme->getNextSequenceNumber();
    replaceRegistry.Register(newdid,gwsme->getSmeIndex(),
       RespRegistry::RegistryData(
         cmd.getProxy(),
         cmd->get_dialogId(),
         "",
         ""),gwsme->getPreferredTimeout()
       );
    cmd->set_dialogId(newdid);
    gwsme->putCommand(cmd);
  }catch(...)
  {
    warn1(smsLog,"REPLACE: failed to put relace command");
  }
  return;
}

void StateMachine::replaceResp(SmscCommand& cmd)
{
  debug1(smsLog,"REPLACE_RESP");
  RespRegistry::RegistryData rd;
  SmeProxy* src_proxy=cmd.getProxy();
  if(replaceRegistry.Get(cmd->get_dialogId(),src_proxy->getSmeIndex(),rd))
  {
    int dlgId=rd.seq;
    SmeProxy* dst_proxy=rd.sme;
    int st=cmd->get_status();
    debug2(smsLog,"record for replace resp found, dlgId=%d",dlgId);
    cmd->set_dialogId(dlgId);
    try{
      dst_proxy->putCommand(cmd);
    }catch(...)
    {
      warn2(smsLog,"REPLACE_RESP: failed to put command to %s",dst_proxy->getSystemId());
    }
  }else
  {
    warn1(smsLog,"REPLACE_RESP: timed out or invalid");
  }
  return;
}


void StateMachine::query(SmscCommand& cmd)
{
  uint64_t msgId=cmd->get_querySm().getMessageId();
  debug2(smsLog,"QUERY: msgId=%lld",msgId);
  uint8_t uid=(uint8_t)(msgId&0xff);
  GatewaySme *gwsme=smsc->getGwSme(uid);
  if(gwsme==0)
  {
    warn2(smsLog,"QUERY: invalid uid=%d!",(int)uid);
    return;
  }

  msgId>>=8;
  sprintf(cmd->get_querySm().messageId.get(),"%lld",msgId);
  try{
    int newdid=gwsme->getNextSequenceNumber();
    queryRegistry.Register(newdid,gwsme->getSmeIndex(),
       RespRegistry::RegistryData(
         cmd.getProxy(),
         cmd->get_dialogId(),
         "",
         ""),gwsme->getPreferredTimeout()
       );
    cmd->set_dialogId(newdid);
    gwsme->putCommand(cmd);
  }catch(...)
  {
    warn1(smsLog,"QUERY: failed to put relace command");
  }
  return;
}

void StateMachine::queryResp(SmscCommand& cmd)
{
  debug1(smsLog,"QUERY_RESP");
  RespRegistry::RegistryData rd;
  SmeProxy* src_proxy=cmd.getProxy();
  if(queryRegistry.Get(cmd->get_dialogId(),src_proxy->getSmeIndex(),rd))
  {
    int dlgId=rd.seq;
    SmeProxy* dst_proxy=rd.sme;
    int st=cmd->get_status();
    debug2(smsLog,"record for quesy resp found, dlgId=%d",dlgId);
    cmd->set_dialogId(dlgId);
    try{
      dst_proxy->putCommand(cmd);
    }catch(...)
    {
      warn2(smsLog,"QUERY_RESP: failed to put command to %s",dst_proxy->getSystemId());
    }
  }else
  {
    warn1(smsLog,"QUERY_RESP: timed out or invalid");
  }
  return;
}

void StateMachine::cancel(SmscCommand& cmd)
{
  uint64_t msgId=cmd->get_cancelSm().getMessageId();
  debug2(smsLog,"CANCEL: msgId=%lld",msgId);
  uint8_t uid=(uint8_t)(msgId&0xff);
  GatewaySme *gwsme=smsc->getGwSme(uid);
  if(gwsme==0)
  {
    warn2(smsLog,"CANCEL: invalid uid=%d!",(int)uid);
    return;
  }

  msgId>>=8;
  sprintf(cmd->get_cancelSm().messageId.get(),"%lld",msgId);
  try{
    int newdid=gwsme->getNextSequenceNumber();
    cancelRegistry.Register(newdid,gwsme->getSmeIndex(),
       RespRegistry::RegistryData(
         cmd.getProxy(),
         cmd->get_dialogId(),
         "",
         ""),gwsme->getPreferredTimeout()
       );
    cmd->set_dialogId(newdid);
    gwsme->putCommand(cmd);
  }catch(...)
  {
    warn1(smsLog,"CANCEL: failed to put relace command");
  }
}

void StateMachine::cancelResp(SmscCommand& cmd)
{
  debug1(smsLog,"CANCEL_RESP");
  RespRegistry::RegistryData rd;
  SmeProxy* src_proxy=cmd.getProxy();
  if(cancelRegistry.Get(cmd->get_dialogId(),src_proxy->getSmeIndex(),rd))
  {
    int dlgId=rd.seq;
    SmeProxy* dst_proxy=rd.sme;
    int st=cmd->get_status();
    debug2(smsLog,"record for cancel resp found, dlgId=%d",dlgId);
    cmd->set_dialogId(dlgId);
    try{
      dst_proxy->putCommand(cmd);
    }catch(...)
    {
      warn2(smsLog,"CANCEL_RESP: failed to put command to %s",dst_proxy->getSystemId());
    }
  }else
  {
    warn1(smsLog,"CANCEL_RESP: timed out or invalid");
  }
}


}//system
}//smsc
