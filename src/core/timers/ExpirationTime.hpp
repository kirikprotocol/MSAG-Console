/* ************************************************************************** *
 * MT Realtime timers facility: 
 * Generic Timer expiration time specificaiton.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_EXPIRATION_TIME_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_EXPIRATION_TIME_HPP

#include "util/MaxSizeof.hpp"

#include "core/timers/TimeOps.hpp"
#include "core/synchronization/TimeSlice.hpp"

namespace smsc {
namespace core {
namespace timers {

using smsc::core::synchronization::TimeSlice;

class ExpirationTime {
public:
  enum ValueType_e {
    valTimeSpec = 0  //absolute timeSpec form
  , valTmoVal        //relative timeout form
  };

private:
  union {
    void *        mAligner;
    unsigned char mBuf[smsc::util::MaxSizeOf2_T<TimeSlice, struct  timespec>::VALUE];
  } mMem;

protected:
  ValueType_e  mKind;

  TimeSlice * pTmo(void) { return (TimeSlice *)(mMem.mBuf); }
  const TimeSlice * pTmo(void) const { return (const TimeSlice *)(mMem.mBuf); }

  struct timespec * pTms(void) { return (struct timespec *)(mMem.mBuf); }
  const struct timespec * pTms(void) const { return (const struct timespec *)(mMem.mBuf); }

public:
  explicit ExpirationTime(long use_tmo = 0, TimeSlice::UnitType_e use_unit = TimeSlice::tuSecs)
    : mKind(valTmoVal)
  {
    new (pTmo()) TimeSlice(use_tmo, use_unit);
  }
  explicit ExpirationTime(const TimeSlice & use_tmo) : mKind(valTmoVal)
  {
    new (pTmo()) TimeSlice(use_tmo);
  }
  explicit ExpirationTime(const struct timespec & abs_time) : mKind(valTimeSpec)
  {
    *pTms() = abs_time;
  }
  ~ExpirationTime()
  { }

  ExpirationTime & operator=(const struct timespec & abs_time)
  {
    mKind = valTimeSpec; *pTms() = abs_time;
    return *this;
  }

  ExpirationTime & operator=(const TimeSlice & use_tmo)
  {
    mKind = valTmoVal; *pTmo() = use_tmo;
    return *this;
  }

  ValueType_e getKind(void) const { return mKind; }

  bool isValueTMO(void) const { return ((mKind == valTmoVal) && pTmo()->Value()); }
  bool isValueTMS(void) const { return mKind == valTimeSpec; }

  //Note: Returns valid value only in case of valTmoVal
  const TimeSlice & getValueTMO(void) const { return *pTmo(); }
  //Note: Returns valid value only in case of valTimeSpec
  const struct timespec & getValueTMS(void) const { return *pTms(); }

  //Adjusts own value to 'Absolute TimeSpec' form
  struct timespec adjust2TMS(const struct timespec * cur_time = 0) const
  {
    return isValueTMS() ? getValueTMS() : getValueTMO().adjust2Nano(cur_time);
  }
  //Adjusts own value to 'Absolute TimeSpec' form and assigns it to itself.
  void convert2TMS(const struct timespec * cur_time = 0)
  {
    *this = adjust2TMS(cur_time);
  }

  bool operator== (const ExpirationTime & cp_obj2) const
  {
    if (mKind != cp_obj2.mKind)
      return false;
    return isValueTMO() ? (*pTmo() == *cp_obj2.pTmo()) : (*pTms() == *cp_obj2.pTms());
  }

  bool operator< (const ExpirationTime & cp_obj2) const
  {
    if (mKind != cp_obj2.mKind)
      return mKind < cp_obj2.mKind;
    return isValueTMO() ? (*pTmo() < *cp_obj2.pTmo()) : (*pTms() < *cp_obj2.pTms());
  }
};

} //timers
} //core
} //smsc
#endif /* SMSC_CORE_TIMERS_EXPIRATION_TIME_HPP */

