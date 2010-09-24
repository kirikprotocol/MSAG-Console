#ifndef _INFORMER_REGION_H
#define _INFORMER_REGION_H

#include "Typedefs.h"

namespace eyeline {
namespace informer {

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

} // informer
} // smsc

#endif
