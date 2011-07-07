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

class Region;

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
    /// nchunks is a number of SMS chunks
    void addRecord( msgtime_type         currentTime,
                    const Region&        region,
                    const Message&       msg,
                    int                  smppStatus,
                    int                  nchunks,
                    uint8_t              fromState );

    /// add records about deletion of messages
    void addDeleteRecords( msgtime_type                   currentTime,
                           const std::vector<msgid_type>& msgIds );

    // flush buffers to the disk.
    // NOTE: typically is only invoked at delivery state change
    // to prevent performance degradation.
    void fsync();

    /// FIXME: @param isOldVersion may be removed after all dlv converted
    static bool readStatistics( const std::string& filename,
                                TmpBufBase<char>& buf,
                                DeliveryStats& ods,
                                bool& isOldVersion );

    static bool readFirstRecordSeconds( const std::string& filename,
                                        TmpBufBase<char>& buf,
                                        unsigned& seconds );

    /// FIXME: may be removed after all dlv converted
    msgtime_type fixActLogFormat( msgtime_type currentTime );

private:
    void createFile( msgtime_type currentTime, struct tm& now );
    void makeActLogPath( char* buf, const struct tm& now ) const;

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
