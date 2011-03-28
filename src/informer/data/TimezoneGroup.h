#ifndef _INFORMER_TIMEZONEGROUP_H
#define _INFORMER_TIMEZONEGROUP_H

#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

/// representation of a region
class TimezoneGroup
{
public:
    enum {
        TZ_UNKNOWN = 0,
        TZ_RUSSIA = 1
    };

    TimezoneGroup( int tzid );

    inline int getId() const { return tzid_; }

    // return offset in seconds
    int fixDst( struct tm& tmnow ) const;

    // convert name into id
    // static int lookupId( const char* name );

protected:
    TimezoneGroup( const TimezoneGroup& );
    TimezoneGroup& operator = ( const TimezoneGroup& );

private:
    int tzid_;
};

} // informer
} // smsc

#endif
