#include <cstdio>
#include <cstdlib>
#include "Message.h"
#include "util/Exception.hpp"

namespace smsc {
namespace infosme2 {

using namespace smsc::util;

int Message::stringToRegionId( const std::string& regionId )
{
    int regId = -1;
    if ( regionId == "regDefault" ) {
        // ok
    } else if ( regionId.empty() ) {
        throw Exception("regionId is empty");
    } else {
        const char* regIdStr = regionId.c_str();
        char* endptr;
        regId = int(std::strtol(regIdStr,&endptr,10));
        if ( regId < -1 ) {
            throw Exception("regionId value is wrong: %s",regIdStr);
        } else if ( regId == -1 ) {
            throw Exception("regionId equal to reserved value -1");
        } else if ( *endptr != '\0' ) {
            throw Exception("regionId not fully converted from '%s'",regIdStr);
        }
    }
    return regId;
}


std::string Message::regionIdToString( int regionId )
{
    char buf[30];
    if ( regionId >= 0 ) {
        sprintf(buf,"%d",regionId);
    } else if ( regionId == -1 ) {
        sprintf(buf,"regDefault");
    } else {
        throw Exception("region %d is wrong",regionId);
    }
    return buf;
}


}
}
