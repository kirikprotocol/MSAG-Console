#ifndef __SMSC_STAT_SMESTATS_HPP__
#define __SMSC_STAT_SMESTATS_HPP__

#include "util/int.h"
#include <vector>
#include <string.h>
#include "smeman/smeman.h"
#include "core/synchronization/Mutex.hpp"

namespace smsc{
namespace stat{

class SmeStats
{
public:
  enum CounterId{cntAccepted,cntRejected,cntRetried,cntDelivered,cntFailed,cntTempError};

  struct Counters{
    uint64_t cnt[cntTempError+1];
    Counters()
    {
      memset(this,0,sizeof(*this));
    }
  };

  SmeStats()
  {
    smeStats.resize(MAX_SME_PROXIES);
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

  const Counters& getCnt(int smeIdx)
  {
    return smeStats[smeIdx];
  }

protected:
  std::vector<Counters> smeStats;
  smsc::core::synchronization::Mutex mtx;
};

}//stat
}//smsc

#endif
