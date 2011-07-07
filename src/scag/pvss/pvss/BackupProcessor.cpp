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
            break;
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

    /*
    if ( ! smsc::core::buffers::File::Exists( archiveDir_.c_str() ) ) {
        try {
            smsc::core::buffers::File::MkDir( archiveDir_.c_str() );
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"cannot create dir %s: %s", archiveDir_.c_str(), e.what());
            fprintf(stderr,"cannot create dir %s: %s\n", archiveDir_.c_str(), e.what());
            exit(-1);
        }
    }
     */
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


/*
void BackupProcessor::startTask( ScopeType scope, const std::string& backup )
{
    if ( backup.empty() ) {
        smsc_log_warn(log_,"not starting task %s as backup is empty", scopeTypeToString(scope) );
        return;
    }
    threadPool_.startTask( new BackupProcessingTask(*this,scope,backup) );
}
 */


// ==================================================


/*
void BackupProcessor::BackupProcessingTask::setFileProcessed( const char* fname,
                                                              time_t nextTime )
{
    smsc::core::buffers::File fd;
    std::string fullname(processor_.archiveDir_ + "/" + scopeTypeToString(scope_) + "/" + fname);
    try {
        fd.RWCreate(fullname.c_str());
    } catch( std::exception& e ) {
        smsc_log_warn(log_,"cannot create file %s",fullname.c_str());
    }
    processed_.insert(fname);
    if ( nextTime > lastTime_ ) {
        lastTime_ = nextTime;
    }
    smsc_log_info(log_,"file %s has been processed",fname);
}
 */


/*
void BackupProcessor::loadJournalDir( ScopeType scope, const std::string& backup )
{
    // setting prefixes
    const unsigned idx = scope;
    backupPath_[idx] = backup;
    if ( backup.empty() ) return;

    const size_t slash = backup.rfind('/');
    if ( slash != std::string::npos ) {
        backupFileFormat_[idx] = backup.substr(slash+1);
        backupPath_[idx].erase(slash);
    } else {
        backupFileFormat_[idx] = backup;
        backupPath_[idx] = ".";
    }
    backupFileFormat_[idx] += ".%04u-%02u-%02u-%02u.log%n";

    assert( idx < processedFiles_.size() );
    if ( processedFiles_[idx] == 0 ) {
        processedFiles_[idx] = new std::set< std::string >();
    }
    std::set< std::string >* theset = processedFiles_[idx];
    if ( ! smsc::core::buffers::File::Exists( archiveDir_.c_str() ) ) {
        try {
            smsc::core::buffers::File::MkDir( archiveDir_.c_str() );
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"cannot create dir %s: %s", archiveDir_.c_str(), e.what());
            fprintf(stderr,"cannot create dir %s: %s\n", archiveDir_.c_str(), e.what());
            exit(-1);
        }
    }
    std::string fulldir( archiveDir_ + "/" + scopeTypeToString(scope) );
    if ( ! smsc::core::buffers::File::Exists( fulldir.c_str() ) ) {
        try {
            smsc::core::buffers::File::MkDir( fulldir.c_str() );
            return;
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

    // get the last time of the files
    if ( ! theset->empty() ) {
        const time_t last = readTime( scope, theset->rbegin()->c_str() );
        if (last > lastTime_[idx]) {
            lastTime_[idx] = last;
        }
    }
}
 */


/*
time_t BackupProcessor::BackupProcessingTask::readTime( const char* fname ) const
{
    int pos = 0;
    struct tm tmo;
    sscanf( fname,
            fileFormat_.c_str(),
            &tmo.tm_year, &tmo.tm_mon, &tmo.tm_mday, &tmo.tm_hour, &pos );

    if (!pos) {
        throw smsc::util::Exception("the file %s has wrong name", fname);
    }

    tmo.tm_year -= 1900;
    --tmo.tm_mon;
    tmo.tm_isdst = -1;
    return mktime(&tmo);
}
 */


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
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"readNextFile exc: %s",e.what());
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
                    throw FileReadException(filename.c_str(),0,e.getPos(),
                                            "persistent parsing exc: %s",
                                            e.what());
                }
                badFilePos = e.getPos();
                MutexGuard mg(mon_);
                mon_.wait(failureSleep);
                continue;
            } // other exceptions are not handled

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
        std::exit(1);
    }
    return 0;
}


/*

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
            
            setFileProcessed( i->c_str(), nextTime );

        } // loop over file names
    }
    smsc_log_info(log_,"stopping processing task %s",scopeTypeToString(scope_));
    return 0;
}


void BackupProcessor::BackupProcessingTask::readDir( std::vector< std::string >& files )
{
    DIR* dir = opendir( dirname_.c_str() );
    if ( !dir ) {
        smsc_log_debug(log_,"cannot open dir '%s'", dirname_.c_str());
        return;
    }
    smsc::core::buffers::TmpBuf<char,512> dirbuf(sizeof(dirent)+
                                                 pathconf(dirname_.c_str(),_PC_NAME_MAX)+1);
    dirent* entry = reinterpret_cast<dirent*>(dirbuf.get());
    dirent* result;
    // std::auto_ptr<char> tail;
    // size_t taillen = 0;
    while ( 0 == readdir_r(dir,entry,&result) && result != 0 ) {
        if ( strcmp(".",result->d_name) == 0 ||
             strcmp("..",result->d_name) == 0 ) continue;
        // dir entry is read
        smsc_log_debug(log_,"entry: %s",result->d_name);
        try {
            // const time_t nextTime = 
            readTime(result->d_name);
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"wrong file '%s'",result->d_name);
            continue;
        }
        if ( processed_.find(result->d_name) != processed_.end() ) {
            smsc_log_debug(log_,"file '%s' is already processed",result->d_name);
            continue;
        }
        files.push_back( result->d_name );
    }
    closedir(dir);
    std::sort( files.begin(), files.end() );
}
 */

} // namespace pvss
} // namespace scag2
