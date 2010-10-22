#include <ctime>
#include <sys/types.h> // for damn sunos
#include "Typedefs.h"
#include "InfosmeException.h"
#include "util/TimeSource.h"

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
    case DLVSTATE_PAUSED    : return "PAUSED";
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
