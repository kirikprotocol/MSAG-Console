#ifndef _INFORMER_REGION_H
#define _INFORMER_REGION_H

#include <string>
#include <vector>
#include "core/synchronization/Mutex.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

class TimezoneGroup;

/// representation of a region
class Region
{
    friend class EmbedRefPtr< Region >;
public:
    Region( regionid_type        regionId,
            const char*          name,
            const char*          smscId,
            unsigned             bw,
            int                  timezone,
            const TimezoneGroup* tzgroup,
            bool                 deleted,    // may be set to true, disabling the region
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
    void setSmscId( const char* smscId );

    void swap( Region& r );

    bool hasEqualMasks( const Region& r ) const;

    /// return local week time since the start of the week (seconds)
    int getLocalWeekTime( msgtime_type now ) const;

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
    const TimezoneGroup*     tzgroup_;
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
