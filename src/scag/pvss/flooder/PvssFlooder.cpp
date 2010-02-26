/* $Id$ */
#include <vector>
#include <stdlib.h>
#include "PvssFlooder.h"
#include "scag/pvss/data/Property.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/base/PersClientException.h"
#include "util/debug.h"
#include "scag/util/io/Drndm.h"
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/IncCommand.h"
#include "scag/pvss/api/packets/IncModCommand.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/ProfileCommandVisitor.h"

using namespace scag2::pvss;
namespace {

PersCommandSingle* addcmd( std::vector< PersCommandSingle >& cmds )
{
    cmds.push_back( PersCommandSingle() );
    return &(cmds.back());
}

}

namespace scag2 { 
namespace pvss { 
namespace flooder {

static const size_t MASK_SIZE = 3;

class PersCallCreator : public ProfileCommandVisitor {
public:
  PersCallCreator(bool batch = false):batch_(batch) {};

  virtual bool visitBatchCommand(BatchCommand &cmd) throw(PvapException) {
    PersCallCreator creator(true);

    std::vector<BatchRequestComponent*> content = cmd.getBatchContent();
    typedef std::vector<BatchRequestComponent*>::iterator BatchIterator;
    std::vector< PersCommandSingle > cmds;
    cmds.reserve(content.size());
    for (BatchIterator i = content.begin(); i != content.end(); ++i) {
      (*i)->visit(creator);
      PersCommandSingle* persCmd = creator.getPersCmd();
      if (!persCmd) {
        continue;
      }
      cmds.push_back(*persCmd);
      delete persCmd;
    }
    if (cmds.empty()) {
      return false;
    }
    persCall_.reset( new PersCall(static_cast<ProfileType>(key_.getScopeType()), new PersCommandBatch(cmds, cmd.isTransactional()), 0));
    setKey();
    return true;
  }
  virtual bool visitDelCommand(DelCommand &cmd) throw(PvapException) {
    persCmd_.reset( new PersCommandSingle( PC_DEL ));
    persCmd_->property().setName(cmd.getVarName());
    createPersCall();
    return true;
  }
  virtual bool visitGetCommand(GetCommand &cmd) throw(PvapException) {
    persCmd_.reset( new PersCommandSingle( PC_GET ) );
    persCmd_->property().setName(cmd.getVarName());
    createPersCall();
    return true;
  }
  virtual bool visitIncCommand(IncCommand &cmd) throw(PvapException) {
    persCmd_.reset(  new PersCommandSingle( PC_INC_RESULT ) );
    persCmd_->property().setName(cmd.getVarName());
    persCmd_->property().setValue(cmd.getProperty());
    createPersCall();
    return true;
  }
  virtual bool visitIncModCommand(IncModCommand &cmd) throw(PvapException) {
    persCmd_.reset( new PersCommandSingle( PC_INC_MOD ) );
    persCmd_->property().setName(cmd.getVarName());
    persCmd_->property().setValue(cmd.getProperty());
    persCmd_->setResult(cmd.getModulus());
    createPersCall();
    return true;
  }
  virtual bool visitSetCommand(SetCommand &cmd) throw(PvapException) {
    persCmd_.reset( new PersCommandSingle( PC_SET ) );
    persCmd_->property().setName(cmd.getVarName());
    persCmd_->property().setValue(cmd.getProperty());
    createPersCall();
    return true;
  }

  void setProfileKey(const ProfileKey& key) {
    key_ = key;
  }

