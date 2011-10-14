/* ************************************************************************** *
 * Time conversion utility functions widely used in 3GPP.
 * ************************************************************************** */
#ifndef __SMSC_3GPP_TIME_CONVERSION_UTIL_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_3GPP_TIME_CONVERSION_UTIL_HPP

#include <inttypes.h>
#include <time.h>

namespace smsc {
namespace cvtutil {

#define _SMSC_CVT_BAD_TIME  (-1)
#define _SMSC_CVT_BAD_YEAR  (-2)
#define _SMSC_CVT_BAD_MONTH (-3)
#define _SMSC_CVT_BAD_DAY   (-4)
#define _SMSC_CVT_BAD_HOUR  (-5)
#define _SMSC_CVT_BAD_MIN   (-6)
#define _SMSC_CVT_BAD_SEC   (-7)
#define _SMSC_CVT_BAD_QTZ   (-8)

//Converts structured representation of time, holding date/time fields that
//is local for timezone identified by 'qtz', to UTC time seconds.
extern time_t cvtTimeSTZ2UTCTimeT(struct tm &tm_st, int &qtz);

//Packs structured representation of time to BCD7 form (YYMMDDHHmmssZZ).
extern void packTimeSTZ2BCD7(unsigned char (*tm_bcd)[7], struct tm &tm_st, int &qtz);
//Packs structured representation of time to BCD8 form (YYYYMMDDHHmmssZZ).
extern void packTimeSTZ2BCD8(unsigned char (*tm_bcd)[8], struct tm &tm_st, int &qtz);
// Unpacks time stored in BCD7 format (YYMMDDHHmmssZZ) to structured
// representation, holding date/time fields that is local for timezone
// identified by 'qtz'
// Returns negative error code in case of invalid  time value.
extern int  unpack7BCD2TimeSTZ(unsigned char (*tm_bcd)[7], struct tm &tm_st, int &qtz);
// Unpacks time stored in BCD7 format (YYMMDDHHmmssZZ) to UTC time seconds.
// Returns -1 in case of invalid BCD string.
extern time_t unpack7BCD2UTCTimeT(unsigned char (*tm_bcd)[7]);

//Packs current local time to BCD7 form (YYMMDDHHmmssZZ).
//Returns false if failed to determine local time.
extern bool  packTimeT2BCD7(unsigned char (*tm_bcd)[7], time_t tmVal);
//Packs current local time to BCD8 form (YYYYMMDDHHmmssZZ).
//Returns false if failed to determine local time.
extern bool  packTimeT2BCD8(unsigned char (*tm_bcd)[8], time_t tmVal);

extern uint8_t packTP_VP_Relative(time_t vp_val);
extern time_t unpackTP_VP_Relative(uint8_t tp_vp);

}//cvtutil
}//smsc
#endif /* __SMSC_3GPP_TIME_CONVERSION_UTIL_HPP */

