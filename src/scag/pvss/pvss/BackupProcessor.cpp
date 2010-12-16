#include <vector>
#include "BackupProcessor.h"
#include "core/buffers/File.hpp"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/data/Property.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/core/server/Server.h"
#include "util/PtrDestroy.h"
#include "PvssDispatcher.h"
#include <algorithm>

namespace scag2 {
namespace pvss {

class BackupProcessor::BackupProcessingTask : public smsc::core::threads::ThreadedTask
{
public:
    BackupProcessingTask( BackupProcessor&   processor,
                          const std::string& backup,
                          ScopeType scope ) :
    processor_(processor),
    scope_(scope),
    log_(0)
    {
        std::string logName(taskName());
        logName.push_back(scopeTypeToString(scope_)[0]);
        log_ = smsc::logger::Logger::getInstance(logName.c_str());

        // dirname
        dirname_ = backup;
        const size_t slash = dirname_.rfind('/');
        if ( slash != std::string::npos ) {
            filename_ = dirname_.substr(slash+1);
            dirname_.erase(slash);
        } else {
            filename_ = dirname_;
            dirname_ = ".";
        }

    }

    virtual const char* taskName() { return "bck.task"; }
    virtual int Execute();
    virtual void stop() {
        smsc::core::threads::ThreadedTask::stop();
        MutexGuard mg(mon_);
        mon_.notify();
    }

    void readDir( std::vector< std::string >& files );

private:
    BackupProcessor&                          processor_;
    std::string                               dirname_;
    std::string                               filename_;
    ScopeType                                 scope_;
    smsc::core::synchronization::EventMonitor mon_;
    smsc::logger::Logger*                     log_;
};


BackupProcessor::BackupProcessor( PvssDispatcher& dispatcher,
                                  const std::string& journalDir,
                                  size_t propertiesPerSec,
                                  const std::string& abonentBackup,
                                  const std::string& serviceBackup,
                                  const std::string& providerBackup,
                                  const std::string& operatorBackup ) :
dispatcher_(&dispatcher), stopping_(true),
propertiesPerSec_(propertiesPerSec),
log_(0),
journalDir_(journalDir),
abonentBackup_(abonentBackup),
serviceBackup_(serviceBackup),
providerBackup_(providerBackup),
operatorBackup_(operatorBackup)
{
    log_ = smsc::logger::Logger::getInstance("bck.proc");
    processedFiles_.resize(5,0);
    loadJournalDir( SCOPE_ABONENT );
    loadJournalDir( SCOPE_SERVICE );
    loadJournalDir( SCOPE_OPERATOR );
    loadJournalDir( SCOPE_PROVIDER );
}


BackupProcessor::~BackupProcessor()
{
    threadPool_.stopNotify();
    threadPool_.shutdown();
    std::for_each( processedFiles_.begin(), processedFiles_.end(), smsc::util::PtrDestroy() );
}


void BackupProcessor::process()
{
    smsc_log_info(log_,"backup processor is started");
    MutexGuard mg(stopMon_);
    stopping_ = false;

    startTask( abonentBackup_, SCOPE_ABONENT );
    startTask( operatorBackup_, SCOPE_OPERATOR );
    startTask( serviceBackup_, SCOPE_SERVICE );
    startTask( providerBackup_, SCOPE_PROVIDER );

    while ( ! stopping_ ) {
        stopMon_.wait(1000);
    }
    threadPool_.stopNotify();
    threadPool_.shutdown();
    smsc_log_info(log_,"backup processor is finished");
}


void BackupProcessor::stop()
{
    MutexGuard mg(stopMon_);
    stopping_ = true;
    stopMon_.notify();
}


/*
time_t BackupProcessor::readTime( const char* timestring )
{
    if ( !timestring ) return time_t(-1);
    unsigned year, month, day, hour;
    if ( 4 == sscanf(timestring,"%04u-%02u-%02u-%02u",&year,&month,&day,&hour) ) {
        smsc_log_debug(smsc::logger::Logger::getInstance("bck.proc"),
                       "readTime(\"%s\"): year=%u month=%u day=%u hour=%u",
                       timestring, year, month, day, hour );
        struct tm tmRead;
        ::memset(&tmRead,0,sizeof(tmRead));
        tmRead.tm_hour = hour;
        tmRead.tm_mday = day;
        tmRead.tm_mon = month-1;
        tmRead.tm_year = year-1900;
        time_t retval = mktime(&tmRead);
        return retval;
    }
    return time_t(-1);
}
 */


void BackupProcessor::startTask( const std::string& backup, ScopeType scope )
{
    if ( backup.empty() ) {
        smsc_log_warn(log_,"not starting task %s as backup is empty", scopeTypeToString(scope) );
        return;
    }
    threadPool_.startTask( new BackupProcessingTask(*this,backup,scope) );
}


void BackupProcessor::setFileProcessed( ScopeType scope, const char* fname )
{
    unsigned idx = scope;
    if ( idx >= processedFiles_.size() ) return;
    smsc::core::buffers::File fd;
    std::string fullname(journalDir_ + "/" + scopeTypeToString(scope) + "/" + fname);
    try {
        fd.RWCreate(fullname.c_str());
    } catch( std::exception& e ) {
        smsc_log_warn(log_,"cannot create file %s",fullname.c_str());
    }
    processedFiles_[idx]->insert(fname);
    smsc_log_info(log_,"file %s has been processed",fname);
}


bool BackupProcessor::isFileProcessed( ScopeType scope, const char* fname ) const
{
    unsigned idx = scope;
    if ( idx >= processedFiles_.size() ) return false;
    std::set< std::string >* theset = processedFiles_[idx];
    return ( theset->find(fname) != theset->end() );
}


void BackupProcessor::loadJournalDir( ScopeType scope )
{
    unsigned idx = scope;
    assert( idx < processedFiles_.size() );
    if ( processedFiles_[idx] == 0 ) {
        processedFiles_[idx] = new std::set< std::string >();
    }
    std::set< std::string >* theset = processedFiles_[idx];
    if ( ! smsc::core::buffers::File::Exists( journalDir_.c_str() ) ) {
        try {
            smsc::core::buffers::File::MkDir( journalDir_.c_str() );
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"cannot create dir %s: %s", journalDir_.c_str(), e.what());
            fprintf(stderr,"cannot create dir %s: %s\n", journalDir_.c_str(), e.what());
            exit(-1);
        }
    }
    std::string fulldir( journalDir_ + "/" + scopeTypeToString(scope) );
    if ( ! smsc::core::buffers::File::Exists( fulldir.c_str() ) ) {
        try {
            smsc::core::buffers::File::MkDir( fulldir.c_str() );
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"cannot create dir %s: %s", fulldir.c_str(), e.what());
            fprintf(stderr,"cannot create dir %s: %s\n", fulldir.c_str(), e.what());
            exit(-1);
        }
    }
    std::vector< std::string > entries;
    entries.reserve(50);
    smsc::core::buffers::File::ReadDir( fulldir.c_str(), entries );
    theset->insert( entries.begin(), entries.end() );
}


