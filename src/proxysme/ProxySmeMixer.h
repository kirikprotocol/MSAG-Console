//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#if !defined PROXY_SME_MIXER_HEADER
#define PROXY_SME_MIXER_HEADER

#include "proxysme.h"
#include "ProxySmeQueue.h"
#include <deque>
#include <stdexcept>
#include <memory>
#include "../smpp/smpp.h"
#include "../sme/SmppBase.hpp"
#include "../core/synchronization/Mutex.hpp"

SMSC_SMEPROXY_BEGIN

using namespace smsc::smpp;
using namespace smsc::sme;
using namespace smsc::core::synchronization;

class Mixer;

/// обработчик событий на SMPP трансмиттере
class PduListener: public SmppPduEventListener
{
  log4cpp::Category&      log_;
  DIRECTION               incom_dirct_; /// направление дл€ команд
  Queue&                  que_;         /// очередь команд
  SmppSession*            trx_;
  Mixer*                  mixer_;
public:
  PduListener(DIRECTION,Queue&,log4cpp::Category&);
  virtual ~PduListener();
  virtual void handleEvent(SmppHeader *pdu);
  virtual void handleError(int errorCode);
  void SetSession(SmppSession* s, Mixer* mixer) { trx_ = s; mixer_ = mixer; }
};

enum SESSION_STATE {
  SESSION_OK,
  SESSION_BROKEN,
  SESSION_DISCONNECTED,
  SESSION_UNRECOVERABLE
};

/// класс призвод€щий прием отправку SMPP пакетов
/// и управл€ющий трансмиттерами
class Mixer { 
  const ProxyConfig&      config_;
  log4cpp::Category&      log_;
  Queue&                  que_;           /// очереь событий
  PduListener             listen_left_;   /// листенер левого трансмиттера
  PduListener             listen_right_;  /// листенер правого трансмиттера
  auto_ptr<SmppSession>   left_;          /// левый трансмиттер
  auto_ptr<SmppSession>   right_;         /// правый трансмиттер
  SESSION_STATE           left_state_,right_state_;
public:
  Mixer(Queue& que,const ProxyConfig&);
  ~Mixer();
  bool Connect();
  bool Disconnect();
  bool Reconnect();
  /// посылает пакет 
  bool SendPdu(DIRECTION,SmppHeader* pdu);
  bool IsUnrecoverable() {
    return left_state_ == SESSION_UNRECOVERABLE ||right_state_ == SESSION_UNRECOVERABLE;
  }
  bool IsConnected() {
    return left_state_ == SESSION_OK ||right_state_ == SESSION_OK;
  }
  void BrokenSession_(DIRECTION d) {
    if ( d == LEFT_TO_RIGHT ) left_state_ = SESSION_BROKEN;
    else if ( d == RIGHT_TO_LEFT ) right_state_ = SESSION_BROKEN;
  }
private:
  Mixer(const Mixer&);
  Mixer& operator=(const Mixer&);
};

SMSC_SMEPROXY_END

#endif // PROXY_SME_MIXER_HEADER

