#include <ctime>
#include <sys/types.h> // for damn sunos
#include "Typedefs.h"
#include "InfosmeException.h"
#include "util/TimeSource.h"

namespace smsc {
namespace infosme {

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


namespace MsgState {
const char* toString( uint8_t state )
{
    switch (state) {
    case input : return "inpt";
    case process : return "proc";
    case sent : return "sent";
    case delivered : return "dlvr";
    case expired : return "xprd";
    case failed : return "fail";
    case retry : return "rtry";
    default : return "????";
    }
}
}

}
}
