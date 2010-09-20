#ifndef _INFOSME_V3_REGION_H
#define _INFOSME_V3_REGION_H

#include "Typedefs.h"

namespace smsc {
namespace infosme {

class Region
{
public:
    Region( regionid_type regionId, unsigned bw ) :
    regionId_(regionId), bw_(bw) {}

    regionid_type getRegionId() const { return regionId_; }
    unsigned      getBandwidth() const { return bw_; }

private:
    regionid_type  regionId_;
    unsigned       bw_; //  sms/sec
};

} // infosme
} // smsc

#endif
