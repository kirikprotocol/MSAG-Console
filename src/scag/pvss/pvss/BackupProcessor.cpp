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
#include "informer/io/FileGuard.h"
#include "informer/io/DirListing.h"
#include "informer/io/InfosmeException.h"
#include "scag/pvss/profile/RollingFileStream.h"

using eyeline::informer::makeDirListing;
using eyeline::informer::InfosmeException;
using eyeline::informer::ErrnoException;
using eyeline::informer::FileReadException;
using eyeline::informer::FileGuard;
using eyeline::informer::NoDotsNameFilter;
using eyeline::informer::EXC_IOERROR;
using eyeline::informer::EXC_BADFILE;

namespace scag2 {
namespace pvss {

struct BackupProcessor::FileNameFilter
{
    FileNameFilter( const BackupProcessor& processor ) : proc_(processor) {}

    inline bool operator() ( const char* fn ) const {
        // the name must start with backupPrefix_
        // and ends with backupSuffix_;
        const size_t len = strlen(fn);
        if ( len <= proc_.backupPrefix_.size() + proc_.backupSuffix_.size() ) {
            return false;
        }
        if ( memcmp(proc_.backupPrefix_.c_str(),fn,
                    proc_.backupPrefix_.size()) ) {
            return false;
        }
        if ( memcmp(proc_.backupSuffix_.c_str(),
                    fn+len-proc_.backupSuffix_.size(),
                    proc_.backupSuffix_.size()) ) {
            return false;
        }
        return true;
    }
    
private:
    const BackupProcessor& proc_;
};


// ==================================================================

struct BackupProcessor::BackupParser : public ProfileLogStreamRecordParser
{
    BackupParser( BackupProcessor& proc ) : proc_(proc) {}
    virtual void parseRecord( const char* record, size_t reclen )
    {
        if ( record[0] == '#' ) {
            // FIXME: impl read version
            return;
        }

        // parsing the record
        ::tm ltm;
        const char* ptr = record;
        size_t ptrlen = reclen;
        const size_t pfxlen = RollingFileStream::scanPrefix( ptr,
                                                             ptrlen,
                                                             ltm,
                                                             catname_ );
        assert( ptrlen > pfxlen );

        // determine the scope using fast switch
        if ( catname_.size() != 9 ) {
            throw InfosmeException(EXC_IOERROR,"bad category in '%.*s'",reclen,record);
        }
        ScopeType scope = ScopeType(0);
        switch (catname_[5]) {
        case 'a' :
            if ( !strcmp(catname_.c_str(),"pvss.abnt") ) { scope = SCOPE_ABONENT; }
            break;
        case 'o' :
            if ( !strcmp(catname_.c_str(),"pvss.oper") ) { scope = SCOPE_OPERATOR; }
            break;
        case 'p' :
            if ( !strcmp(catname_.c_str(),"pvss.prov") ) { scope = SCOPE_PROVIDER; }
            break;
        case 's' :
            if ( !strcmp(catname_.c_str(),"pvss.serv") ) { scope = SCOPE_SERVICE; }
            break;
        default:
            break;
        }
        if ( scope == ScopeType(0) ) {
            throw InfosmeException(EXC_IOERROR,"invalid category in '%.*s'",reclen,record);
        }

        ptr += pfxlen;
        ptrlen -= pfxlen;

        // reading the operation type and key
        int pos = 0;
        char act;
        ProfileKey profileKey;
        if ( scope == SCOPE_ABONENT ) {
            char skey[60];
            sscanf( ptr, "%c key=%59s %n", &act, skey, &pos );
            if ( pos <= 0 || pos > int(ptrlen) ) {
                throw InfosmeException(EXC_IOERROR,"invalid act/key in '%.*s'",reclen,record);
            }
            profileKey.setAbonentKey( skey );
        } else {
            unsigned ikey;
            sscanf( ptr, "%c key=%u %n", &act, &ikey, &pos );
            if ( pos <= 0 || pos > int(ptrlen) ) {
                throw InfosmeException(EXC_IOERROR,"invalid act/key in '%.*s'",reclen,record);
            }
            switch (scope) {
            case SCOPE_SERVICE : profileKey.setServiceKey(ikey); break;
            case SCOPE_OPERATOR : profileKey.setOperatorKey(ikey); break;
            case SCOPE_PROVIDER : profileKey.setProviderKey(ikey); break;
            default: abort();
            }
        }
        std::auto_ptr<ProfileCommand> cmd;

        // reading property
        ptr += size_t(pos);
        ptrlen -= size_t(pos);
        pos = 0;
        switch (act) {
        case 'A':
        case 'U' : {
            sscanf(ptr,"property=%n",&pos);
            if ( pos <= 0 || pos >= int(ptrlen) ) {
                throw InfosmeException(EXC_IOERROR,"invalid property in '%.*s'",reclen,record);
            }
            Property property;
            try {
                property.fromString(ptr+pos);
            } catch ( std::exception& e ) {
                throw InfosmeException(EXC_IOERROR,"cannot parse property in '%.*s'",reclen,record);
            }
            SetCommand* kmd = new SetCommand();
            cmd.reset(kmd);
            kmd->setProperty(property);
            break;
        }
        case 'E' :
        case 'D' : {
            sscanf(ptr,"name=%n",&pos);
            if ( pos <= 0 || pos >= int(ptrlen) ) {
                throw InfosmeException(EXC_IOERROR,"invalid property in '%.*s'",reclen,record);
            }
            DelCommand* kmd = new DelCommand();
            cmd.reset(kmd);
            kmd->setVarName( ptr + pos );
            break;
        }
        case 'G' : {
            smsc_log_warn(proc_.log_,"action G is ignored on key=%s",profileKey.toString().c_str());
            return;
        }
        default:
            throw InfosmeException(EXC_IOERROR,"unknown action '%c' in '%.*s'",act,reclen,record);
        }
        
        ProfileRequest request(profileKey,cmd.release());
        core::server::Server::SyncLogic* logic =
            proc_.dispatcher_->getSyncLogic(proc_.dispatcher_->getIndex(request));
        if (!logic) {
            smsc_log_warn(proc_.log_,"cannot obtain logic on key=%s",profileKey.toString().c_str());
            return;
        }
        delete logic->process(request,true);
        proc_.controlSpeed();
    }

private:
    BackupProcessor& proc_;
    std::string catname_;   // working buffer for catname
};

// ==================================================================

class BackupProcessor::BackupProcessingTask : public smsc::core::threads::ThreadedTask
{
public:
    BackupProcessingTask( BackupProcessor& processor ) :
    processor_(processor),
    log_(smsc::logger::Logger::getInstance(taskName()))
    {
        // std::string logName(taskName());
        // logName.push_back(scopeTypeToString(scope_)[0]);
        // log_ = smsc::logger::Logger::getInstance(logName.c_str());

        /*
        const size_t slash = backup.rfind('/');
        if ( slash != std::string::npos ) {
            fileFormat_ = backup.substr(slash+1);
            dirname_ = backup.substr(0,slash);
        } else {
            fileFormat_ = backup;
            dirname_ = ".";
        }
        fileFormat_ += ".%04u-%02u-%02u-%02u.log%n";

        std::vector< std::string > entries;
        entries.reserve(50);
        smsc::core::buffers::File::ReadDir( fulldir.c_str(), entries );
        processed_.insert( entries.begin(), entries.end() );

        // get the last time of the files
        if ( ! processed_.empty() ) {
            const time_t last = readTime(processed_.rbegin()->c_str());
            if (last > lastTime_) {
                lastTime_ = last;
            }
        }
         */
    }

