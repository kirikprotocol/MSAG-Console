//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#include <memory>
#include <iostream>
#include "ProxySmeSMachine.h"
#include "ProxySmeQueue.h"
#include "ProxySmeMixer.h"
#if !defined _WIN32
#include "ProxySmeConfig.h"
#include "../system/smscsignalhandlers.h" 
#endif
#include "../logger/Logger.h"

#include "../admin/service/Component.h"
#include "../admin/service/Method.h"
#include "../admin/service/Type.h"
#include "../admin/service/ComponentManager.h"
#include "../admin/service/ServiceSocketListener.h"


SMSC_SMEPROXY_BEGIN

using namespace std;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;
using namespace smsc::admin::service;

class ProxySmeComponent : public Component, public SMachineNotifier
{
  Methods methods_;
  bool need_reloading_config_;
  enum { APPLAY_METHOD };
  void error(const char* method, const char* param) {}
public:
  ProxySmeComponent() : need_reloading_config_(false)
  {
    Parameters mhparams;
    Method mth(APPLAY_METHOD,"applay",mhparams,StringType);
    methods_[mth.getName()] = mth;
  }
  bool SMachineBreak_() { 
    bool x = need_reloading_config_; need_reloading_config_ = false; 
    return x;
  }
  virtual const char* const getName() const {return "ProxySme";}
  virtual const Methods& getMethods() const {return methods_;}
  virtual Variant call(const Method& method, const Arguments& args) throw (AdminException)
  {
    if ( method.getId() == APPLAY_METHOD ) {
      smsc::util::Logger::getCategory("smsc.proxysme").info("applay method was called");
      need_reloading_config_ = true;
      return Variant("");
    }
    return Variant("");
  }
};


bool LoadConfig(ProxyConfig& pconf)
{
  smsc::util::Logger::getCategory("smsc.proxysme").info("loading config");
#if defined _WIN32
  static const char* HOST = "smsc";
  static const int   PORT = 9992;
  pconf.quelimit = 100;
// LEFT
  pconf.left.host = HOST;
  pconf.left.port = PORT;
//  pconf.left.sid  = "SME_PROXY";
  pconf.left.sid  = "SME_PROXY_LEFT";
  pconf.left.password = "";
  pconf.left.origAddr = "0.1.999";
  pconf.left.systemType = "";
  pconf.left.timeOut = 100;
// RIGHT
  pconf.right.host = HOST;
  pconf.right.port = PORT;
//  pconf.right.sid  = "SME_PROXY";
  pconf.right.sid  = "SME_PROXY_RIGHT";
  pconf.right.password = "";
  pconf.right.origAddr = "0.1.998";
  pconf.right.systemType = "";
  pconf.right.timeOut = 100;

  return true;
#else
  try {
    ProxyConfig x_pconf;
    ProxySmeConfig cfg("config.xml");
    const ProxySmeConfig::sme& left = cfg.getLeft();
    x_pconf.left.host = left.host;
    x_pconf.left.port = left.port;
    x_pconf.left.sid  = left.sid;
    x_pconf.left.timeOut = left.timeOut;
    x_pconf.left.systemType = left.systemType;
    x_pconf.left.origAddr = left.origAddr;
    x_pconf.left.password = left.password;
    const ProxySmeConfig::sme& right = cfg.getRigth();
    x_pconf.right.host = right.host;
    x_pconf.right.port = right.port;
    x_pconf.right.sid  = right.sid;
    x_pconf.right.timeOut = right.timeOut;
    x_pconf.right.systemType = right.systemType;
    x_pconf.right.origAddr = right.origAddr;
    x_pconf.right.password = right.password;
    x_pconf.quelimit = 100;
    x_pconf.admin_host = cfg.getAdminHost();
    x_pconf.admin_port = cfg.getAdminPort();
    pconf = x_pconf;
    return true;
  }catch(exception& e) {
    smsc::util::Logger::getCategory("smsc.proxysme").error("can't laod config: %s",e.what());
    return false;
  }
#endif
}

#if !defined _WIN32
static SMachine* machine = 0;
void Stopper() {
  smsc::util::Logger::getCategory("smsc.proxysme").info("stop signal");
  if ( machine ) machine->Stop();
}
void SetStopper(SMachine* m){
  machine = 0;
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, smsc::system::SHUTDOWN_SIGNAL);
  sigset(smsc::system::SHUTDOWN_SIGNAL, (void(*)(int))Stopper);
  machine = m;
}
#else
void SetStopper(SMachine*){}
#endif

extern "C" 
int main()
{
  try {
    smsc::util::Logger::Init("proxysme.l4c");
    smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!!");
    smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!! -- Starting");
    smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!!");
  }catch( exception& ){
    cerr << "can't init logger" << endl;
    return -1;
  }

  ProxyConfig pconf;
  
  if ( !LoadConfig(pconf) ) {
    smsc::util::Logger::getCategory("smsc.proxysme").error("can't laod config, aborting process");
    return -1;
  }

  ProxySmeComponent component;
  ServiceSocketListener adminListener; 

  try {
    ComponentManager::registerComponent(&component); 
    adminListener.init(pconf.admin_host.c_str(),pconf.admin_port);               
    adminListener.Start();                                     
  }catch(exception& e){
    smsc::util::Logger::getCategory("smsc.proxysme").error("can't init admin interface: %s, skipped",e.what());
  }

  for (;;) { 
    Queue     que(pconf);
    Mixer     mixer(que,pconf);
    SMachine  smachine(que,mixer,pconf);
    SetStopper(&smachine);
  
    if ( smachine.ProcessCommands(component) == END_PROCESSING ) break;

    // перезачитаем конфиг
    if ( !LoadConfig(pconf) ) {
      smsc::util::Logger::getCategory("smsc.proxysme").error("can't relaod config, skipped");
    }
    SetStopper(0);
  }

  return 0;
}

SMSC_SMEPROXY_END

