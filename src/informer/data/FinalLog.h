#ifndef _INFORMER_FINALLOG_H
#define _INFORMER_FINALLOG_H

#include "informer/io/FileGuard.h"
#include "informer/data/DeliveryInfo.h"

namespace eyeline {
namespace informer {

class Message;

/// this class also holds statistics data.
class FinalLog
{
    static FinalLog* instance_;
public:
    /// pseudo singleton.
    inline static FinalLog* getFinalLog() { return instance_; }

    FinalLog();
    ~FinalLog();

    /// NOTE: msg.retryCount contains Nsms
    void addMsgRecord( msgtime_type         currentTime,
                       dlvid_type           dlvId,
                       const char*          userId,
                       const Message&       msg,
                       int                  smppStatus );

    void addDlvRecord( msgtime_type         currentTime,
                       dlvid_type           dlvId,
                       const char*          userId,
                       DlvState             state );

    void checkRollFile( msgtime_type currentTime );

private:
    void doCheckRollFile( msgtime_type currentTime,
                          bool         create );
    void rollFile( const char* fn );

private:
    smsc::core::synchronization::Mutex lock_;
    FileGuard                          fg_;
    msgtime_type                       createTime_;
    msgtime_type                       period_;
    char                               filename_[100];
};

} // informer
} // smsc

#endif
