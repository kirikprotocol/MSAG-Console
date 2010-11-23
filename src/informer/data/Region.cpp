#include <algorithm>
#include "Region.h"

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
                bool deleted,
                std::vector< std::string >* masks ) :
regionId_(regionId), name_(name), smscId_(smscId), bw_(bw),
timezone_(timezone), deleted_(deleted), ref_(0)
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


bool Region::hasEqualMasks( const Region& r ) const
{
    return ( r.masks_ == masks_ );
}

}
}
