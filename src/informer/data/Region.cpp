#include <algorithm>
#include "Region.h"
#include "TimezoneGroup.h"

namespace {
smsc::logger::Logger* log_ = 0;
void getlog() {
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("region");
    }
}
}

namespace eyeline {
namespace informer {

Region::Region( regionid_type regionId,
                const char* name,
                const char* smscId,
                unsigned bw,
                int timezone,
                const TimezoneGroup* tzgroup,
                bool deleted,
                std::vector< std::string >* masks ) :
regionId_(regionId), name_(name), smscId_(smscId), bw_(bw),
timezone_(timezone), tzgroup_(tzgroup), deleted_(deleted), ref_(0)
{
    getlog();
    if (masks) { masks->swap(masks_); }
    std::sort(masks_.begin(), masks_.end());
    smsc_log_debug(log_,"R=%u @%p ctor: %sbw=%u S='%s' name='%s' tz=%d masks=%u",
                   regionId_, this, deleted ? "DELETED " : "",
                   bw_, smscId_.c_str(), name_.c_str(), timezone_,
                   unsigned(masks_.size()));
}


Region::~Region()
{
    smsc_log_debug(log_,"R=%u @%p dtor",regionId_,this);
}


void Region::setSmscId( const char* smscId )
{
    MutexGuard mg(lock_);
    smscId_ = smscId;
}


void Region::swap( Region& r )
{
    MutexGuard mg(lock_);
    std::swap(regionId_,r.regionId_);
    name_.swap(r.name_);
    smscId_.swap(r.smscId_);
    std::swap(bw_,r.bw_);
    std::swap(deleted_, r.deleted_ );
    std::swap(timezone_,r.timezone_);
    masks_.swap(r.masks_);
}


bool Region::hasEqualMasks( const Region& r ) const
{
    return ( r.masks_ == masks_ );
}


int Region::getLocalWeekTime( msgtime_type now ) const
{
    // move into local time
    now += getTimezone();
    struct tm tmnow;
    {
        const time_t tmp = time_t(now);
        gmtime_r(&tmp,&tmnow);
    }
    const int dst = tzgroup_->fixDst(tmnow);
    static const int daynight = 24*3600;
    static const int aweek = 7*daynight;
    int result = int( (tmnow.tm_wday+6)*daynight +
                      now % daynight +
                      dst ) % aweek;
    return result;
}

}
}
