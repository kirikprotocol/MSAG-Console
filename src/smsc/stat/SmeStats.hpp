#ifndef __SMSC_STAT_SMESTATS_HPP__
#define __SMSC_STAT_SMESTATS_HPP__

#include <vector>
#include <string.h>

#include "util/int.h"
#include "core/synchronization/Mutex.hpp"
#include "smsc/smeman/smeman.h"

namespace smsc{
namespace stat{

class SmeStats
{
public:
  enum CounterId{cntAccepted,cntRejected,cntRetried,cntDelivered,cntFailed,cntTempError};
  enum {
    MAX_ERROR_INDEX=1500
  };

  typedef std::vector<uint64_t> ErrCntVector;

  struct Counters{
    uint64_t cnt[cntTempError+1];
    Counters()
    {
      memset(this,0,sizeof(*this));
    }
  };

  SmeStats()
  {
    smeStats.resize(smsc::smeman::MAX_SME_PROXIES);
    smeErrors.resize(smsc::smeman::MAX_SME_PROXIES);
    for(int i=0;i<smsc::smeman::MAX_SME_PROXIES;i++)
    {
      smeErrors[i].resize(MAX_ERROR_INDEX,0);
    }
  }

  void incCounter(int smeIdx,CounterId id)
  {
    if(smeIdx<0 || smeIdx>smeStats.size() || id<0 || id>cntTempError)
    {
      //error
      return;
    }
    smsc::core::synchronization::MutexGuard mg(mtx);
    smeStats[smeIdx].cnt[id]++;
  }

  void incError(int smeIdx,int errCode)
  {
    if(smeIdx<0 || smeIdx>smeStats.size() || errCode<0 || errCode>MAX_ERROR_INDEX)
    {
      //error
      return;
    }
    smsc::core::synchronization::MutexGuard mg(mtx);
    smeErrors[smeIdx][errCode]++;
  }

  void getErrors(size_t smeIdx,ErrCntVector& rv)
  {
    if(smeIdx<0 || smeIdx>smeStats.size())
    {
      //error
      return;
    }
    rv=smeErrors[smeIdx];
  }

  const Counters& getCnt(size_t smeIdx)
  {
    if(smeIdx<0 || smeIdx>smeStats.size())
    {
      static Counters defaultCounters;
      //error
      return defaultCounters;
    }
    return smeStats[smeIdx];
  }

protected:
  std::vector<Counters> smeStats;
  std::vector<ErrCntVector> smeErrors;
  smsc::core::synchronization::Mutex mtx;
};

}//stat
}//smsc

#endif
