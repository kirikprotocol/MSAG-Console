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
lock_( MTXWHEREAMI ),
regionId_(regionId), name_(name), smscId_(smscId), bw_(bw),
timezone_(timezone), tzgroup_(tzgroup), deleted_(deleted),
reflock_( MTXWHEREAMI ),
ref_(0)
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


void Region::getSmscId( char* smscId ) const
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    strcpy(smscId,smscId_.c_str());
}


void Region::getName( char* name ) const
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    strcpy(name,name_.c_str());
}


void Region::setSmscId( const char* smscId )
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    smscId_ = smscId;
}


void Region::swap( Region& r )
{
    smsc::core::synchronization::MutexGuard mg(lock_);
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
    smsc::core::synchronization::MutexGuard mg(lock_);
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
    const int inday = int(now % daynight);
    int result = int( (tmnow.tm_wday+6)*daynight +
                      inday + dst ) % aweek;
    /*
    smsc_log_debug(log_,"R=%u weekTime: tz=%+d tm_wday=%d inday=%u:%u dst=%d result=%u+%u:%u",
                   regionId_, getTimezone(), tmnow.tm_wday, inday / 3600,
                   inday / 60 % 60, dst,
                   result / daynight, result % daynight / 3600,
                   result % daynight / 60 % 60 );
     */
    return result;
}

}
}
