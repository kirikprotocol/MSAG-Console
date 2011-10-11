/* ************************************************************************** *
 * MT Realtime timers facility: 
 * Generic timers expiration queue interface definition.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_EXPIRATION_QUEUE_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_EXPIRATION_QUEUE_DEFS_HPP

#include "core/timers/TimerFSM.hpp"

namespace smsc {
namespace core {
namespace timers {

class ExpirationQueueIface {
public:
  // -------------------------------------------------------
  // -- ExpirationQueueIface interface methods:
  // -------------------------------------------------------
  virtual bool empty(void) const = 0;
  //
  virtual TimerUId size(void) const = 0;
  //Returns false if in case of unsupported kind of timer expiration time.
  virtual bool insert(TimerLink & sw_link) = 0;
  //NOTE: It's a caller responsibility to ensure that given TimerLink was
  //      inserted previously !!!
  virtual void unlink(TimerLink & sw_link) = 0;

  //Exports nearest expiration time of all entries
  //Returns false if queue is empty.
  virtual bool nextTime(struct timespec & exp_tm) const = 0;
  //Exports the last expiration time of all entries
  //Returns false if queue is empty.
  virtual bool lastTime(struct timespec & exp_tm) const = 0;

  //Pops (unlinks) one of entry with expiration time less than specified.
  //Returns true if succeeded.
  virtual bool popExpired(const struct timespec & exp_tm, TimerRef & sw_ref) = 0;
  //Pops (unlinks) one of entry with expiration time less than current one.
  //Returns true if succeeded.
  virtual bool popExpired(TimerRef & sw_ref) = 0;

protected:
  ExpirationQueueIface()
  { }
  virtual ~ExpirationQueueIface() //forbid interface destruction
  { }
};

} //timers
} //core
} //smsc
#endif /* SMSC_CORE_TIMERS_EXPIRATION_QUEUE_DEFS_HPP */

