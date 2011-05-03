#ifndef _INFORMER_ACTIVITYLOG_H
#define _INFORMER_ACTIVITYLOG_H

#include <vector>
#include <memory>
#include "informer/data/Message.h"
#include "informer/io/FileGuard.h"
#include "informer/io/EmbedRefPtr.h"
#include "DeliveryInfo.h"
#include "informer/io/TmpBuf.h"

namespace eyeline {
namespace informer {

/// this class also holds statistics data.
class ActivityLog
{
public:
    /// dlvinfo gets owned
    ActivityLog( DeliveryInfo* dlvInfo );

    inline DeliveryInfo& getDlvInfo() { return *dlvInfo_; }
    inline const DeliveryInfo& getDlvInfo() const { return *dlvInfo_; }
    inline dlvid_type getDlvId() const { return dlvInfo_->getDlvId(); }

    /// this will automatically increment stats
    void addRecord( msgtime_type         currentTime,
                    regionid_type        regId,
                    const Message&       msg,
                    int                  smppStatus,
                    uint8_t              fromState = 0 );

    /// add records about deletion of messages
    void addDeleteRecords( msgtime_type                   currentTime,
                           const std::vector<msgid_type>& msgIds );

    // flush buffers to the disk.
    // NOTE: typically is only invoked at delivery state change
    // to prevent performance degradation.
    void fsync();

    static bool readStatistics( const std::string& filename,
                                TmpBufBase<char>& buf,
                                DeliveryStats& ods );

private:
    void createFile( msgtime_type currentTime, struct tm& now );

private:
    smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    DeliveryInfo*                      dlvInfo_; // not owned
    FileGuard                          fg_;
    msgtime_type                       createTime_;
    msgtime_type                       period_;
};

} // informer
} // smsc

#endif
