#ifndef SMSC_INFOSME_SMSCCONNECTOR
#define SMSC_INFOSME_SMSCCONNECTOR

#include <logger/Logger.h>
#include <sms/sms.h>
#include <sme/SmppBase.hpp>
#include <util/config/Manager.h>
#include <core/threads/Thread.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include "InfoSmePduListener.h"

const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;
const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;

namespace smsc { namespace infosme {

using smsc::logger::Logger;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::Mutex;
using smsc::util::config::ConfigView;

class InfoSmeConfig : public smsc::sme::SmeConfig {
public:
  InfoSmeConfig(ConfigView& config) throw(ConfigException);
  virtual ~InfoSmeConfig();

private:
    char* strHost;
    char* strSid;
    char* strPassword;
    char* strSysType;
    char* strOrigAddr;
};

using smsc::sme::SmppSession;

/*
struct SeqNum {
  SeqNum():seqNum(0) {};
  SeqNum(uint64_t seqnum, const std::string& smscid):seqNum(sn), smscId(smscid) {};
  uint64_t seqNum;
  string smscId;
};*/

class SmscConnector : public smsc::core::threads::Thread {
public:
  SmscConnector(TaskProcessor& processor, const InfoSmeConfig& cfg, const string& smscId);

  int Execute();
  void stop();
  void reconnect();
  bool isStopped() const;
  int getSeqNum();
  bool send(std::string abonent, std::string message, TaskInfo info, int seqNum);
  uint32_t sendSms(const string& org, const string& dst, const string& txt, bool flash);
  const string& getSmscId() const { return smscId_; };

private:
  bool convertMSISDNStringToAddress(const char* string, smsc::sms::Address& address);

private:
  TaskProcessor& processor_;
  Logger* logger_;
  InfoSmePduListener listener_;
  SmppSession session_;
  string smscId_;
  Mutex sendLock_;
  EventMonitor connectMonitor_;
  int timeout_;
  bool stopped_;
  bool connected_;
};

} //infosme
} //smsc

#endif

