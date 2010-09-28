#ifndef _INFORMER_REGION_H
#define _INFORMER_REGION_H

#include "Typedefs.h"
#include "core/synchronization/Mutex.hpp"
#include "EmbedRefPtr.h"
#include <string>

namespace eyeline {
namespace informer {

/// representation of a region
class Region
{
    friend class EmbedRefPtr< Region >;
public:
    Region( regionid_type regionId, unsigned bw,
            const std::string& smscId ) :
    regionId_(regionId), bw_(bw), smscId_(smscId) {}

    regionid_type getRegionId() const { return regionId_; }
    unsigned      getBandwidth() const { return bw_; }
    const std::string& getSmscId() const { return smscId_; }

private:
    void ref() {
        smsc::core::synchronization::MutexGuard mg(lock_);
        ++ref_;
    }
    void unref() {
        smsc::core::synchronization::MutexGuard mg(lock_);
        if (ref_<=1) {
            delete this;
        } else {
            --ref_;
        }
    }

    Region( const Region& );
    Region& operator = ( const Region& );

private:
    regionid_type  regionId_;
    unsigned       bw_; //  sms/sec
    std::string    smscId_;
    smsc::core::synchronization::Mutex lock_;
    unsigned       ref_;
};

typedef EmbedRefPtr< Region >  RegionPtr;

} // informer
} // smsc

#endif
