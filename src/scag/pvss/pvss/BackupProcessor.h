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
                     size_t propertiesPerSec );
    virtual ~BackupProcessor();

    /// start processor and lock until exited
    void process();
    void stop();
    void startTask( ScopeType scope, const std::string& backup );

private:
    // void setFileProcessed( ScopeType scope, const char* fname );
    // bool isFileProcessed( ScopeType scope, const char* fname ) const;
    // void loadJournalDir( ScopeType scope, const std::string& backup );
    // const std::string& getPath( ScopeType scope ) const;
    // const std::string& getFileFormat( ScopeType scope ) const;
    // time_t getLastTime( ScopeType scope ) const;
    // time_t readTime( ScopeType scope, const char* fname ) const;

private:
    PvssDispatcher*                           dispatcher_; // not owned
    bool                                      stopping_;
    size_t                                    propertiesPerSec_;
    smsc::logger::Logger*                     log_;
    smsc::core::threads::ThreadPool           threadPool_;
    smsc::core::synchronization::EventMonitor stopMon_;
    std::string                               journalDir_;
};

} // namespace pvss
} // namespace scag2

#endif // _SCAG_PVSS_PVSS_BACKUPPROCESSOR_H
