#ifndef _SMSC_INFOSME2_MESSAGE_H
#define _SMSC_INFOSME2_MESSAGE_H

#include <sys/types.h>
#include <string>
// #include <ctime>
#include "util/int.h"

namespace smsc {
namespace infosme2 {

struct Message
{
    static int stringToRegionId( const std::string& regionId ); // throw ConfigException
    static std::string regionIdToString( int regId );

    uint64_t    id;
    time_t      date;
    int         regionId;
    std::string abonent;
    std::string message;
    std::string userData;

    Message(): id(0), date(0) {}

};

}
}

#endif /* _SMSC_INFOSME2_MESSAGE_H */