  PersCall* getPersCall() {
    return persCall_.release();
  }
  PersCommandSingle* getPersCmd() {
    return persCmd_.release();
  }
private:
  void createPersCall() {
    if (!batch_) {
      persCall_.reset( new PersCall(static_cast<ProfileType>(key_.getScopeType()), persCmd_.release(), 0));
      setKey();
    }
  }
  void setKey() {
    if (!persCall_.get()) {
      return;
    }
    if (key_.hasAbonentKey()) {
      persCall_->setKey(key_.getAbonentKey());
      return;
    }
    if (key_.hasOperatorKey()) {
      persCall_->setKey(key_.getOperatorKey());
      return;
    }
    if (key_.hasProviderKey()) {
      persCall_->setKey(key_.getProviderKey());
      return;
    }
    if (key_.hasServiceKey()) {
      persCall_->setKey(key_.getServiceKey());
      return;
    }
  }
private:
  std::auto_ptr<PersCall> persCall_;
  ProfileKey key_;
  std::auto_ptr<PersCommandSingle> persCmd_;
  bool batch_;
};

PvssFlooder::PvssFlooder(pvss::PersClient& pc, const FlooderConfig& config, unsigned skip):persClient_(pc), config_(config), isStopped_(false), callsCount_(0),
                                          logger_(Logger::getInstance("flooder")),
                                          addressFormat_(config.getAddressFormat()),
                                          speed_(config.getSpeed() > 0 ? config.getSpeed() +  config.getSpeed() / 20 : 10),
                                          currentSpeed_(speed_), delay_(1000000000/currentSpeed_), overdelay_(0), startTime_(0),
                                          busyRejects_(0), maxRejects_(1000), sentCalls_(0), successCalls_(0), errorCalls_(0), procTime_(0),
                                          maxprocTime_(0), minprocTime_(MAX_PROC_TIME), pattern_(0)  
{

  const std::vector< std::string >& patterns = config_.getPropertyPatterns();
  if ( patterns.size() <= 0 ) throw exceptions::IOException("too few property patterns configured");

  for ( std::vector< std::string >::const_iterator i = patterns.begin();
        i != patterns.end(); ++i ) {
      std::auto_ptr<Property> p( new Property );
      p->fromString( *i );
      generator_.addPropertyPattern( unsigned(i - patterns.begin()), p.release() );
  }
  generator_.parseCommandPatterns( config_.getCommands() );
  if ( logger_->isInfoEnabled() ) {
      for ( std::vector< ProfileCommand* >::const_iterator i = generator_.getPatterns().begin();
            i != generator_.getPatterns().end();
            ++i ) {
          smsc_log_info(logger_,"pattern #%u: %s", i - generator_.getPatterns().begin(), (*i)->toString().c_str());
      }
  }

  smsc_log_info(logger_,"shuffling %u addresses", unsigned(config_.getAddressesCount()));
  generator_.randomizeProfileKeys( config_.getAddressFormat().c_str(), config_.getAddressesCount(), skip,
                                   config_.getScopeType() );

}

void PvssFlooder::execute() {

  smsc_log_info(logger_, "execution...");

  smsc_log_info(logger_, "generate addresses");

  unsigned sleepTime = 0;
  while (persClient_.getClientStatus() != 0 && !isStopped_ ) {
    if (sleepTime > 60) {
      smsc_log_warn(logger_, "client can't connect to server");
      return;
    }
    smsc_log_warn(logger_, "waiting while pers client connecting to server...");
    sleep(1);
    ++sleepTime;
  }
  
  smsc_log_info(logger_, "pers client connected to server");
  ProfileKey profileKey;
  PersCallCreator callCreator;
  callCreator.setProfileKey(generator_.getProfileKey());
  int n = 0;
  while (!isStopped_) {

    ProfileCommand* cmd = generator_.generateCommand(pattern_);
    if ( ! cmd || config_.getOneCommandPerAbonent() ) {
        // switching to a new abonent
        callCreator.setProfileKey(generator_.getProfileKey());
        ++n;
    }
    if ( ! cmd ) continue;
    if ( cmd->visit(callCreator) )  {
      doCall(callCreator.getPersCall());
    } else {
      smsc_log_warn(logger_, "can't create pers call for cmd: '%s', probably error in batch", cmd->toString().c_str());
    }
    delete cmd;
  }

  smsc_log_info(logger_, "test case stopped");
}

void PvssFlooder::doCall( PersCall* context ) {
  int status = persClient_.getClientStatus();
  if (status == CLIENT_BUSY) {
    smsc_log_warn(logger_, "can't send request: pers client busy!");
    delete context;
    if (busyRejects_ > maxRejects_) {
      throw PersClientException(CLIENT_BUSY);
    }
    ++busyRejects_;
    return;
  }
  if (status == NOT_CONNECTED) {
    delete context;
    throw PersClientException(NOT_CONNECTED);
  }
  busyRejects_ = busyRejects_ > 0 ? busyRejects_ - 1 : 0;
  persClient_.callAsync( context, *this );
  ++callsCount_;
  ++sentCalls_;
  delay();
  //smsc_log_debug(logger_, "call cx:%p calls count %d", context, callsCount_);
  //callSync( context );
}

void PvssFlooder::callSync( PersCall* call ) {
  startTime_ = gethrtime();
  bool result = persClient_.callSync(call);
  if (result) {
    ++callsCount_;
    ++sentCalls_;
  } else {
    smsc_log_warn(logger_, "can't send call cx:%p ", call);
  }
  if (call->status() == PERSCLIENTOK || call->status() == PROPERTY_NOT_FOUND) {
    ++successCalls_;
  } else {
    //smsc_log_warn(logger_, "request result: %s", exceptionReasons[call->status()]);
    smsc_log_warn(logger_, "request result: %s", call->exception().c_str());
    ++errorCalls_;
  }
  delete call;
  delay();
}

void PvssFlooder::continuePersCall( PersCall* call, bool dropped ) {
  --callsCount_;
  smsc_log_debug(logger_, "continue execution cx:%p, calls count %d", call, callsCount_);
  //if (callParams->status() != 0 ) {
    //smsc_log_debug(logger_, "error long call result: %s", strs[callParams->status()]);
  //}
  hrtime_t* startTime = (hrtime_t *)call->context();
  uint64_t procTime = 0;
  if (startTime) {
    procTime = gethrtime() - *startTime;
    procTime_ = procTime;
    uint32_t msprocTime = static_cast<uint32_t>(procTime / 1000000);
    minprocTime_ = msprocTime < minprocTime_ ? msprocTime : minprocTime_; 
    maxprocTime_ = msprocTime > maxprocTime_ ? msprocTime : maxprocTime_; 
    smsc_log_debug(logger_, "continue execution cx:%p, process time: %d ms", call, msprocTime);
    delete startTime;
  }
  if (call->status() == PERSCLIENTOK || call->status() == PROPERTY_NOT_FOUND) {
    ++successCalls_;
    procTime_ += procTime;
  } else {
    smsc_log_warn(logger_, "request result: %s", call->exception().c_str());
    ++errorCalls_;
  }
  delete call;
}

/*
PersCall* PvssFlooder::createPersCall( ProfileType pfType,
                                        const string& addr,
                                        int intKey,
                                        std::auto_ptr<PersCommand> cmd )
{
    startTime_ = gethrtime();
    hrtime_t* st = new hrtime_t;
    *st = startTime_;
    PersCall* call = new PersCall( pfType, cmd.release(), st );
    //PersCall* call = new PersCall( pfType, cmd.release(), 0 );
    if (pfType == PT_ABONENT) {
        call->setKey(addr);
    } else {
        call->setKey(intKey);
    }
    return call;
}
*/

void PvssFlooder::delay() {
  int currentDelay = 1000000000 / currentSpeed_;
  hrtime_t endTime = gethrtime();
  unsigned sleepTime = currentDelay - overdelay_;
  timespec ts,rm={0,0};
  ts.tv_sec = 0;
  ts.tv_nsec= sleepTime;
  nanosleep(&ts,&rm);
  overdelay_ = gethrtime() - endTime - sleepTime;
  if (overdelay_ < 0) {
    overdelay_ = 0;
  }
}

void PvssFlooder::setCurrentSpeed(int sent, int ok) {
  if (ok < sent) {
    currentSpeed_ = ok > 0 ? ok : 1;
    return;
  }
  if (config_.getMaxSpeed()) {
    currentSpeed_ = currentSpeed_ + 50;
    return;
  }
  if (currentSpeed_ < speed_) {
    currentSpeed_ = currentSpeed_ + 50 > speed_ ? speed_ : currentSpeed_ + 50;
  }
}

/*
void PvssFlooder::delay() {
  hrtime_t procTime = gethrtime() - startTime_;
  //hrtime_t procTime = procTime_;
  //procTime += procTime / 10;

  procTime /= 1000;
  smsc_log_debug(logger_, "delay=%d procTime=%d", delay_, procTime);

  if (delay_ > procTime + overdelay_) {
    startTime_ = gethrtime();
    unsigned sleepTime = delay_ - procTime - overdelay_;
    __trace2__("try to sleep:%d, delay=%d", sleepTime, delay_);

    millisleep(sleepTime / 1000);

    timespec ts,rm={0,0};
    ts.tv_sec = 0;
    ts.tv_nsec= sleepTime * 1000;
    nanosleep(&ts,&rm);
    overdelay_ = (gethrtime() - startTime_) / 1000 - sleepTime;
    //overdelay_ = (gethrtime() - startTime_) / 1000 - sleepTime;
  } else {
    __trace2__("overdelay:%d", overdelay_);
    overdelay_ -= delay_;
    if (overdelay_ < 0) overdelay_ = 0;
  }
}
*/
/*
void PvssFlooder::commandsSetConfigured(const string& addr) {
  string strPropName = "some.string.var";
  string strPropValue = "some text";
  string intPropName = "other.int.var";
  int intPropValue = 354;
  std::auto_ptr<PersCommand> strcmd( setCmd(strPropName, strPropValue) );
  doCall( createPersCall(PT_ABONENT, addr, 0, strcmd) );
  std::auto_ptr<PersCommand> intcmd( setCmd(intPropName, intPropValue) );
  doCall( createPersCall(PT_ABONENT, addr, 0, intcmd) );
}

void PvssFlooder::commandsSetConfigured(const string& addr, int intKey, const string& propName, ProfileType pfType, int cmdsCount) {
  //string setStrName = propName + "_set_str_";
  //string setVal("test_string_value_");
  char suffix[5];
  for (int i = 0; i < cmdsCount; ++i) {
    sprintf(suffix,"%04d", i + 1);
    string setStrName(propName + "_set_str_");
    setStrName.append(suffix);
    string setVal("test_string_value_");
    setVal.append(suffix);
    std::auto_ptr<PersCommand> cmd1( getCmd(setStrName) );
    doCall( createPersCall(pfType,addr,intKey,cmd1) );
    std::auto_ptr<PersCommand> cmd2( setCmd(setStrName, setVal) );
    doCall( createPersCall(pfType,addr,intKey,cmd2) );
  }
}

void PvssFlooder::commandsSet(const string& addr, int intKey, const string& propName, ProfileType pfType) {

  string setStrName = propName + "_set_str";
  std::auto_ptr<PersCommand> cmd1( setCmd(setStrName, "test_string_value") );
  doCall( createPersCall(pfType,addr,intKey,cmd1) );

  string setIntName = propName + "_set_int";
  std::auto_ptr<PersCommand> cmd2( setCmd(setIntName,100) );
  doCall( createPersCall(pfType,addr,intKey,cmd2) );

  std::auto_ptr<PersCommand> cmd3( getCmd(setStrName) );
  doCall( createPersCall(pfType,addr,intKey,cmd3) );

  std::auto_ptr<PersCommand> cmd4( getCmd(setIntName) );
  doCall( createPersCall(pfType,addr,intKey,cmd4) );

  string incName = propName + "_inc";
  int incValue = 10;
  std::auto_ptr<PersCommand> cmd5( incCmd(incName, incValue) );
  doCall( createPersCall(pfType,addr,intKey,cmd5) );  

  std::auto_ptr<PersCommand> cmd6( incCmd(incName, incValue) );
  doCall( createPersCall(pfType,addr,intKey,cmd6) );  

  string incModName = propName + "_inc_mod";
  int modValue = 3;
  std::auto_ptr<PersCommand> cmd7( incModCmd(incModName, incValue, modValue) );
  doCall( createPersCall(pfType,addr,intKey,cmd7) );  

  std::auto_ptr<PersCommand> cmd8( getCmd(incName) );
  doCall( createPersCall(pfType,addr,intKey,cmd8) );  

  std::auto_ptr<PersCommand> cmd9( getCmd(incModName) );
  doCall( createPersCall(pfType,addr,intKey,cmd9) );  

  std::auto_ptr<PersCommand> cmd10( batchCmd(propName, true) );
  doCall( createPersCall(pfType,addr,intKey,cmd10) );  

}

PersCommandSingle* PvssFlooder::getCmd(const string& propName, PersCommandSingle* cmd ) {
  PersCommandSingle* persCmd = cmd ? cmd : new PersCommandSingle( PC_GET );
  if ( cmd ) cmd->setType( PC_GET );
  Property& prop = persCmd->property();
  prop.setName(propName);
  return persCmd;
}

PersCommandSingle* PvssFlooder::setCmd(const string& propName, const string& strVal, PersCommandSingle* cmd ) {
  PersCommandSingle* persCmd = cmd ? cmd : new PersCommandSingle( PC_SET );
  if ( cmd ) cmd->setType( PC_SET );
  Property& prop = persCmd->property();
  prop.setName(propName);
  prop.setStringValue(strVal.c_str());
  return persCmd;
}

PersCommandSingle* PvssFlooder::setCmd(const string& propName, int intVal, PersCommandSingle* cmd ) {
  PersCommandSingle* persCmd = cmd ? cmd : new PersCommandSingle( PC_SET );
  if ( cmd ) cmd->setType( PC_SET );
  Property& prop = persCmd->property();
  prop.setName(propName);
  prop.setIntValue(intVal);
  return persCmd;
}

PersCommandSingle* PvssFlooder::incCmd(const string& propName, int inc, PersCommandSingle* cmd ) {
  PersCommandSingle* persCmd = cmd ? cmd : new PersCommandSingle(PC_INC_RESULT);
  if ( cmd ) cmd->setType( PC_INC_RESULT );
  Property& prop = persCmd->property();
  prop.setName(propName);
  prop.setIntValue(inc);
  return persCmd;
}

PersCommandSingle* PvssFlooder::incModCmd(const string& propName, int inc, int mod, PersCommandSingle* cmd ) {
  PersCommandSingle* persCmd = cmd ? cmd : new PersCommandSingle(PC_INC_MOD);
  if ( cmd ) cmd->setType( PC_INC_MOD );
  Property& prop = persCmd->property();
  prop.setName(propName);
  prop.setIntValue(inc);
  persCmd->setResult(mod);
  return persCmd;
}

PersCommandBatch* PvssFlooder::batchCmd(const string& propName, bool trans) {
    string name = propName + "_batch";
    std::vector< PersCommandSingle > cmds;
    cmds.reserve(5);
    setCmd(name, 100, ::addcmd(cmds) );
    incCmd(name, 10, ::addcmd(cmds));
    incModCmd(name, 15, 3, ::addcmd(cmds));
    incModCmd(name, 34, 10, ::addcmd(cmds));
    getCmd(name, ::addcmd(cmds));
    return new PersCommandBatch(cmds, trans);
}
*/
bool PvssFlooder::canStop() {
  return callsCount_ > 0 ? false : true;
}

void PvssFlooder::shutdown() {
  isStopped_ = true;
}

int PvssFlooder::getSuccess() {
    int ret = successCalls_;
    successCalls_ = 0;
    return ret;
}
int PvssFlooder::getError() {
    int ret = errorCalls_;
    errorCalls_ = 0;
    return ret;
}
int PvssFlooder::getSent() {
    int ret = sentCalls_;
    sentCalls_ = 0;
    return ret;
}
int PvssFlooder::getBusy() {
    int ret = busyRejects_;
    busyRejects_ = 0;
    return ret;
}
uint32_t PvssFlooder::getProcTime() {
    smsc_log_info(logger_, "%d/%d ms min/max process time", minprocTime_, maxprocTime_);
    minprocTime_ = MAX_PROC_TIME;
    maxprocTime_ = 0;
    uint32_t ret = successCalls_ > 0 ? (procTime_ / successCalls_) / 1000000 : 0;
    smsc_log_info(logger_, "%d ms average process time", ret);
    procTime_ = 0;
    return ret;
}

}}}
