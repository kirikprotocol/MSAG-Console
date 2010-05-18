#ifndef __SMSC_INFOSME_RETRYPOLICIES_HPP__
#define __SMSC_INFOSME_RETRYPOLICIES_HPP__

#include "core/buffers/IntHash.hpp"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace smsc{
namespace infosme{

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;
namespace conf=smsc::util::config;

class RetryPolicies{
public:
  RetryPolicies()
  {
  }
  void Load(conf::ConfigView* cv)
  {
    log=smsc::logger::Logger::getInstance("retryPlcy");
    smsc_log_info(log,"Loading Retry Policies");
    sync::MutexGuard mg(mtx);
    n2p.Empty();
    std::auto_ptr<conf::CStrSet> pnames(cv->getShortSectionNames());
    for(conf::CStrSet::iterator it=pnames->begin();it!=pnames->end();it++)
    {
      Policy& p=n2p[it->c_str()];
      std::auto_ptr<conf::ConfigView> policy(cv->getSubConfig(it->c_str()));
      std::auto_ptr<conf::CStrSet> codes(policy->getIntParamNames());
      for(conf::CStrSet::iterator cit=codes->begin();cit!=codes->end();cit++)
      {
        if(*cit=="default")
        {
          p.deftime=policy->getInt(cit->c_str());
        }else
        {
          p.c2t.Insert(atoi(cit->c_str()),policy->getInt(cit->c_str()));
        }
      }
    }
    smsc_log_info(log,"loaded %d policies",n2p.GetCount());
    if(!n2p.Exists("default"))
    {
      Policy def;
      def.deftime=3600;
      smsc_log_warn(log,"default policy not found! created with default time %d",def.deftime);
      n2p["default"]=def;
    }
  }
  int getRetryTime(const char* policy,int errorCode)
  {
    sync::MutexGuard mg(mtx);
    Policy* pptr=n2p.GetPtr(policy);
    if(!pptr)
    {
      smsc_log_warn(log,"policy %s not found! returned default retry time 3600.",policy);
      return 3600;
    }
    const Policy& p=*pptr;
    if(p.c2t.Exist(errorCode))
    {
      return p.c2t.Get(errorCode);
    }
    return p.deftime;
  }
protected:
  typedef buf::IntHash<int> Code2Time;
  struct Policy{
#ifdef INTHASH_USAGE_CHECKING
    Policy() : c2t(SMSCFILELINE) {}
#endif
    Code2Time c2t;
    int deftime;
  };
  typedef buf::Hash<Policy> Name2Policy;
  Name2Policy n2p;
  sync::Mutex mtx;
  smsc::logger::Logger* log;
};

}
}

#endif

