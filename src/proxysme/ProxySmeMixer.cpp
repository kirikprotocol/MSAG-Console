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
  log_.info("Mixer::ctor");
  left_  = auto_ptr<SmppSession>(new SmppSession(config_.left,&listen_left_));
  listen_left_.SetSession(left_.get());
  right_ = auto_ptr<SmppSession>(new SmppSession(config_.right,&listen_right_));
  listen_right_.SetSession(right_.get());
}

Mixer::~Mixer()
{
  listen_left_.SetSession(0);
  listen_right_.SetSession(0);
  log_.info("Mixer::dtor");
}

bool Mixer::Connect()
{
  log_.info("Mixer::Connect: connecting...");
  try {
    left_->connect();
    right_->connect();
  }catch( exception& _ ){
    log_.error("<exception> on connect : %s",_.what());
    return false;
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
  return true;
}

/// посылает пакет
bool Mixer::SendPdu(DIRECTION direct,SmppHeader* pdu)
{
  log_.info("Mixer::SendPdu: %s ",ToString(direct).c_str());
  try {
    switch ( direct ) {
    case LEFT_TO_RIGHT:  right_->getAsyncTransmitter()->sendPdu(pdu);  break;
    case RIGHT_TO_LEFT:  left_->getAsyncTransmitter()->sendPdu(pdu); break;
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
}


SMSC_SMEPROXY_END
