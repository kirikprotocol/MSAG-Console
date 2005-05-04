#ifndef __SMSC_MISSCALL_CALLPROC_HPP__
#define __SMSC_MISSCALL_CALLPROC_HPP__

#include <inttypes.h>
#include <time.h>
#include <string>
#include <vector>

#include <core/buffers/Hash.hpp>
#include <core/synchronization/Mutex.hpp>

namespace smsc{
namespace misscall{

using std::string;
using std::vector;
using smsc::core::synchronization::Mutex;
using smsc::core::buffers::Hash;

static const uint8_t NONE    = 0x00;
static const uint8_t ABSENT  = 0x01;
static const uint8_t BUSY    = 0x02;
static const uint8_t NOREPLY = 0x04;
static const uint8_t UNCOND  = 0x08;
static const uint8_t DETACH  = 0x10;
static const uint8_t ALL     = 0xFF;

struct MissedCallEvent{
  string from;
  string to;
  time_t time;
  uint8_t cause;
};
class MissedCallListener{
  public:
    virtual void missed(MissedCallEvent event) = 0;
};
struct Circuits {
  int  hsn;
  int  spn;
  long ts;
};

struct Rule {
  string rx;
  string name;
  int priority;
  int cause;
  int inform;
  bool operator<(const Rule& rule) const {
    return this->priority >= rule.priority;
  }
};

static const uint8_t PREFIXED_STRATEGY = 0x01;
static const uint8_t REDIRECT_STRATEGY = 0x02; // MTS defualt strategy
static const uint8_t MIXED_STRATEGY    = 0x03;
static const uint8_t REDIREC_RULE_STRATEGY = 0x04;

struct ReleaseSettings {
  int strategy;
  int busyCause, busyInform;
  int noReplyCause, noReplyInform;
  int unconditionalCause, unconditionalInform;
  int absentCause, absentInform;
  int detachCause, detachInform;
  int otherCause, otherInform;
  bool skipUnknownCaller;
};

bool setCallingMask(const char* rx);
bool setCalledMask(const char* rx);

class MissedCallProcessor
{
  public:
    static MissedCallProcessor* instance();
    int  run();
    void stop();
    void addMissedCallListener(MissedCallListener* listener);
    void removeMissedCallListener();
    void fireMissedCallEvent(MissedCallEvent& event);
    void setCircuits(Hash<Circuits>& cics);
    void setRules(vector<Rule>& rules);
    void setReleaseSettings(ReleaseSettings params);
    void setRedirectionAddress(const char* address);

  private:
    MissedCallProcessor();
    MissedCallListener* listener;
    static MissedCallProcessor* volatile processor;
    static Mutex lock;
};

}//namespace misscall
}//namespace smsc
#endif
