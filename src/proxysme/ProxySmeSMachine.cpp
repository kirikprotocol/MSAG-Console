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
#include "../util/recoder/recode_dll.h"
#include <memory>
#include <cassert>
#include <vector>
#if !defined _WIN32
#include <time.h>
#endif
SMSC_SMEPROXY_BEGIN

using namespace std;
using namespace smsc::smpp;
using namespace smsc::core::synchronization;

void SMachine::Stop()
{
  if ( stopIt_ != IS_RUNNING ) return;
  stopIt_ = IS_STOPPING;
}

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
    timestruc_t tv={0,10000000L};
    nanosleep(&tv,0);
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

static void ConvertMsg_SMSC7BIT_to_LATIN1(PduXSm* p)
{
  int smlength = p->get_message().get_smLength();
  if ( smlength != 0 ) {
    const char* oldval = p->get_message().get_shortMessage();
    vector<char> buff(smlength+1);
    ConvertSMSC7BitToLatin1(oldval,smlength,&buff[0]);
    p->get_message().set_shortMessage(&buff[0],smlength);
  }
  if ( p->get_optional().has_messagePayload() ) {
    const char* oldval = p->get_optional().get_messagePayload();
    int pl_length = p->get_optional().size_messagePayload();
    vector<char> buff(pl_length+1);
    ConvertSMSC7BitToLatin1(oldval,pl_length,&buff[0]);
    p->get_optional().set_messagePayload(&buff[0],pl_length);
  }
}

static void ConvertMsg_SMSC7BIT_to_LATIN1(PduDataSm* p)
{
  if ( p->get_optional().has_messagePayload() ) {
    const char* oldval = p->get_optional().get_messagePayload();
    int pl_length = p->get_optional().size_messagePayload();
    vector<char> buff(pl_length+1);
    ConvertSMSC7BitToLatin1(oldval,pl_length,&buff[0]);
    p->get_optional().set_messagePayload(&buff[0],pl_length);
  }
}

static void ConvertMsg_LATIN1_to_SMSC7BIT(PduXSm* p)
{
  int smlength = p->get_message().get_smLength();
  if ( smlength != 0 ) {
    const char* oldval = p->get_message().get_shortMessage();
    vector<char> buff(smlength+1);
    ConvertLatin1ToSMSC7Bit(oldval,smlength,&buff[0]);
    p->get_message().set_shortMessage(&buff[0],smlength);
  }
  if ( p->get_optional().has_messagePayload() ) {
    const char* oldval = p->get_optional().get_messagePayload();
    int pl_length = p->get_optional().size_messagePayload();
    vector<char> buff(pl_length+1);
    ConvertLatin1ToSMSC7Bit(oldval,pl_length,&buff[0]);
    p->get_optional().set_messagePayload(&buff[0],pl_length);
  }
}

static void ConvertMsg_LATIN1_to_SMSC7BIT(PduDataSm* p)
{
  if ( p->get_optional().has_messagePayload() ) {
    const char* oldval = p->get_optional().get_messagePayload();
    int pl_length = p->get_optional().size_messagePayload();
    vector<char> buff(pl_length+1);
    ConvertLatin1ToSMSC7Bit(oldval,pl_length,&buff[0]);
    p->get_optional().set_messagePayload(&buff[0],pl_length);
  }
}

