#ifndef _INFORMER_FINALLOG_H
#define _INFORMER_FINALLOG_H

#include "informer/io/FileGuard.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryInfo.h"

namespace eyeline {
namespace informer {

class Message;

/// this class also holds statistics data.
class FinalLog
{
public:
    FinalLog( const CommonSettings& cs );

    void addMsgRecord( msgtime_type         currentTime,
                       dlvid_type           dlvId,
                       const char*          userId,
                       const Message&       msg,
                       int                  smppStatus );

    void addDlvRecord( msgtime_type         currentTime,
                       dlvid_type           dlvId,
                       const char*          userId,
                       DlvState             state );

private:
    void checkRollFile( msgtime_type        currentTime );
    void rollFile( const char* fn );

private:
    smsc::core::synchronization::Mutex lock_;
    const CommonSettings&              cs_;
    FileGuard                          fg_;
    msgtime_type                       createTime_;
    msgtime_type                       period_;
    char                               filename_[100];
};

} // informer
} // smsc

#endif
