#ifndef _SCAG_PVSS_PVSS_BACKUPPROCESSOR_H
#define _SCAG_PVSS_PVSS_BACKUPPROCESSOR_H

#include <set>

#include "logger/Logger.h"
#include "core/threads/ThreadPool.hpp"
#include "scag/pvss/common/ScopeType.h"
#include "core/synchronization/EventMonitor.hpp"
// #include "scag/util/WatchedThreadedTask.h"

namespace scag2 {
namespace pvss {

class PvssDispatcher;

class BackupProcessor
{
private:
    class BackupProcessingTask;

public:
    BackupProcessor( PvssDispatcher& dispatcher,
                     const std::string& journalDir,
                     size_t propertiesPerSec,
                     const std::string& abonentBackup,
                     const std::string& serviceBackup,
                     const std::string& providerBackup,
                     const std::string& operatorBackup );
    virtual ~BackupProcessor();

    /// start processor and lock until exited
    void process();
    void stop();

    // static time_t readTime( const char* timestring );

private:
    void startTask( const std::string& backup, ScopeType scope );
    void setFileProcessed( ScopeType scope, const char* fname );
    bool isFileProcessed( ScopeType scope, const char* fname ) const;
    void loadJournalDir( ScopeType scope );

private:
    PvssDispatcher*                           dispatcher_; // not owned
    bool                                      stopping_;
    size_t                                    propertiesPerSec_;
    smsc::logger::Logger*                     log_;
    smsc::core::threads::ThreadPool           threadPool_;
    smsc::core::synchronization::EventMonitor stopMon_;
    std::string                               journalDir_;
    std::string                               abonentBackup_;
    std::string                               serviceBackup_;
    std::string                               providerBackup_;
    std::string                               operatorBackup_;
    std::vector< std::set< std::string >* >   processedFiles_;
};

} // namespace pvss
} // namespace scag2

#endif // _SCAG_PVSS_PVSS_BACKUPPROCESSOR_H
