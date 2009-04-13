/* $Id$ */
#include <vector>
#include <stdlib.h>
#include "PvssFlooder.h"
#include "scag/pvss/data/Property.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/base/PersClientException.h"
#include "util/debug.h"
#include "scag/util/Drndm.h"

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

void PvssFlooder::execute(int addrsCount, int getsetCount) {

  smsc_log_info(logger_, "execution...");

  smsc_log_info(logger_, "generate addresses");

  scag2::util::Drndm::getRnd().setSeed(uint64_t(time(0)));
  generator_.randomizeProfileKeys(addressFormat_, addrsCount);

  while (persClient_.getClientStatus() != 0 && !isStopped_ ) {
    smsc_log_warn(logger_, "waiting while pers client connecting to server...");
    sleep(1);
  }
  
  smsc_log_info(logger_, "pers client connected to server");
  //int addrsCount = 1000000;
  char addr[20];

  procTime_ = 100000000;
  unsigned number = 0;
  int iterCount = 0;
  for (int i = 0; i < addrsCount; ++i) {
    if (isStopped_) {
      smsc_log_warn(logger_, "pers client stopped");
      break;
    }
    ProfileKey key = generator_.getProfileKey();
    commandsSetConfigured(key.getAbonentKey(), i, "test_abnt_prop", PT_ABONENT, getsetCount);
  }
  /*
  srand(time(NULL));
  while (!isStopped_) {
  //while (iterCount == 0) {
    number = (rand() * RAND_MAX + 1) | (rand() + 1);
    number = number % addrsCount;
    snprintf(addr, sizeof(addr), addressFormat_.c_str(), number);			
    //commandsSetConfigured(addr);
    commandsSetConfigured(addr, number, "test_abnt_prop", PT_ABONENT, getsetCount);
    //commandsSet(addr, number, "test_serv_prop", PT_ABONENT);
    ++iterCount;
    //number = number > addrsCount ? 0 : number + 1;
  }*/

  smsc_log_info(logger_, "test case stopped");
  smsc_log_info(logger_, "stopping pers client...");
  persClient_.Stop();
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
    smsc_log_warn(logger_, "request result: %s", exceptionReasons[call->status()]);
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
    smsc_log_info(logger_, "continue execution cx:%p, process time: %d ms", call, msprocTime);
    delete startTime;
  }
  if (call->status() == PERSCLIENTOK || call->status() == PROPERTY_NOT_FOUND) {
    ++successCalls_;
    procTime_ += procTime;
  } else {
    smsc_log_warn(logger_, "request result: %s", exceptionReasons[call->status()]);
    ++errorCalls_;
  }
  delete call;
}

/*
LongCallContextBase* LCPersClient::getCallContext(PersCallParams *callParams) {
  LongCallContextBase* context = new LongCallContextBase();
  context->initiator = this;
  context->setParams(callParams);
  return context;
}
 */

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
    currentSpeed_ = ok;
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
