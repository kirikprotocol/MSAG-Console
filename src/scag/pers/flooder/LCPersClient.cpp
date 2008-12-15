/* $Id$ */
#include <vector>
#include <stdlib.h>
#include "LCPersClient.h"
#include "scag/pers/util/Property.h"
#include "util/timeslotcounter.hpp"
#include "util/sleep.h"

namespace scag2 { namespace pers { namespace util {
void LCPersClient::execute(int addrsCount, int getsetCount) {

  smsc_log_info(logger_, "execution...");

  while (persClient_.getClientStatus() != 0) {
    smsc_log_warn(logger_, "waiting while pers client connecting to server...");
    sleep(1);
  }
  //int addrsCount = 1000000;
  char addr[20];

  int number = 0;
  while (!isStopped_) {
    number = (rand() * RAND_MAX + 1) | (rand() + 1);
    number = number % addrsCount;
    sprintf(addr, "791%08d", number);			
    commandsSetConfigured(addr, number, "test_abnt_prop", PT_ABONENT, getsetCount);
    //commandsSet(addr, number, "test_serv_prop", PT_PROVIDER);
    //number = number > addrsCount ? 0 : number + 1;
  }

  smsc_log_info(logger_, "test case stopped");
  smsc_log_info(logger_, "stopping pers client...");
  persClient_.Stop();
}

void LCPersClient::doCall(LongCallContextBase* context) {
  int status = persClient_.getClientStatus();
  if (status == CLIENT_BUSY) {
    smsc_log_warn(logger_, "pers client busy!");
    delete context;
    if (busyRejects_ > maxRejects_) {
      //throw PersClientException(CLIENT_BUSY);
    }
    ++busyRejects_;
    return;
  }
  if (status == NOT_CONNECTED) {
    delete context;
    throw PersClientException(NOT_CONNECTED);
  }
  busyRejects_ = busyRejects_ > 0 ? busyRejects_ - 1 : 0;
  persClient_.call(context);
  ++callsCount_;
  smsc_log_debug(logger_, "call cx:%p calls count %d", context, callsCount_);
  delay();
}

void LCPersClient::continueExecution(LongCallContextBase* context, bool dropped) {
  --callsCount_;
  smsc_log_debug(logger_, "continue execution cx:%p, calls count %d", context, callsCount_);
  PersCallParams *callParams = static_cast<PersCallParams *>(context->getParams());
  if (callParams->status() != 0) {
    smsc_log_debug(logger_, "error long call result: %s", strs[callParams->status()]);
  }
  delete context;
}

LongCallContextBase* LCPersClient::getCallContext(PersCallParams *callParams) {
  LongCallContextBase* context = new LongCallContextBase();
  context->initiator = this;
  context->setParams(callParams);
  return context;
}

PersCallParams* LCPersClient::getPersPersCallParams(ProfileType pfType, const string& addr, int intKey, std::auto_ptr<PersCommand> cmd){
  startTime_ = gethrtime();
  PersCallParams *callParams = new PersCallParams(pfType, cmd);
  if (pfType == PT_ABONENT) {
    callParams->setKey(addr);
  } else {
    callParams->setKey(intKey);
  }
  return callParams;
}

void LCPersClient::delay() {
  hrtime_t procTime = gethrtime() - startTime_;
  //procTime += procTime / 10;
  procTime /= 1000;
  if (delay_ > procTime + overdelay_) {
    startTime_ = gethrtime();
    unsigned sleepTime = delay_ - procTime - overdelay_;
    __trace2__("try to sleep:%d", sleepTime);
    millisleep(sleepTime);
    overdelay_ = (gethrtime() - startTime_) / 1000 - sleepTime;
  } else {
    __trace2__("overdelay:%d", overdelay_);
    overdelay_ -= delay_;
    if (overdelay_ < 0) overdelay_ = 0;
  }
}

void LCPersClient::commandsSetConfigured(const string& addr, int intKey, const string& propName, ProfileType pfType, int cmdsCount) {
  string setStrName = propName + "_set_str_";
  string setVal("test_string_value_");
  char suffix[5];
  for (int i = 0; i < cmdsCount; ++i) {
    sprintf(suffix,"%04d", i + 1);
    string setStrName(propName + "_set_str_");
    setStrName.append(suffix);
    string setVal("test_string_value_");
    setVal.append(suffix);
    std::auto_ptr<PersCommand> cmd1( getCmd(setStrName) );
    doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd1)) );  
    std::auto_ptr<PersCommand> cmd2( setCmd(setStrName, setVal) );
    doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd2)) );  
  }
}

