//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#include "ProxySmeMixer.h"
#include <string>

SMSC_SMEPROXY_BEGIN

using namespace smsc::util;
using namespace std;

static string ToString(DIRECTION d) {
  return
   (d==LEFT_TO_RIGHT)?"LEFT_TO_RIGHT":
   (d==RIGHT_TO_LEFT)?"RIGHT_TO_LEFT":
   "IVALID_DIRECTION";
}

Mixer::Mixer(Queue& que,const ProxyConfig& pconf) :
config_(pconf),
que_(que),
log_(Logger::getCategory("smsc.proxysme.mixer")),
listen_left_(LEFT_TO_RIGHT,que,log_), listen_right_(RIGHT_TO_LEFT,que,log_)
{
  left_state_ = right_state_ = SESSION_DISCONNECTED;
  log_.info("Mixer::ctor");
  left_  = auto_ptr<SmppSession>(new SmppSession(config_.left,&listen_left_));
  listen_left_.SetSession(left_.get(),this);
  right_ = auto_ptr<SmppSession>(new SmppSession(config_.right,&listen_right_));
  listen_right_.SetSession(right_.get(),this);
}

Mixer::~Mixer()
{
  listen_left_.SetSession(0,this);
  listen_right_.SetSession(0,this);
  log_.info("Mixer::dtor");
}

bool Mixer::Connect()
{
  log_.info("Mixer::Connect: connecting...");
  if ( left_state_ == SESSION_BROKEN || right_state_ == SESSION_BROKEN ) {
    left_->close();
    left_state_ = SESSION_DISCONNECTED;
    right_->close();
    right_state_ = SESSION_DISCONNECTED;
  }
// LEFT PROXY CONNECTION
  if ( left_state_ != SESSION_OK ) {
    try {
      left_state_ = SESSION_OK;
      left_->connect();
    }catch(SmppConnectException& e) {
      log_.error("<exception> on connect : %s",e.what());
      switch ( e.getReason() ) {
        case SmppConnectException::Reason::bindFailed:
        case SmppConnectException::Reason::smppError:
          left_state_ = SESSION_UNRECOVERABLE;
          log_.error(" left proxy connection is unrecoverable ");
          break;
        default:
          left_state_ = SESSION_DISCONNECTED;
      }
      return false;
    }catch( exception& _ ){
      log_.error("<exception> on connect : %s",_.what());
      left_state_ = SESSION_DISCONNECTED;
      return false;
    }
  }
// RIGHT PROXY CONNECTION
  if ( right_state_ != SESSION_OK ) {
    try {
      right_state_ = SESSION_OK;
      right_->connect();
    }catch(SmppConnectException& e) {
      log_.error("<exception> on connect : %s",e.what());
      switch ( e.getReason() ) {
        case SmppConnectException::Reason::bindFailed:
        case SmppConnectException::Reason::smppError:
          right_state_ = SESSION_UNRECOVERABLE;
          log_.error(" left proxy connection is unrecoverable ");
          break;
        default:
          right_state_ = SESSION_DISCONNECTED;
      }
      return false;
    }catch( exception& _ ){
      log_.error("<exception> on connect : %s",_.what());
      right_state_ = SESSION_DISCONNECTED;
      return false;
    }
  }
  log_.info("Mixer::Connect: sucess");
  return true;
}

bool Mixer::Disconnect()
{
  log_.info("Mixer::Disconnect: ...");
  return true;
}

bool Mixer::Reconnect()
{
  log_.info("Mixer::Reconnect: ...");
  Connect();
  return true;
}

/// посылает пакет
bool Mixer::SendPdu(DIRECTION direct,SmppHeader* pdu)
{
  log_.info("Mixer::SendPdu: %s ",ToString(direct).c_str());
  try {
    switch ( direct ) {
    case LEFT_TO_RIGHT:
      if ( pdu->get_commandId() == SmppCommandSet::DELIVERY_SM_RESP )
        right_->getAsyncTransmitter()->sendDeliverySmResp(*(PduDeliverySmResp*)pdu);
      else
        right_->getAsyncTransmitter()->sendPdu(pdu);
      break;
    case RIGHT_TO_LEFT:
      if ( pdu->get_commandId() == SmppCommandSet::DELIVERY_SM_RESP )
        left_->getAsyncTransmitter()->sendDeliverySmResp(*(PduDeliverySmResp*)pdu);
      else
        left_->getAsyncTransmitter()->sendPdu(pdu);
      break;
    default:
      log_.error("Mixer::SendPdu: invalid direction ");
      return false;
    }
    return true;
  }catch(exception& _){
    log_.error("Mixer::SendPdu: <exception> %s",_.what());
    return false;
  }
  return true;
}

PduListener::PduListener(DIRECTION d,Queue& que,log4cpp::Category& log) :
log_(log),incom_dirct_(d), que_(que)
{
  log_.info("PduListener::ctor");
}

PduListener::~PduListener()
{
  log_.info("PduListener::dtor");
}

void PduListener::handleEvent(SmppHeader *pdu)
{
  log_.info("PduListener::handleEvent: %s",ToString(incom_dirct_).c_str());
  auto_ptr<QCommand> qc(new QCommand);
  qc->direction_ = incom_dirct_;
  qc->pdu_       = pdu;
  // пытаемся запихать команду в очередь
  if ( que_.PutBack(qc.get()) ) {
    // О! запихалась :)
    qc.release();
  }else{
    log_.error("PduListener::handleEvent: %s ,packet skipped bcause queue is full",ToString(incom_dirct_).c_str());
    // упс, перегруз однако! Посылаем клиенту ошибку
    try{
      if ( pdu->get_commandId()==SmppCommandSet::DELIVERY_SM ) {
        // если это был деливер, то сообщаем что отправка не удалась
        PduDeliverySmResp resp;
        resp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
        resp.get_header().set_commandStatus(SmppStatusSet::ESME_RMSGQFUL);
        resp.set_messageId("");
        resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
        trx_->getAsyncTransmitter()->sendDeliverySmResp(resp);
      }else{
        // Хм, что это было????
        log_.error("PduListener::handleEvent: %s broken on non DELIVER unqueued pdu",ToString(incom_dirct_).c_str());
      }
    }catch(exception& _) {
      log_.error("PduListener::handleEvent: %s <exception> : %s",ToString(incom_dirct_).c_str(),_.what());
    }
  }
}

void PduListener::handleError(int errorCode)
{
  log_.info("PduListener::handleError: %s",ToString(incom_dirct_).c_str());
  if ( errorCode == smppErrorNetwork ) {
    mixer_->BrokenSession_(incom_dirct_);
  }
}


SMSC_SMEPROXY_END
