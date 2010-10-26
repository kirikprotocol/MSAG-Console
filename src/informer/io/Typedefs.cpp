#include <ctime>
#include <sys/types.h> // for damn sunos
#include <cstdlib>
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
            struct tm now;
            if (!gmtime_r(&curTime,&now)) {
                abort();
            }
            now.tm_isdst = 0;
            const msgtime_type res(mktime(&now));
            locoffset = curTime - res;
        }
    }
    return locoffset;
}

}


namespace eyeline {
namespace informer {

int formatMsgTime( char* buf, msgtime_type tmp, struct tm* tmb )
{
    struct tm tx;
    if (!tmb) tmb = &tx;
    const time_t t(tmp);
    if ( !gmtime_r(&t,tmb) ) {
        throw InfosmeException("formatMsgTime: cannot gmtime_r");
    }
    const int off = sprintf(buf,"%04u%02u%02u%02u%02u%02u",
                            tmb->tm_year+1900, tmb->tm_mon+1, tmb->tm_mday,
                            tmb->tm_hour, tmb->tm_min, tmb->tm_sec );
    if (off<0) {
        throw InfosmeException("formatMsgTime: cannot sprintf");
    }
    return off;
}


msgtime_type scanfMsgTime( ulonglong tmbuf )
{
    if (tmbuf==0) return 0;
    struct tm tmst;
    ulonglong tmp1 = tmbuf;
    ulonglong tmp2 = tmp1 / 100;
    tmst.tm_sec = int(tmp1 - tmp2*100);
    if (tmst.tm_sec < 0 || tmst.tm_sec > 59) {
        throw InfosmeException("invalid sec in time buf %llu",tmbuf);
    }
    tmp1 /= 10000;
    tmst.tm_min = int(tmp2 - tmp1*100);
    if (tmst.tm_min < 0 || tmst.tm_min > 59) {
        throw InfosmeException("invalid min in time buf %llu",tmbuf);
    }
    tmp2 /= 10000;
    tmst.tm_hour = int(tmp1 - tmp2*100);
    if (tmst.tm_hour < 0 || tmst.tm_hour > 23) {
        throw InfosmeException("invalid hour in time buf %llu",tmbuf);
    }
    tmp1 /= 10000;
    tmst.tm_mday = int(tmp2 - tmp1*100);
    if (tmst.tm_mday < 1 || tmst.tm_mday > 31) {
        throw InfosmeException("invalid mday in time buf %llu",tmbuf);
    }
    tmp2 /= 10000;
    tmst.tm_mon = int(tmp1 - tmp2*100) - 1;
    if (tmst.tm_mon < 0 || tmst.tm_mon > 11) {
        throw InfosmeException("invalid mon in time buf %llu",tmbuf);
    }
    tmp1 /= 10000;
    tmst.tm_year = int(tmp2 - tmp1*100) - 1900;
    if (tmst.tm_year < 100 || tmst.tm_year > 200) {
        throw InfosmeException("invalid year in time buf %llu",tmbuf);
    }
    tmst.tm_isdst = 0;
    return msgtime_type(mktime(&tmst) + localOffset());
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
    case MSGSTATE_TAKEN     : return "takn";
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

}
}