void LCPersClient::commandsSet(const string& addr, int intKey, const string& propName, ProfileType pfType) {

  string setStrName = propName + "_set_str";
  std::auto_ptr<PersCommand> cmd1( setCmd(setStrName, "test_string_value") );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd1)) );  

  string setIntName = propName + "_set_int";
  std::auto_ptr<PersCommand> cmd2( setCmd(setIntName,"", 100) );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd2)) );  

  std::auto_ptr<PersCommand> cmd3( getCmd(setStrName) );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd3)) );  

  std::auto_ptr<PersCommand> cmd4( getCmd(setIntName) );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd4)) );  
/*
  string incName = propName + "_inc";
  int incValue = 10;
  std::auto_ptr<PersCommand> cmd5( incCmd(incName, incValue) );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd5)) );  

  std::auto_ptr<PersCommand> cmd6( incCmd(incName, incValue) );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd6)) );  

  string incModName = propName + "_inc_mod";
  int modValue = 3;
  std::auto_ptr<PersCommand> cmd7( incModCmd(incModName, incValue, modValue) );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd7)) );  

  std::auto_ptr<PersCommand> cmd8( getCmd(incName) );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd8)) );  

  std::auto_ptr<PersCommand> cmd9( getCmd(incModName) );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd9)) );  

  std::auto_ptr<PersCommand> cmd10( batchCmd(propName, true) );
  doCall( getCallContext(getPersPersCallParams(pfType,addr,intKey,cmd10)) );  
*/
}

PersCommandSingle* LCPersClient::getCmd(const string& propName) {
  PersCommandSingle* persCmd = new PersCommandSingle(creator, scag::pers::util::perstypes::PC_GET);
  Property& prop = persCmd->property();
  prop.setName(propName);
  return persCmd;
}

PersCommandSingle* LCPersClient::setCmd(const string& propName, const string& strVal, int intVal) {
  PersCommandSingle* persCmd = new PersCommandSingle(creator, scag::pers::util::perstypes::PC_SET);
  Property& prop = persCmd->property();
  prop.setName(propName);
  if (strVal.empty()) {
    prop.setIntValue(intVal);
  } else {
    prop.setStringValue(strVal.c_str());
  }
  return persCmd;
}

PersCommandSingle* LCPersClient::incCmd(const string& propName, int inc) {
  PersCommandSingle* persCmd = new PersCommandSingle(creator, scag::pers::util::perstypes::PC_INC_RESULT);
  Property& prop = persCmd->property();
  prop.setName(propName);
  prop.setIntValue(inc);
  return persCmd;
}

PersCommandSingle* LCPersClient::incModCmd(const string& propName, int inc, int mod) {
  PersCommandSingle* persCmd = new PersCommandSingle(creator, scag::pers::util::perstypes::PC_INC_RESULT);
  Property& prop = persCmd->property();
  prop.setName(propName);
  prop.setIntValue(inc);
  persCmd->setResult(mod);
  return persCmd;
}

PersCommandBatch* LCPersClient::batchCmd(const string& propName, bool trans) {
  string name = propName + "_batch";
  std::vector<PersCommandSingle> cmds;
  PersCommandSingle* cmd = setCmd(name,"", 100);
  cmds.push_back(*cmd);
  delete cmd;
  cmd = incCmd(name, 10);
  cmds.push_back(*cmd);
  delete cmd;
  cmd = incCmd(name, 10);
  cmds.push_back(*cmd);
  delete cmd;
  cmd = incModCmd(name, 15, 3);
  cmds.push_back(*cmd);
  delete cmd;
  cmd = incModCmd(name, 34, 10);
  cmds.push_back(*cmd);
  delete cmd;
  cmd = getCmd(name);
  cmds.push_back(*cmd);
  delete cmd;
  return new PersCommandBatch(creator, cmds, trans);
}

bool LCPersClient::canStop() {
  return callsCount_ > 0 ? false : true;
}

void LCPersClient::shutdown() {
  isStopped_ = true;
}






}}}