int BackupProcessor::BackupProcessingTask::Execute()
{
    smsc_log_info(log_,"staring processing task %s",scopeTypeToString(scope_));

    while ( ! isStopping ) {

        // reading dir
        std::vector< std::string > logFiles;
        readDir( logFiles );
        if ( logFiles.empty() ) {
            MutexGuard mg(mon_);
            smsc_log_debug(log_,"cycling %s",scopeTypeToString(scope_));
            mon_.wait(10000);
            continue;
        }

        // processing entries
        const util::msectime_type startTime = util::currentTimeMillis();
        size_t processed = 0;
        for ( std::vector< std::string >::const_iterator i = logFiles.begin();
              i != logFiles.end();
              ++i ) {
            const std::string fullname( dirname_ + "/" + i->c_str() );
            smsc_log_info(log_,"processing %s",fullname.c_str());

            if ( isStopping ) break;

            File fd;
            try {
                fd.ROpen(fullname.c_str());

                std::string line, prevline;
                line.reserve(200);
                while ( fd.ReadLine(line) ) {

                    if ( isStopping && prevline.empty() ) break;

                    // processing one line

                    char logLevel, act;
                    unsigned day, month, year, hour, minute, second, msec, thread;
                    int ikey;
                    char cat[30], skey[60];
                    int propstart;
                    
                    bool scanfok = false;
                    do {

                        int sc;
                        if ( scope_ == SCOPE_ABONENT ) {
                            sc = sscanf(line.c_str(), 
                                        "%c %02u-%02u-%04u %02u:%02u:%02u,%03u %u %s %c key=%s %n",
                                        &logLevel,&day,&month,&year,&hour,&minute,&second,&msec,&thread,
                                        cat, &act, skey, &propstart );
                        } else {
                            sc = sscanf(line.c_str(), 
                                        "%c %02u-%02u-%04u %02u:%02u:%02u,%03u %u %s %c key=%u %n",
                                        &logLevel,&day,&month,&year,&hour,&minute,&second,&msec,&thread,
                                        cat, &act, &ikey, &propstart );
                        }
                        if ( sc < 12 ) {
                            smsc_log_warn(log_,"cannot sscanf(%s): %d",line.c_str(),sc);
                            if ( ! prevline.empty() ) {
                                // trying to combine with prevline
                                prevline.reserve(prevline.size() + line.size() + 2);
                                prevline.push_back('\n');
                                prevline.append(line);
                                line.swap(prevline);
                                prevline.clear();
                                continue;
                            }
                        } else {
                            scanfok = true;
                            if ( ! prevline.empty() ) {
                                smsc_log_warn(log_,"scanf ok while prevline=%s", prevline.c_str());
                                prevline.clear();
                            }
                        }

                        break;
                    } while ( true );
                    if ( ! scanfok ) { continue; }

                    ProfileKey profileKey;
                    switch (scope_) {
                    case (SCOPE_ABONENT) : profileKey.setAbonentKey(skey); break;
                    case (SCOPE_SERVICE) : profileKey.setServiceKey(ikey); break;
                    case (SCOPE_OPERATOR) : profileKey.setOperatorKey(ikey); break;
                    case (SCOPE_PROVIDER) : profileKey.setProviderKey(ikey); break;
                    default: throw smsc::util::Exception("cannot set profile key of unknown scope %u", scope_);
                    }

                    ProfileCommand* cmd = 0;
                    switch (act) {
                    case 'A' :
                    case 'U' : {
                        int shift;
                        int sc = sscanf( line.c_str()+propstart,"property=%n", &shift );
                        if ( sc < 0 ) {
                            smsc_log_warn(log_,"cannot scanf property: %s", line.c_str()+propstart);
                            prevline = line;
                            continue;
                        }
                        propstart += shift;
                        Property property;
                        try {
                            property.fromString( line.c_str() + propstart );
                        } catch (...) {
                            smsc_log_warn(log_,"cannot parse property: %s", line.c_str() + propstart );
                            prevline = line;
                            continue;
                        }
                        SetCommand* kmd = new SetCommand();
                        kmd->setProperty(property);
                        cmd = kmd;
                        break;
                    }
                    case 'E' :
                    case 'D' : {
                        int shift;
                        int sc = sscanf( line.c_str()+propstart,"name=%n", &shift );
                        if ( sc < 0 ) {
                            smsc_log_warn(log_,"cannot scanf property name: %s", line.c_str()+propstart);
                            prevline = line;
                            continue;
                        }
                        DelCommand* kmd = new DelCommand();
                        kmd->setVarName( line.c_str() + propstart + shift );
                        cmd = kmd;
                        break;
                    }
                    case 'G' :
                        smsc_log_debug(log_,"get action in '%s'",line.c_str());
                        continue;
                    default:
                        smsc_log_warn(log_,"unknown action '%c' in %s",act,line.c_str());
                        continue;
                    }

                    ProfileRequest request( profileKey, cmd );

                    // passing an entry to a pvss dispatcher
                    core::server::Server::SyncLogic* logic = 
                        processor_.dispatcher_->getSyncLogic(processor_.dispatcher_->getIndex(request));
                    if ( !logic ) {
                        smsc_log_warn(log_,"cannot obtain sync logic on key=%s",profileKey.toString().c_str());
                        continue;
                    }
                    delete logic->process(request);

                    if ( (++processed % 100) == 0 ) {
                        const util::msectime_type now = util::currentTimeMillis();
                        const util::msectime_type elapsedTime = now - startTime;
                        const unsigned expected = unsigned(elapsedTime*processor_.propertiesPerSec_/1000);
                        if ( processed > expected ) {
                            // we have to wait
                            int waitTime = int((processed - expected) * 1000ULL / processor_.propertiesPerSec_);
                            if ( waitTime > 10 ) {
                                MutexGuard mg(mon_);
                                mon_.wait(waitTime);
                            }
                        }
                    }

                } // while readline

                if ( ! line.empty() ) {
                    smsc_log_warn(log_,"no LF at EOF %s",i->c_str());
                }
                    
            } catch ( std::exception& e ) {
                smsc_log_warn(log_,"cannot process %s: %s", fullname.c_str(), e.what());
            } catch (...) {
                smsc_log_warn(log_,"cannot process %s", fullname.c_str());
            }
            
            processor_.setFileProcessed( scope_, i->c_str() );

        } // loop over file names
    }
    smsc_log_info(log_,"stopping processing task %s",scopeTypeToString(scope_));
    return 0;
}


