#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/cvtTime.hpp"

#ifndef _REENTRANT
#define _REENTRANT	//turn on localtime_r() definition
#endif /* _REENTRANT */
#include <time.h>

namespace smsc {
namespace cvtutil {

// Packs unsigned integer [0.99] into octet in BCD form (network order).
inline uint8_t packTinyNum2BCDOct(uint8_t num)
{
  return ((uint8_t)(num % 10) << 4) | (uint8_t)(num / 10);
}
// Unpacks BCD octet (network order) to unsigned integer [0.99].
inline uint8_t unpackBCDOct2Num(uint8_t & bcd_oct)
{
  return ((bcd_oct >> 4) + (bcd_oct & 0x0F)*10);
}

// Unpacks time stored in BCD7 format (YYMMDDHHmmssZZ) to structured
// representation, holding date/time fields that is local for timezone
// identified by 'qtz'
// Returns negative error code in case of invalid  time value.
int unpack7BCD2TimeSTZ(unsigned char (*tm_bcd)[7], struct tm &tm_st, int &qtz)
{
  unsigned tzOctet = unpackBCDOct2Num((*tm_bcd)[6]);

  tm_st.tm_isdst = -1; /* let mktime() to determine wether the DST is appliable */
  tm_st.tm_wday = tm_st.tm_yday = 0;
  tm_st.tm_year = unpackBCDOct2Num((*tm_bcd[0]));
  tm_st.tm_mon  = unpackBCDOct2Num((*tm_bcd[1]));
  tm_st.tm_mday = unpackBCDOct2Num((*tm_bcd[2]));
  tm_st.tm_hour = unpackBCDOct2Num((*tm_bcd[3]));
  tm_st.tm_min  = unpackBCDOct2Num((*tm_bcd[4]));
  tm_st.tm_sec  = unpackBCDOct2Num((*tm_bcd[5]));
  qtz = !(tzOctet & 0x80) ? (int)tzOctet : (-(int)(tzOctet&0x07f));

  if (!(tm_st.tm_year >= 0 && tm_st.tm_year <= 99))
    return _SMSC_CVT_BAD_YEAR;
  tm_st.tm_year += 100; // year = x-1900
    
  if (!( tm_st.tm_mon >= 1 && tm_st.tm_mon <= 12))
    return _SMSC_CVT_BAD_YEAR;
  tm_st.tm_mon -= 1;	// adjust month to [0..11]

  if (!(tm_st.tm_mday >= 1 && tm_st.tm_mday <= 31))
    return _SMSC_CVT_BAD_DAY;
  if (!(tm_st.tm_hour >= 0 && tm_st.tm_hour <= 23))
    return _SMSC_CVT_BAD_HOUR;
  if (!(tm_st.tm_min >= 0 && tm_st.tm_min <= 59))
    return _SMSC_CVT_BAD_MIN;
  if (!(tm_st.tm_sec >=0 && tm_st.tm_sec <= 59))
    return _SMSC_CVT_BAD_SEC;
  if (!(qtz >= -47 && qtz <= 48))
    return  _SMSC_CVT_BAD_QTZ;
  return 0;
}

//Converts structured representation of time, holding date/time fields that
//is local for timezone identified by 'qtz', to UTC time seconds.
//
//NOTE: GMT = LocalTime + TZSeconds; TZSeconds = (-QTZ)*15*60
time_t cvtTimeSTZ2UTCTimeT(struct tm & tm_st, int & qtz)
{
/*  NOTE: mktime() adjust time_t for local timezone and DST */
  time_t tmVal = mktime(&tm_st);

  if (tmVal != (-1)) {
    /* consider the 'qtz' differs from local TZ, adjust time_t for difference */
    tmVal += (long)qtz*15*60 - timezone;
  }
  return tmVal;
}

// Unpacks time stored in BCD7 format (YYMMDDHHmmssZZ) to UTC time seconds.
// Returns -1 in case of invalid BCD string.
time_t unpack7BCD2UTCTimeT(unsigned char (*tm_bcd)[7])
{
  int         gtz;
  struct tm   tmSt;
  return (unpack7BCD2TimeSTZ(tm_bcd, tmSt, gtz) < 0) ? (time_t)(-1) : cvtTimeSTZ2UTCTimeT(tmSt, gtz);
}

//Packs structured representation of time to BCD7 form (YYMMDDHHmmssZZ).
void packTimeSTZ2BCD7(unsigned char (*tm_bcd)[7], struct tm &tm_st, int &qtz)
{
  uint8_t unum = (uint8_t)((tm_st.tm_year > 99) ? (tm_st.tm_year % 100) : tm_st.tm_year);

  (*tm_bcd)[0] = packTinyNum2BCDOct(unum);
  unum = tm_st.tm_mon + 1; //adjust to [1..12]
  (*tm_bcd)[1] = packTinyNum2BCDOct(unum);
  (*tm_bcd)[2] = packTinyNum2BCDOct((uint8_t)tm_st.tm_mday);
  (*tm_bcd)[3] = packTinyNum2BCDOct((uint8_t)tm_st.tm_hour);
  (*tm_bcd)[4] = packTinyNum2BCDOct((uint8_t)tm_st.tm_min);
  (*tm_bcd)[5] = packTinyNum2BCDOct((uint8_t)tm_st.tm_sec);
  unum = (qtz < 0) ? 0x80 | (uint8_t)(-qtz): qtz;
  (*tm_bcd)[6] = packTinyNum2BCDOct(unum);
}

//Packs structured representation of time to BCD8 form (YYYYMMDDHHmmssZZ).
void packTimeSTZ2BCD8(unsigned char (*tm_bcd)[8], struct tm &tm_st, int &qtz)
{ /* pack millenium & century */
  uint8_t unum = (uint8_t)((tm_st.tm_year + 1900)/100);
  (*tm_bcd)[0] = packTinyNum2BCDOct(unum);
  /* pack other parts */
  packTimeSTZ2BCD7((unsigned char (*)[7])(&((*tm_bcd)[1])), tm_st, qtz);
}

//Packs current local time to BCD7 form (YYMMDDHHmmssZZ).
//Returns false if failed to determine local time.
bool packTimeT2BCD7(unsigned char (*tm_bcd)[7], time_t tmVal)
{
  struct tm   ltms;
  if (!localtime_r(&tmVal, &ltms))
    return false;

  int qtz = (int)(-timezone/(15*60)); //'timezone' is defined in time.h
  packTimeSTZ2BCD7(tm_bcd, ltms, qtz);
  return true;
}

//Packs current local time to BCD8 form (YYYYMMDDHHmmssZZ).
//Returns false if failed to determine local time.
bool packTimeT2BCD8(unsigned char (*tm_bcd)[8], time_t tmVal)
{
  struct tm   ltms;
  if (!localtime_r(&tmVal, &ltms))
    return false;

  int qtz = (int)(-timezone/(15*60));  //'timezone' is defined in time.h
  packTimeSTZ2BCD8(tm_bcd, ltms, qtz);
  return true;
}

/* ************************************************************************** *
 * TP Validity Period conversion utility functions
 * ************************************************************************** */
#define MIN5_SECS         300
#define HALF_HOUR_SECS    1800
#define HOUR_SECS         3600
#define HALF_DAY_SECS     (12*3600)
#define DAY_SECS          (24*3600)
#define MONTH_SECS        (30*DAY_SECS)
#define WEEK_SECS         (7*DAY_SECS)
#define WEEKS5_SECS       (35*DAY_SECS)
#define WEEKS63_SECS      (63*WEEK_SECS)

uint8_t packTP_VP_Relative(time_t vp_val)
{
  uint8_t tpVp;

  if (vp_val <= HALF_DAY_SECS) {
    if (vp_val < MIN5_SECS) /* round up to lesser unit */
      vp_val = MIN5_SECS;
    tpVp = (unsigned char)(vp_val/MIN5_SECS - 1);
  } else if (vp_val <= DAY_SECS) {
    tpVp = (unsigned char)((vp_val - HALF_DAY_SECS)/HALF_HOUR_SECS + 143);
  } else if (vp_val <= MONTH_SECS) {
    if (vp_val < 2*DAY_SECS) /* round up to 2 days*/
      vp_val = 2*DAY_SECS;
    tpVp = (unsigned char)(vp_val/DAY_SECS + 166);
  } else if (vp_val <= WEEKS63_SECS) {
    if (vp_val < WEEKS5_SECS) /* round up to 5 weeks */
      vp_val = WEEKS5_SECS;
    tpVp = (unsigned char)(vp_val/WEEK_SECS + 192);
  }
  return tpVp;
}

time_t unpackTP_VP_Relative(uint8_t tp_vp)
{
  time_t vpVal;

  if (tp_vp <= 143) {          /* [5min .. 12hours], unit is 5 minutes */
    vpVal = (time_t)((unsigned long)tp_vp + 1)*MIN5_SECS;
  } else if (tp_vp <= 167) {   /* (12hours .. 24hours], unit is 30 minutes */
    vpVal = (time_t)(((unsigned long)tp_vp - 143)*HALF_HOUR_SECS + HALF_DAY_SECS);
  } else if (tp_vp <= 196) {   /* [2days .. 30days], unit is 1 day */
    vpVal = (time_t)((unsigned long)tp_vp - 166)*DAY_SECS;
  } else /* (tp_vp <= 255) */ {/* [5weeks .. 63weeks], unit is 1 week */
    vpVal = (time_t)((unsigned long)tp_vp - 192)*WEEK_SECS;
  }
  return vpVal;
}

} //cvtutil
} //smsc

