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

Mixer::Mixer(Queue& que) : 
que_(que), 
log_(Logger::getCategory("smsc.proxysme.mixer")),
listen_left_(LEFT_TO_RIGHT,que,log_), listen_right_(RIGHT_TO_LEFT,que,log_)
{
  log_.info("Mixer::ctor");
  SmeConfig bind_info;
  left_  = auto_ptr<SmppSession>(new SmppSession(bind_info,&listen_left_));
  right_ = auto_ptr<SmppSession>(new SmppSession(bind_info,&listen_right_));
}

Mixer::~Mixer()
{
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

/// посылает пакет , если успешно то овнершип уходит транспартной системе 
bool Mixer::SendPdu(DIRECTION direct,SmppHeader** pdu)
{
  log_.info("Mixer::SendPdu: %s ",ToString(direct).c_str());
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
  log_.info("PduListener::handleEvent: %s",ToString(incom_dirct_));
}

void PduListener::handleError(int errorCode)
{
  log_.info("PduListener::handleError: %s",ToString(incom_dirct_));
}


SMSC_SMEPROXY_END
