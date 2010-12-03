#ifndef _INFORMER_ACTIVITYLOG_H
#define _INFORMER_ACTIVITYLOG_H

#include <vector>
#include <memory>
#include "informer/data/Message.h"
#include "informer/io/FileGuard.h"
#include "informer/io/EmbedRefPtr.h"
#include "DeliveryStats.h"
#include "core/buffers/TmpBuf.hpp"
#include "DeliveryInfo.h"

namespace eyeline {
namespace informer {

class UserInfo;

/// this class also holds statistics data.
class ActivityLog
{
public:
    /// dlvinfo gets owned
    ActivityLog( UserInfo& userInfo, DeliveryInfo* dlvInfo );

    inline DeliveryInfo& getDlvInfo() { return * dlvInfo_.get(); }
    inline const DeliveryInfo& getDlvInfo() const { return *dlvInfo_.get(); }
    inline dlvid_type getDlvId() const { return dlvInfo_->getDlvId(); }

    inline UserInfo& getUserInfo() { return *userInfo_; }
    inline const UserInfo& getUserInfo() const { return *userInfo_; }

    /// this will automatically increment stats
    void addRecord( msgtime_type         currentTime,
                    regionid_type        regId,
                    const Message&       msg,
                    int                  smppStatus,
                    uint8_t              fromState = 0 );

    /// add records about deletion of messages
    void addDeleteRecords( msgtime_type                   currentTime,
                           const std::vector<msgid_type>& msgIds );


    /// increment stats, optionally decrementing fromState
    void incStats( uint8_t state,
                   int     value = 1,
                   uint8_t fromState = 0,
                   int     smsValue = 0 );

    void getStats( DeliveryStats& ds )
    {
        MutexGuard mg(statLock_);
        ds = stats_;
    }

    /// the method pops released incremental stats and then clear it.
    void popIncrementalStats( DeliveryStats& ds );

private:
    void doIncStats( uint8_t state, int value, uint8_t fromState, int smsValue );

    bool readStatistics( const std::string& filename,
                         smsc::core::buffers::TmpBuf<char, 8192 >& buf );

    void createFile( msgtime_type currentTime, struct tm& now );

private:
    smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    EmbedRefPtr< UserInfo >            userInfo_;
    std::auto_ptr< DeliveryInfo >      dlvInfo_;
    FileGuard                          fg_;
    msgtime_type                       createTime_;
    msgtime_type                       period_;

    smsc::core::synchronization::Mutex statLock_;
    DeliveryStats                      stats_;
    DeliveryStats                      incstats_[2];
};

} // informer
} // smsc

#endif
