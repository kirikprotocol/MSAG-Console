//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#include "ProxySmeQueue.h"
#include <cassert>
#include <algorithm>

SMSC_SMEPROXY_BEGIN

using namespace smsc::smpp;
using namespace smsc::core::synchronization;
using namespace std;

/// устанавливает лимит на длину очереди
void Queue::SetLimit(int limit)
{
  assert (limit > 0); if ( limit <= 0 ) return;
  MutexGuard guard(mutex_);
  qlimit_ = limit;
}

/// помещает команду в конец очереди (овнершип переходит к очереди)
bool Queue::PutBack(QCommand** cmd)
{
  MutexGuard guard(mutex_);
  if ( queue_.size() >= unsigned(qlimit_) ) return false;
  assert( cmd && *cmd ); if ( !cmd || !*cmd ) return false;
  queue_.push_back(*cmd);
  *cmd = 0;
  return true;
}

/// возвращает указатель на первую команду в очереди 
/// (овнершип остается у очереди)
QCommand* Queue::Top()
{
  MutexGuard guard(mutex_);
  return queue_.front();
}

/// возвращает указатель на первую команду в очереди и удаляет ее из очереди 
/// (овнершип переходит клиенту)
QCommand* Queue::Next()
{
  MutexGuard guard(mutex_);
  if ( queue_.empty() ) return 0;
  QCommand* q = queue_.front();
  queue_.pop_front();
  return q;
}

/// удаляет команду из очереди и уничтожает обьект - команду
bool Queue::Pop()
{
  MutexGuard guard(mutex_);
  if ( queue_.empty() ) return false;
  queue_.pop_front();
  return true;
}

Queue::Queue(const ProxyConfig& pconfig) 
{
  SetLimit(pconfig.quelimit);
}

static inline void Dispose(QCommand*& qc) {delete qc; qc = 0;}

Queue::~Queue()
{
  for_each(queue_.begin(),queue_.end(),Dispose);
}

SMSC_SMEPROXY_END

