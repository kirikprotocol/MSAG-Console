#ifndef _INFORMER_REGION_H
#define _INFORMER_REGION_H

#include <string>
#include <vector>
#include "core/synchronization/Mutex.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

/// representation of a region
class Region
{
    friend class EmbedRefPtr< Region >;
public:
    Region( regionid_type regionId,
            const char*   name,
            const char*   smscId,
            unsigned      bw,
            int           timezone,
            std::vector<std::string>* masks = 0 );

    ~Region();

    regionid_type getRegionId() const { return regionId_; }
    unsigned      getBandwidth() const { return bw_; }
    const std::string& getSmscId() const { return smscId_; }
    const std::string& getName() const { return name_; }
    const int getTimezone() const { return timezone_; }
    const std::vector<std::string>& getMasks() const { return masks_; }

    void swap( Region& r ) {
        std::swap(regionId_,r.regionId_);
        name_.swap(r.name_);
        smscId_.swap(r.smscId_);
        std::swap(bw_,r.bw_);
        std::swap(timezone_,r.timezone_);
        masks_.swap(r.masks_);
    }

private:
    void ref() {
        smsc::core::synchronization::MutexGuard mg(lock_);
        ++ref_;
    }
    void unref() {
        {
            smsc::core::synchronization::MutexGuard mg(lock_);
            if (ref_>1) {
                --ref_;
                return;
            }
        }
        delete this;
    }

    Region( const Region& );
    Region& operator = ( const Region& );

private:
    regionid_type            regionId_;
    std::string              name_;
    std::string              smscId_;
    unsigned                 bw_;          //  sms/sec
    int                      timezone_;
    std::vector<std::string> masks_;
    smsc::core::synchronization::Mutex lock_;
    unsigned                           ref_;
};

typedef EmbedRefPtr< Region >  RegionPtr;

class RegionFinder
{
public:
    virtual ~RegionFinder() {}
    virtual regionid_type findRegion( personid_type subscriber ) = 0;
};

} // informer
} // smsc

#endif
