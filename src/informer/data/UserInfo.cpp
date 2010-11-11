#include <cassert>
#include "logger/Logger.h"
#include "UserInfo.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/Typedefs.h"
#include "CommonSettings.h"

namespace {
smsc::logger::Logger* log_ = 0;
void getlog() {
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("userinfo");
    }
}

using namespace eyeline::informer;

const uint64_t userroles[] = {
    1ULL << int(USERROLE_ADMIN),
    1ULL << int(USERROLE_USER)
};

struct OrderByDlvId
{
    bool operator () ( const DeliveryPtr& ptr, dlvid_type dlvId ) const {
        return (ptr->getDlvId() < dlvId);
    }
};

}

namespace eyeline {
namespace informer {

UserInfo::UserInfo( const char* id,
                    const char* pwd,
                    unsigned    priority,
                    unsigned    speed,
                    unsigned    totaldlv ) :
ref_(0),
roles_(0),
maxTotalDeliveries_(totaldlv),
speed_(speed),
priority_(priority)
{
    getlog();
    assert(id && pwd);
    if ( strlen(id) >= userid_type::npos ) {
        throw InfosmeException(EXC_BADNAME,"U='%s' too long name, must be less than %u",
                               id, unsigned(userid_type::npos) );
    }
    // check symbols
    {
        char c;
        if ( ! isGoodAsciiName(id,&c) ) {
            throw InfosmeException(EXC_BADNAME,"U='%s' has forbidden char='%c'", id, c);
        }
    }
    userId_ = id;
    password_ = pwd;
    stats_.clear();
    incstats_[0].clear();
    incstats_[1].clear();
    static const unsigned maxprio = 100;
    if (priority_ > maxprio) {
        smsc_log_warn(log_,"U='%s' too big priority %u replaced with %u",id,priority_,maxprio);
        priority_ = maxprio;
    }
    static const unsigned maxspeed = 1000;
    if (speed_ > maxspeed) {
        smsc_log_warn(log_,"U='%s' too big speed %u replaced with %u",id,speed_,maxspeed);
        speed_ = maxspeed;
    }
    static const unsigned maxdlv = 1000000;
    if (maxTotalDeliveries_ > maxdlv) {
        smsc_log_warn(log_,"U='%s' too many maxdlvs %u replaced with %u",id,maxTotalDeliveries_,maxdlv);
        maxTotalDeliveries_ = maxdlv;
    }
    smsc_log_debug(log_,"U='%s' ctor done",id);
}


UserInfo::~UserInfo()
{
    smsc_log_debug(log_,"U='%s' dtor",userId_.c_str());
}


bool UserInfo::hasRole( UserRole role ) const
{
    if (unsigned(role) >= sizeof(userroles)/sizeof(userroles[0]) ) {
        throw InfosmeException(EXC_NOTFOUND,"U='%s' wrong role %u",userId_.c_str(),unsigned(role));
    }
    return (roles_ & userroles[unsigned(role)]) != 0;
}


void UserInfo::addRole( UserRole role )
{
    if (unsigned(role) >= sizeof(userroles)/sizeof(userroles[0]) ) {
        throw InfosmeException(EXC_NOTFOUND,"U='%s' wrong role %u",userId_.c_str(),unsigned(role));
    }
    roles_ |= userroles[unsigned(role)];
}


void UserInfo::update( const UserInfo& user )
{
    MutexGuard mg(lock_);
    password_ = user.password_;
    roles_ = user.roles_;
    maxTotalDeliveries_ = user.maxTotalDeliveries_;
    speed_ = user.speed_;
    priority_ = user.priority_;
}



void UserInfo::getDeliveries( DeliveryList& dlvs )
{
    MutexGuard mg(lock_);
    dlvs = deliveries_;
}


void UserInfo::incStats( const CommonSettings& cs,
                         uint8_t state,
                         uint8_t fromState )
{
    if (state == fromState ) return;
    MutexGuard mg(statLock_);
    if (fromState) stats_.incStat(fromState,-1);
    stats_.incStat(state,1);
    unsigned total = stats_.getTotal();
    if ( total > maxTotalDeliveries_ ) {
        stats_.incStat(state,-1);
        if (fromState) stats_.incStat(fromState,1);
        throw InfosmeException(EXC_DLVLIMITEXCEED,
                               "U='%s' add delivery state='%s' failed: count=%u limit=%u",
                               userId_.c_str(), 
                               dlvStateToString(DlvState(state)),
                               total,
                               maxTotalDeliveries_ );
    }
    const unsigned idx = cs.getStatBankIndex();
    incstats_[idx].incStat(state,1);
}


void UserInfo::popIncrementalStats( const CommonSettings& cs, UserDlvStats& ds )
{
    MutexGuard mg(statLock_);
    const unsigned idx = 1 - cs.getStatBankIndex();
    ds = incstats_[idx];
    incstats_[idx].clear();
}


void UserInfo::attachDelivery( const DeliveryPtr& dlv )
{
    assert(&dlv->getUserInfo() == this);
    MutexGuard mg(lock_);
    DeliveryList::iterator i = 
        std::lower_bound( deliveries_.begin(),
                          deliveries_.end(),
                          dlv->getDlvId(),
                          ::OrderByDlvId() );
    if ( i != deliveries_.end() ) {
        if ( i->get() == dlv.get() ) return; // already attached
        if ( (*i)->getDlvId() == dlv->getDlvId() ) {
            throw InfosmeException(EXC_ALREADYEXIST,"U='%s' already has delivery D=%u",
                                   userId_.c_str(),dlv->getDlvId());
        }
    }
    deliveries_.insert(i,dlv);
}


void UserInfo::detachDelivery( dlvid_type dlvId )
{
    MutexGuard mg(lock_);
    DeliveryList::iterator i =
        std::lower_bound( deliveries_.begin(),
                          deliveries_.end(),
                          dlvId,
                          ::OrderByDlvId() );
    if (i != deliveries_.end()) {
        deliveries_.erase(i);
    }
}


void UserInfo::ref()
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    smsc_log_debug(log_,"U='%s' ref=%u +1",userId_.c_str(),ref_);
    ++ref_;
}


void UserInfo::unref()
{
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        smsc_log_debug(log_,"U='%s' ref=%u -1",userId_.c_str(),ref_);
        if (ref_>1) {
            --ref_;
            return;
        }
    }
    delete this;
}


}
}
