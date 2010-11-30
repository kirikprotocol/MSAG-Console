#ifndef _INFORMER_USERINFO_H
#define _INFORMER_USERINFO_H

#include <string.h>
#include <vector>
#include "core/buffers/FixedLengthString.hpp"
#include "core/synchronization/Mutex.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "informer/io/Typedefs.h"
#include "informer/io/InfosmeException.h"
#include "Delivery.h"
#include "SpeedControl.h"

namespace eyeline {
namespace informer {

typedef smsc::core::buffers::FixedLengthString<64> userid_type;

typedef enum
{
    USERROLE_ADMIN = 0,
    USERROLE_USER = 1
} UserRole;


// number of deliveries by user
struct UserDlvStats
{
    uint32_t paused;
    uint32_t planned;
    uint32_t active;
    uint32_t finished;
    uint32_t cancelled;

    void clear() {
        ::memset(this,0,sizeof(*this));
    }

    bool isEmpty() const {
        return paused == 0 && planned == 0 && active == 0 && finished == 0 && cancelled == 0;
    }

    // NOTE: we do not count finished deliveries
    uint32_t getTotal() const {
        return paused + planned + active;
    }

    void incStat( uint8_t state, int value ) {
        switch (state) {
        case DLVSTATE_PAUSED : paused += value; break;
        case DLVSTATE_PLANNED : planned += value; break;
        case DLVSTATE_ACTIVE: active += value; break;
        case DLVSTATE_FINISHED: finished += value; break;
        case DLVSTATE_CANCELLED: cancelled += value; break;
        default:
            throw InfosmeException(EXC_LOGICERROR,"unknown delivery state %d",state);
        }
    }
};

class UserInfo
{
    friend class EmbedRefPtr< UserInfo >;
    friend class DeliveryImpl;

public:
    static const size_t USERID_LENGTH = userid_type::MAX_SZ;
    static const size_t PASSWORD_LENGTH = 32;

    typedef std::vector< DeliveryPtr > DeliveryList;

    UserInfo( const char* id,
              const char* pwd,
              unsigned    priority = 1,
              unsigned    speed = 1,
              unsigned    totaldlv = 10 );
              
    ~UserInfo();

    // NOTE: userId never changes!
    inline const char* getUserId() const {
        return userId_.c_str();
    }

    inline char* getPassword( char* pwd ) const {
        MutexGuard mg(dataLock_);
        strcpy(pwd,password_.c_str());
        return pwd;
    }

    bool hasRole( UserRole role ) const;

    /// max number of deliveries or (-1==unlimited).
    unsigned getMaxTotalDeliveries() const { return maxTotalDeliveries_; }

    /// return the number of microsecond to sleep.
    /// if ready return 0 and consume a quant of time.
    usectime_type isReadyAndConsumeQuant( usectime_type currentTime );

    // a limit of sms per second
    unsigned getSpeed() const { return unsigned(speedControl_.getSpeed()); }
    /// priority -- the more the better
    unsigned getPriority() const { return priority_; }

    bool isDeleted() const { return isDeleted_; }

    void addRole( UserRole role );

    // roles are replaced by those of ruser.
    void update( const UserInfo& ruser );

    // mark the user as deleted
    void setDeleted( bool del ) {
        MutexGuard mg(dataLock_);
        isDeleted_ = del;
    }

    /// get current stats
    inline void getStats( UserDlvStats& stats ) {
        MutexGuard mg(statLock_);
        stats = stats_;
    }

    /// get the list of deliveries, ordered by dlvid
    void getDeliveries( DeliveryList& dlvs ) const;

    /// increment number of deliveries
    void incDlvStats( uint8_t state,
                      uint8_t fromState = 0,
                      bool checkDlvLimit = false );

    void popIncrementalStats( UserDlvStats& ds );

    /// NOTE: service methods, do not invoke!
    void attachDelivery( const DeliveryPtr& dlv );
    void detachDelivery( dlvid_type dlvId );

private:
    void ref();
    void unref();

private:

    // NOTE: for faster reaction speed control is under reflock
    mutable smsc::core::synchronization::Mutex refLock_;
    unsigned    ref_;
    SpeedControl<usectime_type,tuPerSec> speedControl_;

    mutable smsc::core::synchronization::Mutex dataLock_;
    userid_type userId_;
    userid_type password_;
    uint64_t    roles_;
    unsigned    maxTotalDeliveries_;
    unsigned    priority_;
    bool        isDeleted_;
    // the list of owned deliveries, ordered by dlvid
    DeliveryList deliveries_;

    // statistics
    smsc::core::synchronization::Mutex statLock_;
    UserDlvStats                       stats_;
    UserDlvStats                       incstats_[2];

};

typedef EmbedRefPtr<UserInfo> UserInfoPtr;

} // informer
} // smsc

#endif
