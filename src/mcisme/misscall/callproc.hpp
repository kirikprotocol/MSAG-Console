#ifndef __SMSC_MISSCALL_CALLPROC_HPP__
#define __SMSC_MISSCALL_CALLPROC_HPP__

#include <inttypes.h>
#include <time.h>
#include <string>
#include <vector>

#include <core/buffers/Hash.hpp>
#include <core/synchronization/Mutex.hpp>
#include <logger/Logger.h>
#include <core/network/Socket.hpp>

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

// missed call event flags
static const uint8_t NON_ANTI_AON_FOR_CALLER = 0;
static const uint8_t ANTI_AON_FOR_CALLER = 0x01;

struct MissedCallEvent{
  string from;
  string to;
  time_t time;
  uint8_t cause;
  uint8_t flags;
  bool gotFromIAMSME;
  std::string toString() const {
    char strBuf[1024];
    snprintf(strBuf, sizeof(strBuf), "from=%s,to=%s,time=0x%x,cause=%u,flags=%u,gotFromIAMSME=%u",
             from.c_str(), to.c_str(), time, cause, flags, gotFromIAMSME);
    return strBuf;
  }
};
class MissedCallListener{
  public:
    virtual void missed(const MissedCallEvent& event) = 0;
};
struct Circuits {
  uint32_t ts;
  uint8_t  hsn;
  uint8_t  spn;
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
static const uint8_t COUNTRY_CODE_MAX_LEN = 0x03;

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
  typedef enum {REAL_CALL_PROCESOR, CALL_PROCESOR_EMULATOR} instance_type_t;
  static void setInstanceType(instance_type_t instance_type);
  static MissedCallProcessor* instance();
  virtual int  run();
  virtual void stop();
  virtual void addMissedCallListener(MissedCallListener* listener);
  virtual void removeMissedCallListener();
  virtual void fireMissedCallEvent(const MissedCallEvent& event);
  virtual void setCircuits(Hash<Circuits>& cics);
  virtual void setRules(vector<Rule>& rules);
  virtual void setReleaseSettings(ReleaseSettings params);
  virtual void setRedirectionAddress(const char* address);
  virtual bool setCountryCode(const char* _countryCode);

protected:
  MissedCallProcessor();

  MissedCallListener* listener;
  static MissedCallProcessor* volatile processor;
  static Mutex lock;
  static instance_type_t _instanceType;
};

class MissedCallProcessorEmulator : public MissedCallProcessor
{
public:
  MissedCallProcessorEmulator();
  virtual int  run();
  static void setHost(const std::string& host);
  static void setPort(int port);
private:
  smsc::logger::Logger* _logger;
  smsc::core::network::Socket _serverSocket;
  static std::string _host;
  static in_port_t _port;
};

}//namespace misscall
}//namespace smsc
#endif
