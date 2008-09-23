#ifndef __SMSC_INFOSME_RETRYPOLICIES_HPP__
#define __SMSC_INFOSME_RETRYPOLICIES_HPP__

#include "core/buffers/IntHash.hpp"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc{
namespace infosme{

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;
namespace conf=smsc::util::config;

class RetryPolicies{
public:
  void Load(conf::ConfigView* cv)
  {
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
  }
  int getRetryTime(const char* policy,int errorCode)
  {
    sync::MutexGuard mg(mtx);
    const Policy& p=n2p[policy];
    if(p.c2t.Exist(errorCode))
    {
      return p.c2t.Get(errorCode);
    }
    return p.deftime;
  }
protected:
  typedef buf::IntHash<int> Code2Time;
  struct Policy{
    Code2Time c2t;
    int deftime;
  };
  typedef buf::Hash<Policy> Name2Policy;
  Name2Policy n2p;
  sync::Mutex mtx;
};

}
}

#endif

