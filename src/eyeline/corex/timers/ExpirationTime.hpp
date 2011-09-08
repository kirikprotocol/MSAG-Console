/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef _CORE_TIMERS_EXPIRATION_TIME_HPP
#ident "@(#)$Id$"
#define _CORE_TIMERS_EXPIRATION_TIME_HPP

#include <inttypes.h>
#include "core/synchronization/TimeSlice.hpp"
#include "eyeline/corex/timers/TimeOps.hpp"

namespace eyeline {
namespace corex {
namespace timers {

using smsc::core::synchronization::TimeSlice;

//StopWatch expiration time
class ExpirationTime {
public:
  enum ValueType_e {
      valTime = 0   //absolute timeSpec form
    , valTmo        //relative timeout form
  };

protected:
  ValueType_e  _kind;
  struct {
    TimeSlice           _tmo;
    struct  timespec    _tms;
  }           _val;

public:
  ExpirationTime(long use_tmo = 0, TimeSlice::UnitType_e use_unit = TimeSlice::tuSecs)
    : _kind(valTmo)
  {
    _val._tmo = TimeSlice(use_tmo, use_unit);
  }
  ExpirationTime(const TimeSlice & use_tmo) : _kind(valTmo)
  {
    _val._tmo = use_tmo;
  }
  ExpirationTime(const struct timespec & abs_time) : _kind(valTime)
  {
    _val._tms = abs_time;
  }
  ~ExpirationTime()
  { }

  ExpirationTime & operator=(const struct timespec & abs_time)
  {
    _kind = valTime; _val._tms = abs_time;
    return *this;
  }

  ExpirationTime & operator=(const TimeSlice & use_tmo)
  {
    _kind = valTmo; _val._tmo = use_tmo;
    return *this;
  }

  ValueType_e getKind(void) const { return _kind; }
  //Note: Returns valid value only in case of valTmo
  const TimeSlice & getValueTMO(void) const { return _val._tmo; }
  //Note: Returns valid value only in case of valTime
  const struct timespec & getValueTMS(void) const { return _val._tms; }

  //Adjusts own value to 'Absolute TimeSpec' form
  struct timespec adjust2TMS(const struct timespec * cur_time = 0) const
  {
    if (_kind == valTime)
      return getValueTMS();
    return getValueTMO().adjust2Nano(cur_time);
  }
  //Adjusts own value to 'Absolute TimeSpec' form and assigns it to itself.
  void convert2TMS(const struct timespec * cur_time = 0)
  {
    *this = adjust2TMS(cur_time);
  }

  bool operator== (const ExpirationTime & obj2) const
  {
    if (_kind != obj2._kind)
      return false;
    return (_kind == valTmo) ? (_val._tmo == obj2._val._tmo)
                              : (_val._tms == obj2._val._tms);
  }

  bool operator< (const ExpirationTime & obj2) const
  {
    if (_kind != obj2._kind)
      return _kind < obj2._kind;
    return (_kind == valTmo) ? (_val._tmo < obj2._val._tmo)
                              : (_val._tms < obj2._val._tms);
  }
};


class ExpirationQueueIface {
protected:
  virtual ~ExpirationQueueIface() //forbid interface destruction
  { }

public:
  virtual bool empty(void) const = 0;
  //
  virtual uint32_t size(void) const = 0;
  //
  virtual bool insert(uint32_t sw_id, const ExpirationTime & tgt_time) = 0;
  //
  virtual void erase(uint32_t sw_id, const ExpirationTime & tgt_time) = 0;

  //Exports nearest expiration time of all entries
  //Returns false if queue is empty.
  virtual bool nextTime(struct timespec & exp_tm) const = 0;
  //Exports the last expiration time of all entries
  //Returns false if queue is empty.
  virtual bool lastTime(struct timespec & exp_tm) const = 0;

  //Pops one of entry with expiration time less than specified.
  virtual bool popExpired(const struct timespec & exp_tm, uint32_t & sw_id) = 0;
  //Pops one of entry with expiration time less than current one.
  virtual bool popExpired(uint32_t & sw_id) = 0;
};


} //timers
} //corex
} //eyeline
#endif /* _CORE_TIMERS_EXPIRATION_TIME_HPP */

