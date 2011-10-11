/* ************************************************************************** *
 * MT Realtime timers facility: basic definitions.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_TIMER_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_TIMER_DEFS_HPP

#include <inttypes.h>

namespace smsc {
namespace core {
namespace timers {

typedef uint32_t TimerUId;

//Timer expiration signal handler interface
class TimerListenerIface {
public:
  enum EventResult_e {//result of timer event processing
    evtOk = 0         //event is succesfully handled
  , evtResignal = 1   //event should be resignaled again
  };

  //Argument 'tmr_stat' is a small string containing timer usage statistics,
  //and is passed for informative purpose only.
  //
  //NOTE: TimerListener is allowed to block for a long time while this call.
  //It also may return evtResignal if unable to immediately handle event.
  //In that case timer event will be signalled again a little bit later.
  virtual EventResult_e onTimerEvent(TimerUId tmr_id, const char * tmr_stat) = 0;

protected:
  TimerListenerIface()
  { }
  virtual ~TimerListenerIface() //forbid interface destruction
  { }
};
//
typedef TimerListenerIface::EventResult_e  TimerEventResult_e;


} //timers
} //core
} //smsc
#endif /* SMSC_CORE_TIMERS_TIMER_DEFS_HPP */

