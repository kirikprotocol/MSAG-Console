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
#if defined _WIN32
#include "ProxySmeConfig.h"
#endif
#include "../logger/Logger.h"

SMSC_SMEPROXY_BEGIN

using namespace std;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;

bool LoadConfig(ProxyConfig& pconf)
{
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
    ProxySmeConfig cfg("proxysme.cnf");
    const ProxySmeConfig::sme& left = cfg.getLeft();
    pconf.left.host = left.host;
    pconf.left.port = left.port;
    pconf.left.sid  = left.sid;
    pconf.left.timeOut = left.timeOut;
    pconf.left.systemType = left.systemType;
    pconf.left.origAddr = left.origAddr;
    pconf.left.password = left.password;
    const ProxySmeConfig::sme& right = cfg.getRight();
    pconf.right.host = right.host;
    pconf.right.port = right.port;
    pconf.right.sid  = right.sid;
    pconf.right.timeOut = right.timeOut;
    pconf.right.systemType = right.systemType;
    pconf.right.origAddr = right.origAddr;
    pconf.right.password = right.password;
    pconf.quelimit = 100;
  }catch(exception& e) {
    smsc::util::Logger::getCategory("smsc.proxysme").error("can't laod config: %s",e.what());
  }
#endif
}

extern "C" 
int main()
{
  smsc::util::Logger::Init("proxysme.l4c");
  smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!!");
  smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!! -- Starting");
  smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!!");
  
  ProxyConfig pconf;

  if ( !LoadConfig(pconf) ) {
    smsc::util::Logger::getCategory("smsc.proxysme").error("can't laod config, aborting process");
    return -1;
  }
  
  Queue     que(pconf);
  Mixer     mixer(que,pconf);
  SMachine  smachine(que,mixer,pconf);
  smachine.ProcessCommands();

  return 0;
}

SMSC_SMEPROXY_END

