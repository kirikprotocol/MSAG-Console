#ifndef _INFORMER_FINALLOG_H
#define _INFORMER_FINALLOG_H

#include <vector>
#include "informer/io/FileGuard.h"
#include "informer/data/DeliveryInfo.h"

namespace eyeline {
namespace informer {

struct Message;

/// this class also holds statistics data.
class FinalLog
{
    struct LogFileFilter;
    static FinalLog* instance_;
public:
    /// pseudo singleton.
    inline static FinalLog* getFinalLog() { return instance_; }

    explicit FinalLog( const std::vector< std::string >& finalpaths );
    ~FinalLog();

    void addMsgRecord( msgtime_type         currentTime,
                       dlvid_type           dlvId,
                       const char*          userId,
                       const Message&       msg,
                       int                  smppStatus,
                       int                  nsms );

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
    std::vector< std::string >         finalpaths_;
    FileGuard                          fg_;
    msgtime_type                       createTime_;
    msgtime_type                       period_;
    char                               filename_[50]; // YYYYMMDDHHMMSS.log
};

} // informer
} // smsc

#endif
