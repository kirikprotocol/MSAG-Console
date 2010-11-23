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
            bool          deleted,    // may be set to true, disabling the region
            std::vector<std::string>* masks = 0 );

    ~Region();

    regionid_type getRegionId() const { return regionId_; }
    unsigned      getBandwidth() const { return bw_; }
    const std::string& getSmscId() const { return smscId_; }
    const std::string& getName() const { return name_; }
    int getTimezone() const { return timezone_; }
    bool isDeleted() const { return deleted_; }
    const std::vector<std::string>& getMasks() const { return masks_; }

    void setDeleted( bool del ) { deleted_ = del; }

    void swap( Region& r ) {
        std::swap(regionId_,r.regionId_);
        name_.swap(r.name_);
        smscId_.swap(r.smscId_);
        std::swap(bw_,r.bw_);
        std::swap(deleted_, r.deleted_ );
        std::swap(timezone_,r.timezone_);
        masks_.swap(r.masks_);
    }

    bool hasEqualMasks( const Region& r ) const;

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
    bool                     deleted_;
    std::vector<std::string> masks_;       //  sorted masks
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