void SMachine::RecodeIfNeed(DIRECTION direct,SmppHeader* pdu) {
  if ( pdu->get_commandId() == SmppCommandSet::DELIVERY_SM  ) {
    PduXSm* p = (PduXSm*)pdu;
    if ( p->get_message().dataCoding == 0 && config_.left_dcs != config_.right_dcs ) {
      if ( direct == LEFT_TO_RIGHT ) {
        if ( config_.left_dcs == DEFAULTDCS_SMSC7BIT ) {
          ConvertMsg_SMSC7BIT_to_LATIN1(p);
        }else if ( config_.left_dcs == DEFAULTDCS_LATIN1 ) {
          ConvertMsg_LATIN1_to_SMSC7BIT(p);
        }else
          smsc_log_error(smsc::logger::Logger::getInstance("smsc.proxysme"), "invalid left default encoding value %x",config_.left_dcs);
      } else { // RIGHT_TO_LEFT
        if ( config_.right_dcs == DEFAULTDCS_SMSC7BIT ) {
          ConvertMsg_SMSC7BIT_to_LATIN1(p);
        }else if ( config_.right_dcs == DEFAULTDCS_LATIN1 ) {
          ConvertMsg_LATIN1_to_SMSC7BIT(p);
        }else
          smsc_log_error(smsc::logger::Logger::getInstance("smsc.proxysme"), "invalid right default encoding value %x",config_.right_dcs);
      }
    }
  } else if( pdu->get_commandId() == SmppCommandSet::DATA_SM ) {
    PduDataSm* p = (PduDataSm*)pdu;
    if ( p->get_data().dataCoding == 0 && config_.left_dcs != config_.right_dcs ) {
      if ( direct == LEFT_TO_RIGHT ) {
        if ( config_.left_dcs == DEFAULTDCS_SMSC7BIT ) {
          ConvertMsg_SMSC7BIT_to_LATIN1(p);
        }else if ( config_.left_dcs == DEFAULTDCS_LATIN1 ) {
          ConvertMsg_LATIN1_to_SMSC7BIT(p);
        }else
          smsc_log_error(smsc::logger::Logger::getInstance("smsc.proxysme"), "invalid left default encoding value %x",config_.left_dcs);
      } else { // RIGHT_TO_LEFT
        if ( config_.right_dcs == DEFAULTDCS_SMSC7BIT ) {
          ConvertMsg_SMSC7BIT_to_LATIN1(p);
        }else if ( config_.right_dcs == DEFAULTDCS_LATIN1 ) {
          ConvertMsg_LATIN1_to_SMSC7BIT(p);
        }else
          smsc_log_error(smsc::logger::Logger::getInstance("smsc.proxysme"), "invalid right default encoding value %x",config_.right_dcs);
      }
    }
  }
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
        smsc_log_error(smsc::logger::Logger::getInstance("smsc.proxysme"), "can't connect left/right smscs");
        MacroSleep();
      }
      continue;
    }
    auto_ptr<QCommand> qcmd ( que_.Next() );
    if ( qcmd.get() ) {
      // процессим команду
      DumpCommand(qcmd->direction_,qcmd->pdu_);
      RecodeIfNeed(qcmd->direction_,qcmd->pdu_);
      switch ( qcmd->pdu_->get_commandId() ) {
      case SmppCommandSet::DATA_SM:
        mixer_.SendPdu(qcmd->direction_,qcmd->pdu_);
        break;
      case SmppCommandSet::DATA_SM_RESP:
        ((PduDataSmResp*)qcmd->pdu_)->set_messageId("");
        mixer_.SendPdu(qcmd->direction_,qcmd->pdu_);
        break;
      case SmppCommandSet::DELIVERY_SM:
        TranslateToSubmitAndSend(qcmd->direction_,qcmd->pdu_);
        break;
      case SmppCommandSet::SUBMIT_SM_RESP:
        TranslateToDeliverRespAndSend(qcmd->direction_,qcmd->pdu_);
        break;
      case SmppCommandSet::ENQUIRE_LINK:
      case SmppCommandSet::ENQUIRE_LINK_RESP:
        mixer_.SendPdu(qcmd->direction_,qcmd->pdu_);
        break;
      default:
        smsc_log_warn(smsc::logger::Logger::getInstance("smsc.proxysme"), "invalid cmdid=%x in process command",qcmd->pdu_->get_commandId());
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
  ((PduDeliverySmResp*)pdu)->set_messageId("");
  mixer_.SendPdu(direct,pdu);
}

void SMachine::DumpCommand(DIRECTION direct,SmppHeader* pdu)
{

}

SMSC_SMEPROXY_END
