#include <ctime>
#include <sys/types.h> // for damn sunos
#include <cstdlib>
#include <cassert>
#include "Typedefs.h"
#include "InfosmeException.h"
#include "util/TimeSource.h"
#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

namespace {
int locoffset = -100000;
smsc::core::synchronization::Mutex offsetLock_;
}

int getLocalTimezone()
{
    if (locoffset == -100000 ) {
        smsc::core::synchronization::MutexGuard mg(offsetLock_);
        if (locoffset == -100000) {
            // calculation of local offset for use with gmtime/mktime pair.
            const time_t curTime(currentTimeMicro()/tuPerSec);
            ::tm now;
            if (!gmtime_r(&curTime,&now)) {
                throw InfosmeException(EXC_SYSTEM,"localOffset: gmtime_r(%u) failed",unsigned(curTime));
            }
            now.tm_isdst = 0;
            const int res(int(mktime(&now)));
            locoffset = int(curTime) - res;
            assert(locoffset % 3600 == 0);
            smsc_log_info(smsc::logger::Logger::getInstance("core"),
                          "local timezone = %d",locoffset);
        }
    }
    return locoffset;
}


ulonglong msgTimeToYmd( msgtime_type tmp, std::tm* tmb )
{
    std::tm tx;
    if (!tmb) tmb = &tx;
    if (!tmp) {
        return 0;
    }
    const time_t t(tmp);
    if ( !gmtime_r(&t,tmb) ) {
        throw InfosmeException(EXC_SYSTEM,"formatMsgTime: gmtime_r(%u) failed",tmp);
    }
    return ( ( ( ( ulonglong(tmb->tm_year+1900)*100 +
                   tmb->tm_mon + 1 ) * 100 +
                 tmb->tm_mday ) * 100 +
               tmb->tm_hour ) * 100 +
             tmb->tm_min ) * 100 + tmb->tm_sec;
}


msgtime_type ymdToMsgTime( ulonglong tmp, std::tm* tmb )
{
    std::tm tx;
    if (!tmb) tmb = &tx;
    if (!tmp) {
        return 0;
    }
    ulonglong tmp1 = tmp;
    tmb->tm_sec = int(tmp1 % 100);
    if (tmb->tm_sec < 0 || tmb->tm_sec > 59) {
        throw InfosmeException(EXC_SYSTEM,"invalid sec %d in time buf %llu",tmb->tm_sec,tmp);
    }
    tmp1 /= 100;
    tmb->tm_min = int(tmp1 % 100);
    if (tmb->tm_min < 0 || tmb->tm_min > 59) {
        throw InfosmeException(EXC_SYSTEM,"invalid min %d in time buf %llu",tmb->tm_min,tmp);
    }
    tmp1 /= 100;
    tmb->tm_hour = int(tmp1 % 100);
    if (tmb->tm_hour < 0 || tmb->tm_hour > 23) {
        throw InfosmeException(EXC_SYSTEM,"invalid hour %d in time buf %llu",tmb->tm_hour,tmp);
    }
    tmp1 /= 100;
    tmb->tm_mday = int(tmp1 % 100);
    if (tmb->tm_mday < 1 || tmb->tm_mday > 31) {
        throw InfosmeException(EXC_SYSTEM,"invalid mday %d in time buf %llu",tmb->tm_mday,tmp);
    }
    tmp1 /= 100;
    tmb->tm_mon = int(tmp1 % 100) - 1;
    if (tmb->tm_mon < 0 || tmb->tm_mon > 11) {
        throw InfosmeException(EXC_SYSTEM,"invalid mon %d in time buf %llu",tmb->tm_mon,tmp);
    }
    tmp1 /= 100;
    tmb->tm_year = int(tmp1) - 1900;
    if (tmb->tm_year < 100 || tmb->tm_year > 200) {
        throw InfosmeException(EXC_SYSTEM,"invalid year %d in time buf %llu",tmb->tm_year,tmp);
    }
    tmb->tm_isdst = 0;
    return msgtime_type(mktime(tmb) + getLocalTimezone());
}


usectime_type currentTimeMicro()
{
    static smsc::util::TimeSourceSetup::AbsUSec usecSource;
    return usectime_type(usecSource.getUSec());
}


msgtime_type parseDateTime( const char* datetime )
{
    int shift = 0;
    unsigned day, month, year, hour, minute, second;
    sscanf(datetime,"%02u.%02u.%04u %02u:%02u:%02u%n",&day,&month,&year,&hour,&minute,&second,&shift);
    if (!shift) {
        throw InfosmeException(EXC_IOERROR,"invalid datetime '%s'",datetime);
    }
    return ymdToMsgTime((((ulonglong(year*100+month)*100+day)*100+hour)*100+minute)*100+second);
}


