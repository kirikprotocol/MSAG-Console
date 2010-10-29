#include <ctime>
#include <sys/types.h> // for damn sunos
#include <cstdlib>
#include <cassert>
#include "Typedefs.h"
#include "InfosmeException.h"
#include "util/TimeSource.h"
#include "core/synchronization/Mutex.hpp"

namespace {
using namespace eyeline::informer;

int locoffset = -100000;
smsc::core::synchronization::Mutex offsetLock_;
int localOffset()
{
    if (locoffset == -100000 ) {
        smsc::core::synchronization::MutexGuard mg(offsetLock_);
        if (locoffset == -100000) {
            // calculation of local offset for use with gmtime/mktime pair.
            const time_t curTime(currentTimeMicro()/tuPerSec);
            ::tm now;
            if (!gmtime_r(&curTime,&now)) {
                std::abort();
            }
            now.tm_isdst = 0;
            const int res(int(mktime(&now)));
            locoffset = int(curTime) - res;
            assert(locoffset % 3600 == 0);
        }
    }
    return locoffset;
}

}


namespace eyeline {
namespace informer {

ulonglong msgTimeToYmd( msgtime_type tmp, std::tm* tmb )
{
    std::tm tx;
    if (!tmb) tmb = &tx;
    if (!tmp) {
        // FIXME: memset(tmb,0,sizeof(*tmb));
        return 0;
    }
    const time_t t(tmp);
    if ( !gmtime_r(&t,tmb) ) {
        throw InfosmeException("formatMsgTime: cannot gmtime_r");
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
        throw InfosmeException("invalid sec %d in time buf %llu",tmb->tm_sec,tmp);
    }
    tmp1 /= 100;
    tmb->tm_min = int(tmp1 % 100);
    if (tmb->tm_min < 0 || tmb->tm_min > 59) {
        throw InfosmeException("invalid min %d in time buf %llu",tmb->tm_min,tmp);
    }
    tmp1 /= 100;
    tmb->tm_hour = int(tmp1 % 100);
    if (tmb->tm_hour < 0 || tmb->tm_hour > 23) {
        throw InfosmeException("invalid hour %d in time buf %llu",tmb->tm_hour,tmp);
    }
    tmp1 /= 100;
    tmb->tm_mday = int(tmp1 % 100);
    if (tmb->tm_mday < 1 || tmb->tm_mday > 31) {
        throw InfosmeException("invalid mday %d in time buf %llu",tmb->tm_mday,tmp);
    }
    tmp1 /= 100;
    tmb->tm_mon = int(tmp1 % 100) - 1;
    if (tmb->tm_mon < 0 || tmb->tm_mon > 11) {
        throw InfosmeException("invalid mon %d in time buf %llu",tmb->tm_mon,tmp);
    }
    tmp1 /= 100;
    tmb->tm_year = int(tmp1) - 1900;
    if (tmb->tm_year < 100 || tmb->tm_year > 200) {
        throw InfosmeException("invalid year %d in time buf %llu",tmb->tm_year,tmp);
    }
    tmb->tm_isdst = 0;
    return msgtime_type(mktime(tmb)+localOffset());
}


usectime_type currentTimeMicro()
{
    static smsc::util::TimeSourceSetup::AbsUSec usecSource;
    return usectime_type(usecSource.getUSec());
}


const char* msgStateToString( MsgState state )
{
    switch (state) {
    case MSGSTATE_INPUT     : return "inpt";
    case MSGSTATE_PROCESS   : return "proc";
    // case MSGSTATE_TAKEN     : return "takn";
    case MSGSTATE_SENT      : return "sent";
    case MSGSTATE_DELIVERED : return "dlvd";
    case MSGSTATE_EXPIRED   : return "xprd";
    case MSGSTATE_FAILED    : return "fail";
    case MSGSTATE_RETRY     : return "rtry";
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


char* makeDeliveryPath( dlvid_type dlvId, char* buf )
{
    const unsigned chunk = unsigned(dlvId/100)*100;
    const int rv = sprintf(buf,"deliveries/%010u/%u/",chunk,dlvId);
    if (rv<0) {
        throw InfosmeException("cannot form delivery path D=%u, chunk=%u",dlvId,chunk);
    }
    return buf + rv;
}


bool isGoodAsciiName( const char* str, char* badchar )
{
    assert(str);
    for ( const char* p = str; *p != '\0'; ++p ) {
        register const char c = *p;
        if ( c >= '-' && c <= '9' && c != '/' ) {
            // ok
        } else if ( c >= '@' && c <= 'Z' ) {
            // ok
        } else if ( c == '_' ) {
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
