/* $Id$ */

#ifndef SCAG_LC_PERS_CLIENT_H
#define SCAG_LC_PERS_CLIENT_H

#include <string>
#include "scag/pvss/base/PersClient.h"
#include "scag/pvss/base/PersCommand.h"
#include "scag/pvss/base/PersCall.h"
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "util/timeslotcounter.hpp"
#include "util/sleep.h"
#include "RequestGenerator.h"
#include "FlooderConfig.h"

namespace scag2 {
namespace pvss {
namespace flooder {

using smsc::logger::Logger;
using std::string;
/*
struct FlooderContext {
};
*/

static const uint32_t MAX_PROC_TIME = 10000;

class PvssFlooder : public pvss::PersCallInitiator
{
public:
  PvssFlooder(pvss::PersClient& pc, const FlooderConfig& config, unsigned skip);
  void execute();
  virtual void continuePersCall( pvss::PersCall* context, bool dropped );
  void shutdown();
  bool canStop();

  int getSuccess();
  int getError();
  int getSent();
  int getBusy();
  void setCurrentSpeed(int sent, int ok);
  uint32_t getProcTime();

private:
    void doCall( pvss::PersCall* context );
    void callSync( PersCall* call );
    void delay();
    //pvss::PersCommandSingle* getCmd(const string& propName, pvss::PersCommandSingle* cmd = 0 );
    //pvss::PersCommandSingle* setCmd(const string& propName, const string& strVal, pvss::PersCommandSingle* cmd = 0 );
    //pvss::PersCommandSingle* setCmd( const string& propName, int intVal, pvss::PersCommandSingle* cmd = 0 );
    //pvss::PersCommandSingle* incCmd(const string& propName, int inc, pvss::PersCommandSingle* cmd = 0 );
    //pvss::PersCommandSingle* incModCmd(const string& propName, int inc, int mod, pvss::PersCommandSingle* cmd = 0 );
    //pvss::PersCommandBatch* batchCmd(const string& propName, bool trans);
    //void commandsSet(const string& addr, int intKey, const string& propName, pvss::ProfileType pfType);
    //void commandsSetConfigured(const string& addr, int intKey, const string& propName, pvss::ProfileType pfType, int cmdsCount);
    //void commandsSetConfigured(const string& addr);
    //pvss::PersCall* createPersCall( pvss::ProfileType pfType, const string& addr, int intKey, std::auto_ptr<pvss::PersCommand> cmd );


private:
  pvss::PersClient& persClient_;
  FlooderConfig config_;
  bool isStopped_;
  uint32_t callsCount_;
  Logger* logger_;
  string addressFormat_;
  int speed_; //req/sec
  int currentSpeed_;
  int delay_;
  int overdelay_;
  hrtime_t startTime_;
  int busyRejects_;
  int maxRejects_;

  int successCalls_;
  int errorCalls_;
  int sentCalls_;
  uint64_t procTime_;
  uint32_t maxprocTime_;
  uint32_t minprocTime_;
  RequestGenerator generator_;
  unsigned pattern_;
};

class CallsCounter : public smsc::core::threads::Thread {
public:
  CallsCounter(PvssFlooder* client, int period):client_(client), period_(period > 0 ? period : 5), stopped_(false), success_(0), error_(0), procTime_(0),
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
    //int procTime = client_->getProcTime();
    int sent = client_->getSent() / period_;
    int ok = client_->getSuccess() / period_;
    int errtotal = client_->getError();
    client_->setCurrentSpeed(sent, ok);
    int err = errtotal / period_;
    if (sent != 0 || ok != 0 || err != 0) {
      success_ += ok;
      sent_ += sent;
      error_ += err;
      //procTime_ += procTime;
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
    if (count_ > 0) {
      smsc_log_info(logger_, "average %d/%d/%d sent/ok/error per second",
                     sent_/count_, success_/count_, error_/count_);
    }
  }

private:
  bool stopped_;
  PvssFlooder* client_;
  Logger* logger_;
  int period_;
  int success_;
  int error_;
  int sent_;
  int count_;
  int procTime_;
};


}//flooder
}//pvss
}//scag2

#endif
