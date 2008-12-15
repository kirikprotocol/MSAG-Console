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

#include "util/timeslotcounter.hpp"

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
                                          busyRejects_(0), maxRejects_(1000) {};
  void execute(int addrsCount, int getsetCount);
  void continueExecution(LongCallContextBase* context, bool dropped);
  void shutdown();
  bool canStop();

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
};


}//util
}//pers
}//scag

#endif
