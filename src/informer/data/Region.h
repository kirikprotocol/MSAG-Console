#ifndef _INFORMER_REGION_H
#define _INFORMER_REGION_H

#include <string>
#include <vector>
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/FixedLengthString.hpp"
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
    int getTimezone() const { return timezone_; }
    bool isDeleted() const { return deleted_; }

    /// NOTE: this method is not under lock!
    const std::vector<std::string>& getMasks() const { return masks_; }

    void setDeleted( bool del ) { deleted_ = del; }

    // synchronized access
    // NOTE: the buf must be at least of SMSC_ID_LENGTH
    void getSmscId( char* buf ) const;
    // NOTE: the buf must be at least of REGION_NAME_LENGTH
    void getName( char* buf ) const;
    void setSmscId( const char* smscId );
    void swap( Region& r );
    bool hasEqualMasks( const Region& r ) const;

    /// return local week time since the start of the week (seconds)
    int getLocalWeekTime( msgtime_type now ) const;

private:
    inline void ref() {
        smsc::core::synchronization::MutexGuard mg(reflock_);
        ++ref_;
    }
    inline void unref() {
        {
            smsc::core::synchronization::MutexGuard mg(reflock_);
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
    smsc::core::synchronization::Mutex reflock_;
    unsigned                           ref_;

    mutable smsc::core::synchronization::Mutex lock_;
    regionid_type            regionId_;
    smsc::core::buffers::FixedLengthString<REGION_NAME_LENGTH> name_;
    smsc::core::buffers::FixedLengthString<SMSC_ID_LENGTH>     smscId_;
    unsigned                 bw_;          //  sms/sec
    int                      timezone_;
    const TimezoneGroup*     tzgroup_;
    bool                     deleted_;
    std::vector<std::string> masks_;       //  sorted masks
};

typedef EmbedRefPtr< Region >  RegionPtr;

class RegionFinder
{
public:
    virtual ~RegionFinder() {}
    virtual void findRegion( personid_type subscriber,
                             RegionPtr& ptr ) = 0;
    virtual bool getRegion( regionid_type regionId,
                            RegionPtr& ptr ) = 0;
};

} // informer
} // smsc

#endif
