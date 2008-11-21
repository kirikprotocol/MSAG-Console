#ifndef __SCAG_MTPERS_PERFORMANCECOUNTER_H__
#define __SCAG_MTPERS_PERFORMANCECOUNTER_H__

#include "util/timeslotcounter.hpp"

namespace scag { namespace mtpers {

struct PerfCounter {
  PerfCounter(bool counterOn = false):counterOn_(counterOn), accepted(10, 20), processed(10, 20) {};
  int getAccepted() {
    if (!counterOn_) {
      return 0;
    }
    return accepted.Get() / 10;
  }
  void incAccepted(int count = 1) {
    if (counterOn_) {
      accepted.Inc(count);
    }
  }
  int getProcessed() {
    if (!counterOn_) {
      return 0;
    }
    return processed.Get() / 10;
  }
  void incProcessed(int count = 1) {
    if (counterOn_) {
      processed.Inc(count);
    }
  }
private:
  smsc::util::TimeSlotCounter<> accepted;
  smsc::util::TimeSlotCounter<> processed;
  bool counterOn_;
};

struct Performance {
  Performance():processed(0), accepted(0), incCount(0) {};
  void inc(const Performance& performance) {
    processed += performance.processed;
    accepted += performance.accepted;
    if (performance.processed > 0 || performance.accepted > 0) {
      ++incCount;
    }
  }
  void inc(PerfCounter& perfCounter) {
    int proc = perfCounter.getProcessed();
    int accept = perfCounter.getAccepted(); 
    processed += proc;
    accepted += accept;
    if (proc > 0 || accept > 0) {
      ++incCount;
    }
  }
  void inc(int accept, int proc) {
    processed += proc;
    accepted += accept;
    if (proc > 0 || accept > 0) {
      ++incCount;
    }
  }
  void reset() {
    processed = 0;
    accepted = 0;
    incCount = 0;
  }
  int getAveregeAccepted() const {
    return incCount == 0 ? 0 : accepted / incCount;
  }
  int getAveregeProcessed() const {
    return incCount == 0 ? 0 : processed / incCount;
  }
  int processed;
  int accepted;
  int incCount;
};

}//mtpers
}//scag

#endif
  
 
