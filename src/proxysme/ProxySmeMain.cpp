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
#include "ProxySmeSMachine.h"
#include "ProxySmeQueue.h"
#include "ProxySmeMixer.h"
#if !defined _WIN32
#include "ProxySmeConfig.h"
#endif
#include "../logger/Logger.h"

#include "../admin/service/Component.h"
#include "../admin/service/Method.h"
#include "../admin/service/Type.h"


SMSC_SMEPROXY_BEGIN

using namespace std;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;

class ProxySmeComponent : public Component, public SMachineNotifier
{
  Methods methods_;
  bool need_reloading_config_
  enum { APPLAY_METHOD };
  void error(const char* method, const char* param) {}
public:
  ProxySmeComponent() : need_reloading_config_(false)
  {
    methods[APPLAY_METHOD] = Method(APPLAY_METHOD,"applay",mhparams,StringType);
  }
  bool SMachineBreak_() { 
    bool x = need_reloading_config_; need_reloading_config_ = false; 
    return x;
  }
  virtual const char* const getName() const {return "ProxySme";}
  virtual const Methods& getMethods() const {return methods_;}
  virtual Variant call(const Method& method, const Arguments& args)
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

extern "C" 
int main()
{
  try {
    smsc::util::Logger::Init("proxysme.l4c");
    smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!!");
    smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!! -- Starting");
    smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!!");
  }catch( exception& e ){
    cerr << "can't init logger" << endl;
    return -1;
  }

  ProxyConfig pconf;
  
  if ( !LoadConfig(pconf) ) {
    smsc::util::Logger::getCategory("smsc.proxysme").error("can't laod config, aborting process");
    return -1;
  }

  try {
    ProxySmeAdminComponent component;
    ComponentManager::registerComponent(&component); 
    adminListener.init(pconf.admin_host,pconf.admin_port);               
    adminListener.Start();                                     
  }catch(exception& e){
    smsc::util::Logger::getCategory("smsc.proxysme").error("can't init admin interface, skipped");
  }

  for (;;) { 
    Queue     que(pconf);
    Mixer     mixer(que,pconf);
    SMachine  smachine(que,mixer,pconf);
  
    if ( smachine.ProcessCommands() == END_PROCESSING ) break;

    // перезачитаем конфиг
    if ( !LoadConfig(pconf) ) {
      smsc::util::Logger::getCategory("smsc.proxysme").error("can't relaod config, skipped");
    }
  }

  return 0;
}

SMSC_SMEPROXY_END

