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

/// обработчик событий на SMPP трансмиттере
class PduListener: public SmppPduEventListener
{
  log4cpp::Category&      log_;
  DIRECTION               incom_dirct_; /// направление дл€ команд
  Queue&                  que_;         /// очередь команд
public:
  PduListener(DIRECTION,Queue&,log4cpp::Category&);
  virtual ~PduListener();
  virtual void handleEvent(SmppHeader *pdu);
  virtual void handleError(int errorCode);
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
public:
  Mixer(Queue& que,const ProxyConfig&);
  ~Mixer();
  bool Connect();
  bool Disconnect();
  bool Reconnect();
  /// посылает пакет , если успешно то овнершип уходит транспартной системе 
  bool SendPdu(DIRECTION,SmppHeader** pdu);
private:
  Mixer(const Mixer&);
  Mixer& operator=(const Mixer&);
};

SMSC_SMEPROXY_END

#endif // PROXY_SME_MIXER_HEADER

