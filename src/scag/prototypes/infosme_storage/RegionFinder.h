#ifndef _SCAG_PROTOTYPES_INFOSME_REGIONFINDER_H
#define _SCAG_PROTOTYPES_INFOSME_REGIONFINDER_H

#include <ctime>

namespace scag2 {
namespace prototypes {
namespace infosme {

class RegionFinder
{
public:
    /// return the validity time of the region finder.
    virtual time_t validityTime() const = 0;

    /// return the region id for the subscriber.
    virtual int findRegion( const char* subscriber ) = 0;
};

}
}
}

#endif /* ! _SCAG_PROTOTYPES_INFOSME_REGIONFINDER_H */
