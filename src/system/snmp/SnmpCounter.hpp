#ifndef __SMSC_SYSTEM_SNMP_COUNTERS_HPP__
#define __SMSC_SYSTEM_SNMP_COUNTERS_HPP__

#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/Hash.hpp"
#include "core/threads/Thread.hpp"
#include "logger/Logger.h"
#include "snmp/SnmpAgent.hpp"
#include "core/buffers/FixedLengthString.hpp"
#include "core/buffers/CyclicQueue.hpp"

namespace smsc{
namespace system{

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;

class SnmpCounter:public smsc::core::threads::Thread{
public:
  static void Init(const char* cfgFile)
  {
    instance=new SnmpCounter;
    instance->LoadCfg(cfgFile);
    instance->Start();
  }
  static void Shutdown()
  {
    instance->Stop();
    instance->WaitFor();
    delete instance;
    instance=0;
  }
  static SnmpCounter& getInstance()
  {
    return *instance;
  }

  void LoadCfg(const char* fileName);
  void ReloadCfg()
  {
    LoadCfg(configFileName.c_str());
  }

  enum CounterId{
    cntAccepted,
    cntRejected,
    cntRetried,
    cntDelivered,
    cntFailed,
    cntTempError,
    cntErr0x14,
    cntErr0x58,
    cntErr0xd,
    cntErrSDP,
    cntErrOther,
    cntCountersNumber
  };

  void incCounter(CounterId id,const char* smeId)
  {
    if(id>=cntCountersNumber)
    {
      smsc_log_warn(smsc::logger::Logger::getInstance("snmpcnt"),"INVALID VALUE FOR COUNTER ID:%d",id);
      return;
    }
    CounterEvent evt(id,smeId);
    sync::MutexGuard mg(queueMon);
    cntQueue.Push(evt);
    queueMon.notify();
    /*
    sync::MutexGuard mg(mon);
    totalCnt.values[id]++;
    Counters* ptr=smeCnt.GetPtr(smeId);
    if(!ptr)
    {
      ptr=smeCnt.SetItem(smeId,Counters());
    }
    ptr->values[id]++;
    */
  }

  struct SeverityLimits
  {
    int values[cntCountersNumber][4];
    bool enabled;
    SeverityLimits()
    {
      memset(values,0,sizeof(values));
      enabled=true;
    }
  };

  struct SmeTrapSeverities{
    int onRegister;
    int onRegisterFailed;
    int onUnregister;
    int onUnregisterFailed;
    SmeTrapSeverities()
    {
      onRegister=smsc::snmp::SnmpAgent::NORMAL;
      onRegisterFailed=smsc::snmp::SnmpAgent::MAJOR;
      onUnregister=smsc::snmp::SnmpAgent::WARNING;
      onUnregisterFailed=smsc::snmp::SnmpAgent::MAJOR;
    }
  };

  SmeTrapSeverities getSmeSeverities(const char* sysId)
  {
    sync::MutexGuard mg(cfgMtx);
    SmeTrapSeverities* ptr=smeTrpSvrts.GetPtr(sysId);
    if(!ptr)return defTrpSvrt;
    return *ptr;
  }

protected:
  struct Counters
  {
    Counters()
    {
      memset(values,0,sizeof(values));
    }
    uint32_t values[cntCountersNumber];
  };

  typedef smsc::snmp::SnmpAgent::alertSeverity SeverityType;

  struct Severities
  {
    Severities()
    {
      for(int i=0;i<cntCountersNumber;i++)
      {
        values[i]=smsc::snmp::SnmpAgent::NORMAL;
      }
    }
    SeverityType values[cntCountersNumber];
  };

  std::string configFileName;

  Counters totalCnt;
  Severities totalSvrt;
  buf::Hash<Counters> smeCnt;
  buf::Hash<Severities> smeSvrt;
  //sync::EventMonitor mon;
  bool isStopping;

  int interval;

  smsc::logger::Logger* log;

  sync::Mutex cfgMtx;
  buf::Hash<SeverityLimits> svrtHash;
  SeverityLimits defSvrtLim;

  sync::Mutex smeSvrtsMtx;
  buf::Hash<SmeTrapSeverities> smeTrpSvrts;
  SmeTrapSeverities defTrpSvrt;

  struct CounterEvent{
    CounterId id;
    smsc::core::buffers::FixedLengthString<32> sysId;
    CounterEvent(){}
    CounterEvent(CounterId argId,const char* argSysId):id(argId),sysId(argSysId){}
  };
  buf::CyclicQueue<CounterEvent> cntQueue;
  sync::EventMonitor queueMon;


  SnmpCounter():isStopping(false)
  {
    log=smsc::logger::Logger::getInstance("snmp.cnt");
  }
  static SnmpCounter* instance;

  void Stop()
  {
    //sync::MutexGuard mg(mon);
    isStopping=true;
    //mon.notify();
  }

  int Execute()
  {
    time_t nextCheck=time(NULL);
    nextCheck+=interval-(nextCheck%interval);
    smsc::logger::Logger* chkLog=smsc::logger::Logger::getInstance("snmp.chk");
    while(!isStopping)
    {
      {
        sync::MutexGuard qmg(queueMon);
        if(cntQueue.Count()==0)
        {
          queueMon.wait(1000);
        }else
        {
          CounterEvent evt;
          if(cntQueue.Pop(evt))
          {
            totalCnt.values[evt.id]++;
            Counters* ptr=smeCnt.GetPtr(evt.sysId.c_str());
            if(!ptr)
            {
              ptr=smeCnt.SetItem(evt.sysId.c_str(),Counters());
            }
            ptr->values[evt.id]++;
          }
        }
      }
      if(isStopping)break;
      if(nextCheck>time(NULL))continue;
      //sync::MutexGuard mg(mon);
      nextCheck=time(NULL)+1;
      nextCheck+=interval-(nextCheck%interval);
      smsc_log_info(chkLog,"check started. next check at %d",(int)nextCheck);
      Check("GLOBAL",totalCnt,totalSvrt);
      char* name;
      Counters* cnt;
      smeCnt.First();
      while(smeCnt.Next(name,cnt))
      {
        Severities* svrt=smeSvrt.GetPtr(name);
        if(!svrt)
        {
          svrt=smeSvrt.SetItem(name,Severities());
        }
        Check(name,*cnt,*svrt);
      }
      smsc_log_info(chkLog,"check finished.");
    }
    return 0;
  }

  SeverityType GetSeverityValue(const char* name,int cntIdx,uint32_t cntValue)
  {
    sync::MutexGuard mg(cfgMtx);
    SeverityLimits *limPtr=svrtHash.GetPtr(name);
    if(!limPtr)limPtr=&defSvrtLim;
    if(!limPtr->enabled)return smsc::snmp::SnmpAgent::NORMAL;
    int* limArr=limPtr->values[cntIdx];
    int rv=smsc::snmp::SnmpAgent::NORMAL;
    if(limArr[0]<limArr[1])
    {
      smsc_log_debug(log,"normal severity order for %s/%d",name,cntIdx);
      for(int i=0;i<4;i++)
      {
        if(cntValue<limArr[i])break;
        rv++;
      }
    }else
    {
      smsc_log_debug(log,"reversed severity order for %s/%d",name,cntIdx);
      for(int i=0;i<4;i++)
      {
        if(cntValue>limArr[i])break;
        rv++;
      }
    }
    smsc_log_info(log,"severity=%d for %s(%d):%d",rv,name,cntIdx,cntValue);
    return (SeverityType)rv;
  }

  void Check(const char* name,Counters& cnt,Severities& svrt);

};

} //system
} //smsc

#endif
