#include <ctime>
#include <sys/types.h> // for damn sunos
#include "Typedefs.h"
#include "InfosmeException.h"
#include "util/TimeSource.h"

namespace eyeline {
namespace informer {

char* formatMsgTime( char* buf, msgtime_type tmp )
{
    struct tm tx = {0};
    const time_t t(tmp);
    if ( !gmtime_r(&t,&tx) ) {
        throw InfosmeException("formatMsgTime: cannot gmtime_r");
    }
    sprintf(buf,"%02u-%02u-%02u+%02u:%02u:%02u",
            tx.tm_year%100, tx.tm_mon+1, tx.tm_mday, tx.tm_hour, tx.tm_min, tx.tm_sec );
    return buf;
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
    case MSGSTATE_SENT      : return "sent";
    case MSGSTATE_DELIVERED : return "dlvr";
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
