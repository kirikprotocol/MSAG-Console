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
#include <cassert>

SMSC_SMEPROXY_BEGIN

using namespace std;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;

void SMachine::Stop()
{
  if ( stopIt_ != IS_RUNNING ) return;
  stopIt_ = IS_STOPPING;
//  while ( stopIt_ != IS_STOPPED ) 
//#if defined _WIN32
//    Sleep(100);
//#else
//    sleep(1);
//#endif
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

/// ждет несколько миллисекунд
void MicroSleep() 
{
#if defined _WIN32
  Sleep(10);
#else
#endif
}

/// ждет несколько секунд
void MacroSleep() 
{
#if defined _WIN32
  Sleep(3000);
#else
  sleep(3);
#endif
}

/// обрабатывает комманды поступающие в очередь из SMPP API
unsigned SMachine::ProcessCommands(SMachineNotifier& notifier)
{
  stopIt_ = IS_RUNNING;
  while (stopIt_ == IS_RUNNING) {
    if ( notifier.SMachineBreak_()) return BREAK_PROCESSING;
    if ( !mixer_.IsConnected() ) {
      if ( mixer_.IsUnrecoverable() ) return END_PROCESSING;
      if ( !mixer_.Connect() ) {
        smsc::util::Logger::getCategory("smsc.proxysme").error("can't connect left/right smscs");
        MacroSleep();
      }
      continue;
    }
    auto_ptr<QCommand> qcmd ( que_.Next() );
    if ( qcmd.get() ) {
      // процессим команду
      switch ( qcmd->pdu_->get_commandId() ) {
      case SmppCommandSet::DELIVERY_SM:
        TranslateToSubmitAndSend(qcmd->direction_,qcmd->pdu_);
        break;
      case SmppCommandSet::SUBMIT_SM_RESP:
        TranslateToDeliverRespAndSend(qcmd->direction_,qcmd->pdu_);
        break;
      default:
        ;// упс, скиппед
      }
    }
    MicroSleep();
  }
  stopIt_ = IS_STOPPED;
  return END_PROCESSING;
}

/// транслирует пакет DELIVER_SM -> SUBMIT_SM
/// и отправляет адресату
void SMachine::TranslateToSubmitAndSend(DIRECTION direct, SmppHeader* pdu)
{
  assert(pdu->get_commandId() == SmppCommandSet::DELIVERY_SM);
  // вспоминаем что DELIVERY_SM и SUBMIT_SM реализованы через одну и тже структуру данных
  pdu->set_commandId(SmppCommandSet::SUBMIT_SM);
  mixer_.SendPdu(direct,pdu);
}

/// транслирует пакет SUBMIT_SM_RESP -> DELIVER_SM_RESP
/// и отправляет адресату
void SMachine::TranslateToDeliverRespAndSend(DIRECTION direct,SmppHeader* pdu)
{
  assert(pdu->get_commandId() == SmppCommandSet::SUBMIT_SM_RESP);
  // вспоминаем что DELIVERY_SM_RESP и SUBMIT_SM_RESP реализованы через одну и тже структуру данных
  pdu->set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
  mixer_.SendPdu(direct,pdu);
}

SMSC_SMEPROXY_END
