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

extern "C" 
int main()
{
  smsc::util::Logger::Init("proxysme.l4c");
  smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!!");
  smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!! -- Starting");
  smsc::util::Logger::getCategory("smsc.proxysme.X").info("!!!!");
  
  Queue     que;
  Mixer     mixer(que);
  SMachine  smachine(que,mixer);
  
  if ( !mixer.Connect() ) {
    smsc::util::Logger::getCategory("smsc.proxysme").error("can't connect to smsc, aborting process");
  }

  smachine.ProcessCommands();
  return 0;
}

SMSC_SMEPROXY_END