    virtual const char* taskName() { return "bck.task"; }
    virtual int Execute();
    virtual void stop() {
        smsc::core::threads::ThreadedTask::stop();
        MutexGuard mg(mon_);
        mon_.notify();
    }

    // void readDir( std::vector< std::string >& files );
    // time_t readTime( const char* fname ) const;
    // void setFileProcessed( const char* fname, time_t nextTime );

private:
    BackupProcessor&                          processor_;
    smsc::core::synchronization::EventMonitor mon_;
    smsc::logger::Logger*                     log_;
    /*
    std::string                               dirname_;
    std::string                               fileFormat_;
    time_t                                    lastTime_;
    std::set<std::string>                     processed_;
     */
};


BackupProcessor::BackupProcessor( PvssDispatcher& dispatcher,
                                  const std::string& archiveDir,
                                  size_t propertiesPerSec,
                                  const std::string& backupPrefix,
                                  const std::string& backupSuffix ) :
dispatcher_(&dispatcher),
stopping_(false),
propertiesPerSec_(propertiesPerSec),
log_(0),
archiveDir_(archiveDir),
inputDir_(backupPrefix),
backupPrefix_(backupPrefix),
backupSuffix_(backupSuffix),
speedControl_(propertiesPerSec)
{
    log_ = smsc::logger::Logger::getInstance("bck.proc");

    size_t lastSlash = backupPrefix_.rfind('/');
    if ( lastSlash == std::string::npos ) {
        inputDir_ = "./";
    } else {
        inputDir_.erase(lastSlash+1);
        backupPrefix_.erase(0,lastSlash+1);
    }

    if ( archiveDir_.empty() ) {
        archiveDir_ = "./";
    } else if ( archiveDir_[archiveDir_.size()-1] != '/' ) {
        archiveDir_.push_back('/');
    }
}


BackupProcessor::~BackupProcessor()
{
    {
        MutexGuard mg(stopMon_);
        stopping_ = true;
        stopMon_.notifyAll();
    }
    threadPool_.shutdown();
}


void BackupProcessor::start()
{
    smsc_log_info(log_,"backup processor is starting");
    {
        MutexGuard mg(stopMon_);
        stopping_ = false;
    }
    threadPool_.startTask( new BackupProcessingTask(*this) );
    smsc_log_info(log_,"backup processor is started");
}


void BackupProcessor::stop()
{
    smsc_log_info(log_,"backup processor is stopping");
    {
        MutexGuard mg(stopMon_);
        stopping_ = true;
        stopMon_.notifyAll();
    }
    threadPool_.stopNotify();
    threadPool_.shutdown();
    smsc_log_info(log_,"backup processor is stopped");
}


std::string BackupProcessor::readTheNextFile( const std::string& dir )
{
    std::vector< std::string > entries;
    makeDirListing(FileNameFilter(*this),S_IFREG).list(dir.c_str(),entries);
    if ( ! entries.empty() ) {
        // there are files
        std::sort( entries.begin(), entries.end() );
        for ( std::vector< std::string >::reverse_iterator i = entries.rbegin();
              i != entries.rend();
              ++i ) {
            if ( extractDateTime(*i) ) {
                // good file found, we have to parse its trail
                RollingFileStreamReader rfsr;
                std::string filename(dir);
                filename.append(*i);
                const std::string res = rfsr.readNextFile(filename.c_str());
                if (res.empty()) {
                    throw InfosmeException(EXC_IOERROR,"invalid file '%s' in archive",i->c_str());
                }
                return res;
            }
        }
    }
    // we could not find any suitable file
    entries.clear();
    makeDirListing(NoDotsNameFilter(),S_IFDIR).list(dir.c_str(),entries);
    std::sort(entries.begin(), entries.end());
    for ( std::vector< std::string >::reverse_iterator i = entries.rbegin();
          i != entries.rend(); ++i ) {
        std::string subdir(dir);
        subdir.append(*i);
        subdir.push_back('/');
        const std::string res = readTheNextFile(subdir);
        if ( !res.empty() ) { return res; }
    }
    return std::string();
}


time_t BackupProcessor::extractDateTime( const std::string& fn ) const
{
    return RollingFileStream::extractTime( fn.c_str(),
                                           backupPrefix_.c_str(),
                                           backupSuffix_.c_str() );
}


void BackupProcessor::controlSpeed()
{
    ++speedCounter_;
    speedControl_.consumeQuant();
    const util::msectime_type now = util::currentTimeMillis();
    static util::msectime_type maxDelay = 5000;
    util::msectime_type towait = speedControl_.isReady( now, maxDelay );
    if ( towait > 10 && !stopping_ ) {
        MutexGuard mg(stopMon_);
        stopMon_.wait(towait);
    }
}


int BackupProcessor::BackupProcessingTask::Execute()
{
    const int failureSleep = 3000;
    smsc_log_info(log_,"starting backup processing task");

    try {
        
        // 1. collect all files in journal dir
        // 2. read the last file tail to find out what file should be next
        std::string nextFileName;
        try {
            nextFileName = processor_.readTheNextFile( processor_.archiveDir_ );
        } catch ( FileReadException& e ) {
            smsc_log_error(log_,"exc in '%s': %s",e.getFileName(),e.what());
            throw InfosmeException(e.getCode(),"%s in file '%s'",e.what(),e.getFileName());
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"readNextFile exc: %s",e.what());
            throw;
        }
        if ( nextFileName.empty() ) {
            smsc_log_info(log_,"next backup file is empty, so this is the first run");
        } else {
            smsc_log_info(log_,"next backup file found: %s", nextFileName.c_str());
        }

        // then in the loop

        bool wasnotthere = false;
        size_t badFilePos = size_t(-1);
        while ( ! isStopping ) {

            time_t fileDateTime = 0;

            if ( nextFileName.empty() ) {
                // none of the files was processed yet
                std::vector< std::string > entries;
                FileNameFilter cgf(processor_);
                makeDirListing(cgf,S_IFREG).list( processor_.inputDir_.c_str(),
                                                  entries );
                std::sort( entries.begin(), entries.end() );
                fileDateTime = 0;
                for ( std::vector< std::string >::const_iterator i = entries.begin();
                      i != entries.end(); ++i ) {
                    try {
                        fileDateTime = processor_.extractDateTime(*i);
                        if ( fileDateTime ) {
                            nextFileName = *i;
                            break;
                        }
                    } catch ( std::exception& e ) {
                        smsc_log_debug(log_,"file '%s' has invalid format, skipped",i->c_str());
                    }
                }
                if ( !fileDateTime ) {
                    MutexGuard mg(mon_);
                    mon_.wait(failureSleep);
                    continue;
                }
            } else {
                fileDateTime = processor_.extractDateTime(nextFileName);
            }

            std::string filename(processor_.inputDir_);
            filename.append(nextFileName);

            // 3. wait for this file
            struct stat st;
            if ( -1 == stat(filename.c_str(),&st) ) {
                if ( errno == ENOENT ) {
                    // not present yet
                    MutexGuard mg(mon_);
                    mon_.wait(failureSleep);
                    wasnotthere = true;
                    continue;
                }
                throw ErrnoException(errno,"stat");
            } else if ( ! S_ISREG(st.st_mode) ) {
                throw ErrnoException(errno,"stat");
            }

            if ( wasnotthere ) {
                // wait one additional second to allow file to settle
                wasnotthere = false;
                MutexGuard mg(mon_);
                mon_.wait(failureSleep);
                if ( isStopping ) { break; }
            }

            // 4. process it
            RollingFileStreamReader rfsr;
            try {
                BackupParser bp(processor_);
                smsc_log_info(log_,"starting to process %s",nextFileName.c_str());
                rfsr.read( filename.c_str(), &isStopping, &bp);
            } catch ( FileReadException& e ) {
                smsc_log_warn(log_,"file '%s' parsing exc at pos=%llu: %s",
                              filename.c_str(),
                              static_cast<unsigned long long>(e.getPos()),
                              e.what() );
                if ( badFilePos == e.getPos() ) {
                    throw InfosmeException(EXC_BADFILE,
                                           "exc in file '%s': %s",
                                           filename.c_str(),e.what());
                }
                badFilePos = e.getPos();
                MutexGuard mg(mon_);
                mon_.wait(failureSleep);
                continue;
            } catch ( std::exception& e ) {
                throw InfosmeException(EXC_BADFILE,"exc in file '%s': %s",
                                       filename.c_str(),e.what());
            }

            badFilePos = size_t(-1); // reset bad file pos

            if ( ! rfsr.isFinished() ) {
                smsc_log_warn(log_,"file '%s' is not yet finished",
                              filename.c_str());
                MutexGuard mg(mon_);
                mon_.wait(failureSleep);
                continue;
            }

            smsc_log_info(log_,"file %s has been read, records=%u crc=%u",
                          nextFileName.c_str(), rfsr.getLines(), rfsr.getCrc32() );

            // 5. move the file to journal dir
            {
                char subdirbuf[128];
                ::tm ltm;
                ::localtime_r(&fileDateTime,&ltm);
                sprintf( subdirbuf, "%04u/%02u/%02u/%02u/",
                         ltm.tm_year+1900, ltm.tm_mon+1, ltm.tm_mday, ltm.tm_hour );
                std::string subdir(processor_.archiveDir_);
                subdir.append(subdirbuf);
                FileGuard::makedirs(subdir);
                // move the file
                subdir.append(nextFileName);
                if ( 0 == rename(filename.c_str(),subdir.c_str()) ) {
                    // success
                } else if ( errno == EXDEV ) {
                    // different devices, trying to copy
                    FileGuard::copyfile( filename.c_str(), (subdir + ".tmp").c_str() );
                    if ( -1 == rename((subdir + ".tmp").c_str(), subdir.c_str()) ) {
                        throw ErrnoException(errno,"rename(%s)",subdir.c_str());
                    }
                    try {
                        FileGuard::unlink( filename.c_str() );
                    } catch ( std::exception& ) {
                        smsc_log_warn(log_,"file %s has been copied but not removed",
                                      nextFileName.c_str());
                    }
                }
            }

            // 6. update nextfile var, goto 3
            nextFileName = rfsr.getNextFile();
        }
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"backup proc exc: %s",e.what());
        fprintf(stderr,"backup proc exc: %s\n",e.what());
        std::exit(17);
    }
    return 0;
}

} // namespace pvss
} // namespace scag2
