/*
  $Id$
*/
#include "smedispatch.h"
namespace smsc {
namespace smeman {

using core::synchronization::MutexGuard;
#define __synchronized__ MutexGuard mguard(dispatch_lock);

// !!!!
// not synchronized because only one thread processed on this method
// !!!!
SmeProxy* SmeProxyDispatcher::dispatchIn(unsigned long /*timeout*/,int* idx)
{
  for(;;)
  {
    {__synchronized__
      Unit* unit = unqueuedProxies;
      while ( unit )
      {
        Unit* __next = unit->next;
        __require__ ( unit->proxy != NULL );
        if ( unit->proxy->hasInput() )
        {
          unit->prior = unit->proxy->getPriority();
        }
        if ( unit->prev ) unit->prev->next = unit->next; // remove from list
        else
        {
          __require__ ( unit == unqueuedProxies );
          unqueuedProxies = unit->next; // remove first elemment
          if ( unqueuedProxies ) unqueuedProxies->prev = 0;
        }
        unit->next = queuedProxies;
        unit->prev = 0;
        if ( queuedProxies ) queuedProxies->prev = unit;
        queuedProxies = unit;
        unit = __next;
      }
      if ( queuedProxies )
      {
        Unit* prior_unit = 0;
        SmeProxyPriority prior = SmeProxyPriorityMin;
        unit = queuedProxies;
        while ( unit ) {
          SmeProxyPriority tmp;
          if ( ( tmp = unit->prior) > prior )
          {
            prior = tmp;
            prior_unit = unit;
          }
          unit = unit->next;
        }

        if ( prior_unit->prev ) 
        {
          prior_unit->prev->next = prior_unit->next;
          if ( prior_unit->next ) prior_unit->next->prev = prior_unit->prev;
        }
        else
        {
          __require__(prior_unit == queuedProxies );
          queuedProxies = prior_unit->next;
          if ( queuedProxies ) queuedProxies->prev = 0;
        }
        prior_unit->next = unqueuedProxies;
        if ( unqueuedProxies ) unqueuedProxies->prev = prior_unit;
        prior_unit->prev = 0;
        unqueuedProxies = prior_unit;
        __require__(prior_unit != NULL);
        __require__(prior_unit->proxy != NULL);
        if ( idx ) *idx = prior_unit->idx;
				return prior_unit->proxy;
      }
    }// __synchronization__
    mon.Wait();
  }
}

void SmeProxyDispatcher::attachSmeProxy(SmeProxy* proxy,int idx)
{
__synchronized__
  __require__ ( proxy!= NULL );
  if ( proxy->attached() ) throw SmeError();
  Unit* unit = new Unit;
  unit->prev = 0;
  unit->next = unqueuedProxies;
  if ( unqueuedProxies ) unqueuedProxies->prev = unit;
  unqueuedProxies = unit;
  unit->proxy = proxy;
	unit->idx = idx;
  proxy->attachMonitor(&mon);
}

void SmeProxyDispatcher::detachSmeProxy(SmeProxy* proxy)
{
__synchronized__
  __require__ ( proxy != NULL );
  __require__ ( proxy->attached() );
  proxy->attachMonitor(0);
  Unit* unit = queuedProxies;
  while ( unit )
  {
    if ( unit->proxy = proxy )
    {
      if ( unit == queuedProxies ) 
      {
        queuedProxies = unit->next;
        if ( queuedProxies ) queuedProxies->prev = 0;
      }
      else
      {
        unit->prev->next = unit->next;
        if ( unit->next ) unit->next->prev = unit->prev;
      }
      delete unit;
      return;
    }
    unit = unit->next;
  }
  unit = unqueuedProxies;
  while ( unit )
  {
    if ( unit->proxy = proxy )
    {
      if ( unit == unqueuedProxies ) 
      {
        unqueuedProxies = unit->next;
        if ( unqueuedProxies ) unqueuedProxies->prev = 0;
      }
      else
      {
        unit->prev->next = unit->next;
        if ( unit->next ) unit->next->prev = unit->prev;
      }
      delete unit;
      return;
    }
    unit = unit->next;
  }
  throw SmeError();
}

}; // namespace smeman
}; // namespace smsc
