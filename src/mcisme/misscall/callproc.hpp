#ifndef __SMSC_MISSCALL_CALLPROC_HPP__
#define __SMSC_MISSCALL_CALLPROC_HPP__

#include <time.h>
#include <string>
#include "core/synchronization/Mutex.hpp"

namespace smsc{
namespace misscall{

using std::string;
using smsc::core::synchronization::Mutex;

struct MissedCallEvent{
  string from;
  string to;
  time_t time;
};
class MissedCallListener{
  public:
    virtual void missed(MissedCallEvent& event) = 0;
};
class MissedCallProcessor{
  public:
    static MissedCallProcessor* instance();
    void run();
    void stop();
    void addMissedCallListener(MissedCallListener* listener);
    void removeMissedCallListener();
    void fireMissedCallEvent(MissedCallEvent& event);
  private:
    MissedCallProcessor();
    MissedCallListener* listener;
    static MissedCallProcessor* volatile processor;
    static Mutex lock;
};

}//namespace misscall
}//namespace smsc
#endif