void BackupProcessor::BackupProcessingTask::readDir( std::vector< std::string >& files )
{
    std::string format(filename_ + ".%[0123456789-].log%n");
    DIR* dir = opendir( dirname_.c_str() );
    if ( !dir ) {
        smsc_log_debug(log_,"cannot open dir %s", dirname_.c_str());
        return;
    }
    smsc::core::buffers::TmpBuf<char,512> dirbuf(sizeof(dirent)+
                                                 pathconf(dirname_.c_str(),_PC_NAME_MAX)+1);
    dirent* entry = reinterpret_cast<dirent*>(dirbuf.get());
    dirent* result;
    std::auto_ptr<char> tail;
    size_t taillen = 0;
    while ( 0 == readdir_r(dir,entry,&result) && result != 0 ) {
        // dir entry is read
        smsc_log_debug(log_,"entry: %s",result->d_name);
        const size_t reslen = ::strlen(result->d_name) + 1;
        if ( taillen < reslen ) {
            taillen = reslen;
            tail.reset( new char[reslen]);
        }
        int shift = 0;
        int rv = sscanf(result->d_name,format.c_str(),tail.get(),&shift);
        if ( rv <= 0 ) {
            smsc_log_debug(log_,"sscanf returned %d",rv);
            continue;
        }
        if (shift == 0) {
            continue;
        }
        if ( processor_.isFileProcessed(scope_,result->d_name) ) {
            smsc_log_debug(log_,"file %s is already processed",result->d_name);
            continue;
        }
        files.push_back( result->d_name );
    }
    closedir(dir);
    std::sort( files.begin(), files.end() );
}

} // namespace pvss
} // namespace scag2
