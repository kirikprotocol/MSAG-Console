#ifndef _INFORMER_DELIVERYINFO_H
#define _INFORMER_DELIVERYINFO_H

#include "informer/io/Typedefs.h"
#include "logger/Logger.h"
// #include "system/status.h"
// #include "DeliveryStats.h"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class CommonSettings;
class UserInfo;
class InfosmeCore;

typedef enum {
    DLVMODE_SMS = 0,
    DLVMODE_USSDPUSH = 1,
    DLVMODE_USSDPUSHVLR = 2
} DeliveryMode;


class DeliveryInfo
{
public:
    DeliveryInfo( const CommonSettings& cs,
                  dlvid_type            dlvId,
                  UserInfo*             userInfo = 0 ) :
    cs_(cs), dlvId_(dlvId),
    from_(0x4010000000000000ULL + 10000), // FIXME: .0.1.10000
    isReplaceIfPresent_(true), isFlash_(false), useDataSm_(false),
    transactionMode_(0),
    deliveryMode_(DLVMODE_SMS),
    state_(DlvState(0)), // DLVSTATE_PAUSED),
    planTime_(0),
    userInfo_(userInfo)
    {
        if (!log_) log_ = smsc::logger::Logger::getInstance("dlvinfo");
    }

    const CommonSettings& getCS() const { return cs_; }

    // this method is invoked from regional storage.
    // void incrementStats( const DeliveryStats& stats, DeliveryStats* result = 0 );
    // this method is invoked to update stats from activity.log
    // void updateStats( const DeliveryStats& stats );

    // get stats
    // void getStats( DeliveryStats& stats ) const;

    dlvid_type getDlvId() const { return dlvId_; }

    unsigned getPriority() const { return 1; }

    DlvState getState( msgtime_type* planTime = 0 ) const {
        if (planTime) *planTime = planTime_;
        return state_;
    }

    const UserInfo* getUserInfo() const { return userInfo_; }

    void setState( DlvState state, msgtime_type planTime );

    personid_type getFrom() const { return from_; } 

    /// message validity time, seconds
    unsigned getMessageValidityTime() const { return 3600; }

    bool isReplaceIfPresent() const { return isReplaceIfPresent_; }

    const std::string& getSvcType() const { return svcType_; }

    int getTransactionMode() const { return transactionMode_; }

    bool isFlash() const { return isFlash_; }

    bool useDataSm() const { return useDataSm_; }

    DeliveryMode getDeliveryMode() const { return deliveryMode_; }

    /// read delivery info
    void read( InfosmeCore& core );

    /// evaluate number of chunks
    unsigned evaluateNchunks( const char* out, size_t outLen ) const;

private:
    static smsc::logger::Logger* log_;

private:
    const CommonSettings& cs_;
    dlvid_type      dlvId_;
    personid_type   from_;
    bool            isReplaceIfPresent_, isFlash_, useDataSm_;
    std::string     svcType_;
    int             transactionMode_;
    DeliveryMode    deliveryMode_;
    std::string     retryPolicyName_;
    DlvState        state_;
    msgtime_type    planTime_;
    UserInfo*       userInfo_;

    mutable smsc::core::synchronization::Mutex lock_;
};

} // informer
} // smsc

#endif
