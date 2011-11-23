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
class FileBuffer;

/// this class also holds statistics data.
class ActivityLog
{
    static smsc::logger::Logger* slog_;
    static void initLog();
public:
    /*
    class Recovery {
    public:
        Recovery( const std::string& fn, bool zipped );

        void testAndRecover();

    private:
        std::string fname_;
        bool        zipped_; // if reading log is zipped
        FileGuard   from_;
        FileGuard   to_;     // in case there is a failure
    };
     */

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
    static bool readStatistics( FileBuffer& fb,
                                DeliveryStats* ods,
                                bool& isOldVersion );

    /// open and scan zipfile to the end
    static void scanZipToEnd( FileBuffer& fb );

    static bool readFirstRecordSeconds( const std::string& filename,
                                        TmpBufBase<char>& buf,
                                        unsigned& seconds );

    /// FIXME: may be removed after all dlv converted
    msgtime_type fixActLogFormat( msgtime_type currentTime );

    /// try to join all logs for a given delivery.
    /// @param hourly - if set then join hourly, otherwise daily
    static void joinLogs( const std::string& dlvdir,
                          bool hourly = true,
                          bool destroyDir = false );

    static void fixLog( const char* fn, bool zipped );

    /// @return zipversion, position fg on the first chunk record
    static unsigned readZipHead( FileBuffer& fb );

    /// @param ymdtime if present will contain the 
    /// @return the chunk length or 0
    static size_t readChunkHead( FileBuffer& fb,
                                 unsigned zipVersion,
                                 ulonglong* ymdtime = 0 );

    /// @return version or throw
    static unsigned readChunkVersion( FileBuffer& fb );
    static bool readStatLine( const char* line, unsigned version, DeliveryStats& ds );
    static void testRecord( const char* line, size_t fpos );

private:
    void createFile( msgtime_type currentTime, struct tm& now );
    void makeActLogPath( char* buf, const struct tm& now ) const;
    static void closeJoinedLog( const std::string& resultpath,
                                smsc::logger::Logger* thelog,
                                bool destroyDir );

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
