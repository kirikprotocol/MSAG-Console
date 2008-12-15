/* $Id$ */

#ifndef SCAG_LC_PERS_CLIENT_H
#define SCAG_LC_PERS_CLIENT_H

#include <string>
#include "scag/pers/util/PersClient2.h"
#include "scag/re/base/LongCallContextBase.h"
#include "scag/re/base/ActionContext2.h"
#include "scag/pers/util/PersCommand.h"
#include "scag/pers/util/PersCallParams.h"
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "util/timeslotcounter.hpp"
#include "util/sleep.h"

namespace scag2 { namespace pers { namespace util {

using scag2::lcm::LongCallContextBase;
using smsc::logger::Logger;
using std::string;
using namespace scag::pers::util::perstypes;


class LCCommandCreator : public PersCommandCreator {
public:
  virtual PersCmd cmdType() const {
    return PC_UNKNOWN;
  }
  virtual void storeResults(re::actions::ActionContext &ctx, PersCommand &cmd) { }
};

class LCPersClient : public scag2::lcm::LongCallInitiator {
public:
  LCPersClient(PersClient& pc, int speed):persClient_(pc), isStopped_(false), callsCount_(0), logger_(Logger::getInstance("lcclient")),
                                          speed_(speed > 0 ? speed : 1), delay_(1000000/speed_), overdelay_(0), startTime_(0),
                                          busyRejects_(0), maxRejects_(1000), sentCalls_(0), successCalls_(0), errorCalls_(0) {};
  void execute(int addrsCount, int getsetCount);
  void continueExecution(LongCallContextBase* context, bool dropped);
  void shutdown();
  bool canStop();

  int getSuccess();
  int getError();
  int getSent();
  int getBusy();

private:
  void doCall(LongCallContextBase* context);
  PersCommandSingle* getCmd(const string& propName);
  PersCommandSingle* setCmd(const string& propName, const string& strVal, int intVal = 0);
  PersCommandSingle* incCmd(const string& propName, int inc);
  PersCommandSingle* incModCmd(const string& propName, int inc, int mod);
  PersCommandBatch* batchCmd(const string& propName, bool trans);
  void commandsSet(const string& addr, int intKey, const string& propName, ProfileType pfType);
  void commandsSetConfigured(const string& addr, int intKey, const string& propName, ProfileType pfType, int cmdsCount);
  LongCallContextBase* getCallContext(PersCallParams *callParams);
  PersCallParams *getPersPersCallParams(ProfileType pfType, const string& addr, int intKey, std::auto_ptr<PersCommand> cmd);
  void delay();

private:
  PersClient& persClient_;
  bool isStopped_;
  uint32_t callsCount_;
  LCCommandCreator creator;
  Logger* logger_;
  int speed_; //req/sec
  int delay_;
  int overdelay_;
  hrtime_t startTime_;
  int busyRejects_;
  int maxRejects_;

  int successCalls_;
  int errorCalls_;
  int sentCalls_;
};

class CallsCounter : public smsc::core::threads::Thread {
public:
  CallsCounter(LCPersClient* client, int period):client_(client), period_(period), stopped_(false), success_(0), error_(0),
                                                 sent_(0), logger_(Logger::getInstance("counter")) {
    Start();
  };
  ~CallsCounter() {
    stopped_ = true;
  }
  int Execute() {
    smsc_log_info(logger_, "counter started");
    while (!stopped_) {
      sleep(period_);
      //millisleep(period_ * 1000);
      if (stopped_) {
        break;
      }
      perfCount();
    }
    smsc_log_info(logger_, "counter stopped");
    return 1;
  }
  bool stop() {
    stopped_ = true;
    averageCount();
    return stopped_;
  }
private:
  void perfCount() {
    int sent = client_->getSent() / period_;
    int ok = client_->getSuccess() / period_;
    int errtotal = client_->getError();
    int err = errtotal / period_;
    if (sent != 0 || ok != 0 || err != 0) {
      success_ += ok;
      sent_ += sent;
      error_ += err;
      ++count_;
      smsc_log_info(logger_, "%d/%d/%d sent/ok/error per second", sent, ok, err);
    }
    if (errtotal > 0 && err == 0) {
      smsc_log_warn(logger_, "%d errors per %d seconds", errtotal, period_);
    }
    int busy = client_->getBusy() / period_;
    if (busy > 0) {
      smsc_log_warn(logger_, "%d busy rejects per second", busy);
    }
  }

  void averageCount() {
    smsc_log_info(logger_, "average %d/%d/%d sent/ok/error per second", sent_/count_, success_/count_, error_/count_);
  }

private:
  bool stopped_;
  LCPersClient* client_;
  Logger* logger_;
  int period_;
  int success_;
  int error_;
  int sent_;
  int count_;
};


}//util
}//pers
}//scag

#endif
