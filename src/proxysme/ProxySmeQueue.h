//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#if !defined PROXY_SME_QUUE_HEADER
#define PROXY_SME_QUUE_HEADER

#include "proxysme.h"
#include <deque>
#include <stdexcept>
#include <memory>
#include "../smpp/smpp.h"
#include "../core/synchronization/Mutex.hpp"

SMSC_SMEPROXY_BEGIN

using namespace smsc::smpp;
using namespace smsc::core::synchronization;


enum DIRECTION { LEFT_TO_RIGHT, RIGHT_TO_LEFT, INVALID_DIRECTION };

/// команда
struct QCommand {
  DIRECTION   direction_;     /// направление команды
  SmppHeader* pdu_;           /// проксируемый пакет
  QCommand() : direction_(INVALID_DIRECTION), pdu_(0) {}
  ~QCommand() { if ( pdu_ ) disposePdu(pdu_); }
private:
  QCommand(const QCommand&);
  QCommand& operator = (const QCommand&);
};

/// исключение, бросается при превышении лимита на очереди
struct QueueLimitExceeded : public std::runtime_error {
  QueueLimitExceeded(const char* text = "SPQ::QueueLimitExceeded") :
    runtime_error(text) {}
};

/// очередь команд 
class Queue {

  typedef std::deque<QCommand*> QUEUE; /// контейнер команд для очереди

  int   qlimit_;  /// предел длины очереди
  QUEUE queue_;   /// очередь команд
  Mutex mutex_;   /// обьект синхронизации

public:

  /// устанавливает лимит на длину очереди
  void SetLimit(int limit);
  /// помещает команду в конец очереди (овнершип переходит к очереди)
  bool PutBack(QCommand** cmd);
  /// возвращает указатель на первую команду в очереди 
  /// (овнершип остается у очереди)
  QCommand* Top();
  /// возвращает указатель на первую команду в очереди и удаляет ее из очереди 
  /// (овнершип переходит клиенту)
  QCommand* Next();
  /// удаляет команду из очереди и уничтожает обьект - команду
  bool Pop(); 

  Queue(const ProxyConfig&);
  ~Queue();

};

SMSC_SMEPROXY_END


#endif // PROXY_SME_QUUE_HEADER



