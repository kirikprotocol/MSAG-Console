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
#include "../logger/Logger.h"

SMSC_SMEPROXY_BEGIN

using namespace std;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;

bool LoadConfig(ProxyConfig& pconf)
{
  static const char* HOST = "smsc";
  static const int   PORT = 6001;
  pconf.quelimit = 100;
// LEFT
  pconf.left.host = HOST;
  pconf.left.port = PORT;
//  pconf.left.sid  = "SME_PROXY";
  pconf.left.sid  = "test_1";
  pconf.left.password = "";
  pconf.left.origAddr = "";
  pconf.left.systemType = "";
  pconf.left.timeOut = 100;
// RIGHT
  pconf.right.host = HOST;
  pconf.right.port = PORT;
//  pconf.right.sid  = "SME_PROXY";
  pconf.right.sid  = "test_2";
  pconf.right.password = "";
  pconf.right.origAddr = "";
  pconf.right.systemType = "";
  pconf.right.timeOut = 100;

  return true;
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
  
  if ( !mixer.Connect() ) {
    smsc::util::Logger::getCategory("smsc.proxysme").error("can't connect to smsc, aborting process");
    return -1;
  }

  smachine.ProcessCommands();

  return 0;
}

SMSC_SMEPROXY_END

