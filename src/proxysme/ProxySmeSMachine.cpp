//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#include "ProxySmeSMachine.h"
#include <memory>

SMSC_SMEPROXY_BEGIN

using namespace std;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;

void SMachine::Stop()
{
  if ( stopIt_ != IS_RUNNING ) return;
  stopIt_ = IS_STOPPING;
  while ( stopIt_ != IS_STOPPED ) 
#if defined _WIN32
    Sleep(100);
#else
    sleep(1);
#endif
}

//void SMachine::Start()
//{
//  Thread::Start();
//  stopIt_ = IS_STOPPED;
//}

SMachine::SMachine(Queue& que,Mixer& mixer,const ProxyConfig& pconf) 
:config_(pconf), que_(que), mixer_(mixer)
{
  stopIt_ = IS_STOPPED;
}

SMachine::~SMachine()
{
}

static void MicroSleep() 
{
#if defined _WIN32
  Sleep(10);
#else
#endif
}

void SMachine::ProcessCommands()
{
  stopIt_ = IS_RUNNING;
  while (stopIt_ == IS_RUNNING) {
    auto_ptr<QCommand> qcmd ( que_.Next() );
    if ( qcmd.get() != 0 ) {
      // процессим команду
    }
    MicroSleep();
  }
}

SMSC_SMEPROXY_END
