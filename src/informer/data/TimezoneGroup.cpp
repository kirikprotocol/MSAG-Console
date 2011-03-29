#include <string.h>
#include "TimezoneGroup.h"

namespace eyeline {
namespace informer {

TimezoneGroup::TimezoneGroup( int tzid ) : tzid_(tzid) {}


int TimezoneGroup::fixDst( struct tm& now ) const
{
    int result = 0; // in hours
    switch ( tzid_ ) {
    case TZ_RUSSIA :
        // medvedev cancelled the dst
        result = 3600;
        break;
    default:
        result = 0;
        break;
    }
    return result;
}


/*
int TimezoneGroup::lookupId( const char* name )
{
    if ( !name && !name[0] ) return TZ_RUSSIA;
#define TZRUSSIA(val) if (!strcmp(name,val)) return TZ_RUSSIA
    TZRUSSIA("Europe/Kaliningrad");
    TZRUSSIA("Europe/Moscow");
    TZRUSSIA("Europe/Samara");
    TZRUSSIA("Europe/Volgograd");
    TZRUSSIA("Asia/Anadyr");
    TZRUSSIA("Asia/Irkutsk");
    TZRUSSIA("Asia/Kamchatka");
    TZRUSSIA("Asia/Krasnoyarsk");
    TZRUSSIA("Asia/Magadan");
    TZRUSSIA("Asia/Novokuznetsk");
    TZRUSSIA("Asia/Novosibirsk");
    TZRUSSIA("Asia/Omsk");
    TZRUSSIA("Asia/Sakhalin");
    TZRUSSIA("Asia/Vladivostok");
    TZRUSSIA("Asia/Yakutsk");
    TZRUSSIA("Asia/Yekaterinburg");
    return TZ_UNKNOWN;
}
 */

}
}
