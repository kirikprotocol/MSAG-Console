#ifndef _INFORMER_ACTIVITYLOG_H
#define _INFORMER_ACTIVITYLOG_H

#include "informer/data/Message.h"
#include "informer/io/FileGuard.h"
#include "DeliveryStats.h"
#include "core/buffers/TmpBuf.hpp"

namespace eyeline {
namespace informer {

class DeliveryInfo;

/// this class also holds statistics data.
class ActivityLog
{
public:
    ActivityLog( dlvid_type dlvId );

    dlvid_type getDlvId() const;

    /// this will automatically increment stats
    void addRecord( msgtime_type         currentTime,
                    regionid_type        regId,
                    const Message&       msg,
                    int                  smppStatus,
                    uint8_t              fromState = 0 );

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

private:
    smsc::core::synchronization::Mutex lock_;
    dlvid_type                         dlvId_;
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