msgtime_type parseDate( const char* date )
{
    int shift = 0;
    unsigned day, month, year;
    sscanf(date,"%02u.%02u.%04u%n",&day,&month,&year,&shift);
    if (!shift) {
        throw InfosmeException(EXC_IOERROR,"invalid date '%s'",date);
    }
    return ymdToMsgTime((ulonglong(year*100+month)*100+day)*1000000);
}


timediff_type parseTime( const char* theTime )
{
    int shift = 0;
    unsigned hour, minute, second;
    sscanf(theTime,"%02u:%02u:%02u%n",&hour,&minute,&second,&shift);
    if (!shift) {
        throw InfosmeException(EXC_IOERROR,"invalid time '%s'",theTime);
    }
    if (hour > 23) {
        throw InfosmeException(EXC_IOERROR,"invalid hour %u in '%s'",hour,theTime);
    }
    if (minute > 59) {
        throw InfosmeException(EXC_IOERROR,"invalid minute %u in '%s'",minute,theTime);
    }
    if (second > 60) { // leap second
        throw InfosmeException(EXC_IOERROR,"invalid second %u in '%s'",second,theTime);
    }
    return timediff_type((hour*60+minute)*60+second);
}


/// parse address in one of three form 'NNNNN', '+NNNNN', '.T.P.NNNNN'
personid_type parseAddress( const char* isdn )
{
    unsigned ton, npi, len;
    ulonglong value;
    int shift = 0;
    if ( !isdn ) {
        throw InfosmeException(EXC_LOGICERROR,"address NULL passed");
    }
    if (isdn[0] == '+') {
        ton = 1;
        npi = 1;
        sscanf(isdn,"+%llu%n",&value,&shift);
        len = unsigned(shift - 1);
    } else if (isdn[0] == '.') {
        int start = 0;
        sscanf(isdn,".%u.%u.%n%llu%n",&ton,&npi,&start,&value,&shift);
        len = unsigned(shift-start);
    } else {
        sscanf(isdn,"%llu%n",&value,&shift);
        ton = 0;
        npi = 1;
        len = unsigned(shift);
    }
    if (!shift) {
        throw InfosmeException(EXC_BADADDRESS,"invalid address '%s'",isdn);
    }
    if (unsigned(shift) != strlen(isdn)) {
        throw InfosmeException(EXC_BADADDRESS,"address '%s' has extra chars",isdn);
    }
    if (len<1 || len>16) {
        throw InfosmeException(EXC_BADADDRESS,"invalid length=%u in '%s'",len,isdn);
    }
    if (ton>15) {
        throw InfosmeException(EXC_BADADDRESS,"invalid ton=%u in '%s'",ton,isdn);
    }
    if (npi>15) {
        throw InfosmeException(EXC_BADADDRESS,"invalid npi=%u in '%s'",npi,isdn);
    }
    return addressToSubscriber(uint8_t(len),uint8_t(ton),uint8_t(npi),uint64_t(value));
}


const char* msgStateToString( MsgState state )
{
    switch (state) {
    case MSGSTATE_INPUT     : return "NEW";
    case MSGSTATE_PROCESS   : return "PROC";
    case MSGSTATE_SENT      : return "SENT";
    case MSGSTATE_RETRY     : return "RTRY";
    case MSGSTATE_DELIVERED : return "DLVD";
    case MSGSTATE_EXPIRED   : return "EXPD";
    case MSGSTATE_FAILED    : return "FAIL";
    case MSGSTATE_KILLED    : return "KILL";
    default                 : return "????";
    }
}


const char* dlvStateToString( DlvState state )
{
    switch (state) {
    case DLVSTATE_PAUSED    : return "STOPPED";
    case DLVSTATE_PLANNED   : return "PLANNED";
    case DLVSTATE_ACTIVE    : return "ACTIVE";
    case DLVSTATE_FINISHED  : return "FINISHED";
    case DLVSTATE_CANCELLED : return "CANCELLED";
    default                 : return "??????";
    };
}


char* makeDeliveryPath( char* buf, dlvid_type dlvId )
{
    const unsigned chunk = unsigned(dlvId/100)*100;
    const int rv = sprintf(buf,"deliveries/%010u/%u/",chunk,dlvId);
    if (rv<0) {
        throw InfosmeException(EXC_SYSTEM,"cannot form delivery path D=%u, chunk=%u",dlvId,chunk);
    }
    return buf + rv;
}


bool isGoodAsciiName( const char* str, char* badchar )
{
    if (!str || !str[0]) {
        if (badchar) { *badchar = '0'; }
        return false;
    }
    assert(str);
    for ( const char* p = str; *p != '\0'; ++p ) {
        register const char c = *p;
        if ( c >= '-' && c <= ';' && c != '/' ) {
            // ok
        } else if ( c >= '@' && c <= 'Z' ) {
            // ok
        } else if ( c == '_' || c == '#' || c == '^' || c == '=' ) {
            // ok
        } else if ( c >= 'a' && c <= 'z' ) {
            // ok
        } else {
            if (badchar) *badchar = c;
            return false;
        }
    }
    return true;
}

}
}
