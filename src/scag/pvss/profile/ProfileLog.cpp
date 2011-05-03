#include <cassert>
#include <string.h>
#include "ProfileLog.h"
// #include "RollingFileStream.h"
#include "util/Exception.hpp"
#include "informer/io/InfosmeException.h"

using eyeline::informer::EmbedRefPtr;

namespace scag2 {
namespace pvss {

ProfileLogStream::ProfileLogStream( const char* name ) :
reflock_(),
ref_(0),
lock_(),
name_(name),
needPostfix_(false)
{
}


// ===================================================================

ProfileLog::ProfileLog( const char* name ) :
name_(name),
level_(smsc::logger::Logger::LEVEL_FATAL),
uselog_(false),
stream_(0),
toset_(0),
llog_(smsc::logger::Logger::getInstance(name))
{
}


ProfileLog::~ProfileLog() {}


void ProfileLog::init( smsc::logger::Logger::LogLevel level,
                       ProfileLogStream*              stream,
                       bool                           useLog )
{
    toset_.reset(stream);
    level_ = level;
    uselog_ = useLog;
}


void ProfileLog::logva( int level, const char* const format, va_list args ) throw()
{
    char buf[2048];

    class BufferGuard {
    public:
        BufferGuard() : buf_(0) {}
        ~BufferGuard() { if (buf_) { delete [] buf_; } }
        void setBuf( char* buf ) {
            if (buf_) {
                delete [] buf_;
            }
            buf_ = buf;
        }
    private:
        char* buf_;
    } bufguard;

    if (stream_.get() != toset_.get() && toset_.get()) {
        stream_ = toset_;
    }
    if (!stream_.get()) {
        fprintf(stderr,"logger is not initialized");
        std::terminate();
    }

    const size_t prefixLength = stream_->formatPrefix( buf, sizeof(buf), name_.c_str() );

    char* buffer = buf;
    size_t bufsize = sizeof(buf) - prefixLength;
    try {

        while (true) {
    
            int n = ::vsnprintf( buffer + prefixLength, bufsize, format, args );
    
            if ( (n>-1) && size_t(n) < bufsize ) {
                bufsize = size_t(n) + prefixLength;
                break;
            }
            bufsize = ( n > -1 ) ?
                size_t(n + 1) :  // ISO/IEC 9899:1999
                bufsize*2;       // twice the old size
            bufguard.setBuf( buffer = new char[bufsize+prefixLength] );
            memcpy(buffer,buf,prefixLength);
    
        }

    } catch ( std::exception& e ) {
        fprintf(stderr,"out of memory");
        return;
    }

    // replace trailing \0 with \n
    buffer[bufsize] = '\n';
    stream_->write( buffer, ++bufsize );
}


/*
size_t ProfileLog::formatPrefix( char* buf, size_t bufsize, usectime_type now ) const throw()
{
    const unsigned msm = unsigned((now / 1000) % 10000000); // min, sec, msec
    return size_t(::snprintf(buf,bufsize,"%02u:%02u,%03u ",msm / 100000, (msm/1000) % 100, msm % 1000));
}


void ProfileLog::init( const char* pathPrefix,
                       const smsc::logger::Logger::LogLevel level,
                       unsigned interval )
{
    MutexGuard mg(lock_);
    level_ = int(level);
    size_t pathlen;
    if ( !pathPrefix || !(pathlen = strlen(pathPrefix)) ) {
        throw smsc::util::Exception("empty path prefix");
    }
    if ( pathPrefix[pathlen-1] == '/' ) {
        throw smsc::util::Exception("path prefix '%s' ends with slash",pathPrefix);
    }
    if ( strchr(pathPrefix,'%') ) {
        throw smsc::util::Exception("path prefix '%s' contains '%%'",pathPrefix);
    }
    interval_ = interval;

    time_t now = time(0);
    if ( file_.isOpened() ) {
        if ( prefix_ != pathPrefix || time_t(startTime_ + interval_) < now ) {
            doRollover(now,pathPrefix);
        }
    } else {
        char buf[128];
        makeFileSuffix(buf,sizeof(buf),now);
        std::string fileName( prefix_ );
        fileName.append(buf);
        file_.Append(fileName.c_str());
        startTime_ = now;
        crc32_ = 0;
        lines_ = 0;
    }
    prefix_ = pathPrefix;
}


void ProfileLog::logva( usectime_type now, int level,
                        const char* const format, va_list args ) throw()
{
    char buf[2048];

    const size_t prefixLength = formatPrefix(buf,sizeof(buf),now);

    // formatting of the message
    size_t bufsize = sizeof(buf) - prefixLength;
    char* buffer = buf;

    class BufferGuard {
    public:
        BufferGuard() : buf_(0) {}
        ~BufferGuard() { if (buf_) { delete [] buf_; } }
        void setBuf( char* buf ) {
            if (buf_) {
                delete [] buf_;
            }
            buf_ = buf;
        }
    private:
        char* buf_;
    } bufguard;

    try {

        while (true) {
    
            int n = ::vsnprintf( buffer + prefixLength, bufsize, format, args );
    
            if ( (n>-1) && size_t(n) < bufsize ) {
                bufsize = size_t(n) + prefixLength;
                break;
            }
            bufsize = ( n > -1 ) ?
                size_t(n + 1) :  // ISO/IEC 9899:1999
                bufsize*2;       // twice the old size
            bufguard.setBuf( buffer = new char[bufsize+prefixLength] );
            memcpy(buffer,buf,prefixLength);
    
        }

    } catch ( std::exception& e ) {
        fprintf(stderr,"out of memory");
        return;
    }

    // NOTE: here we have buffer and bufsize filled
    MutexGuard mg(lock_);
    if ( ! file_.isOpened() ) {
        fprintf(stderr,"logic problem in ProfileLog: file is not opened");
        std::terminate();
        return;
    }

    // file is guaranteed to be opened here
    file_.Write(buffer,bufsize);
    crc32_ = smsc::util::crc32(crc32_,buffer,bufsize);
    ++lines_;
}


time_t ProfileLog::tryToRoll( time_t now )
{
    MutexGuard mg(lock_);
    time_t rollTime = startTime_ + interval_;
    if ( rollTime < now ) {
        doRollover(now,prefix_.c_str());
        rollTime = startTime_ + interval_;
    }
    return rollTime;
}


void ProfileLog::doRollover( time_t now, const char* pathPrefix )
{
    assert( file_.isOpened() );

    TmpBuf<char,1024> buf;
    makeFileSuffix( buf.get(), buf.getSize(), now );

    std::string newfileName( pathPrefix );
    newfileName.append( buf.get() );

    File newf;
    newf.Append( newfileName.c_str() );

    finishFile( file_, crc32_, lines_, newfileName.c_str() );
    file_.Swap( newf );
    startTime_ = now;
    crc32_ = 0;
    lines_ = 0;
}

    
void ProfileLog::collectUnfinishedLogs( BadLogList& list )
{
    list.clear();

    std::string dirname(prefix_);
    std::string filename;
    {
        const size_t lastSlash = dirname.rfind('/');
        if (lastSlash != std::string::npos) {
            std::string(dirname,lastSlash+1).swap(filename);
            dirname.erase(lastSlash);
        } else {
            filename = dirname;
            dirname = ".";
        }
        filename.append( ::FILEFORMAT );
    }
    std::vector< std::string > entries;
    smsc::core::buffers::File::ReadDir( dirname.c_str(), entries,
                                        smsc::core::buffers::File::rdfFilesOnly|
                                        smsc::core::buffers::File::rdfNoDots );

    for ( std::vector<std::string>::const_iterator i = entries.begin();
          i != entries.end(); ++i ) {

        const size_t isize = i->size();
        if ( isize < 5 ) continue;

        // first of all check the extension
        if ( strcmp(i->c_str() + isize - STDEXTLEN, STDEXT) ) {
            // not equal
            continue;
        }

        // good extension is found
        ::tm ltm;
        memset(&ltm,0,sizeof(ltm));
        int pos = 0;
        sscanf( i->c_str(), filename.c_str(),
                &ltm.tm_year, &ltm.tm_mon, &ltm.tm_mday,
                &ltm.tm_hour, &ltm.tm_min, &ltm.tm_sec, &pos );
        if ( pos == 0 ) {
            // invalid name
            continue;
        }
        if ( size_t(pos) != isize ) {
            // not all chars read
            continue;
        }

        ltm.tm_year -= 1900;
        --ltm.tm_mon;
        ltm.tm_isdst = -1;
        time_t thetime = mktime(&ltm);
        list.push_back(thetime);
    }
    std::sort(list.begin(),list.end());
}


void ProfileLog::fixLogFile( time_t currentLogTime, time_t nextLogTime )
{
    uint32_t crc32 = 0;
    uint32_t lines = 0;

    std::string filename(prefix_);
    {
        char buf[50];
        makeFileSuffix(buf,sizeof(buf),currentLogTime);
        filename.append(buf);
    }
    smsc::core::buffers::File oldf;
    oldf.RWOpen(filename.c_str());
    // FIXME: count crc32 and lines

    std::string newname(prefix_);
    {
        size_t lastSlash = newname.rfind('/');
        if ( lastSlash != std::string::npos ) {
            newname.erase(0,lastSlash+1);
        }
        char buf[50];
        makeFileSuffix(buf,sizeof(buf),nextLogTime);
        newname.append(buf);
        newname.erase(newname.size()-STDEXTLEN);
        newname.append(finalSuffix_);
    }
    finishFile( oldf, crc32, lines, newname.c_str() );
}


void ProfileLog::makeFileSuffix( char* buf, size_t bufsize, time_t now ) throw()
{
    ::tm ltm;
    ::localtime_r(&now,&ltm);
    int pos = 0;
    ::snprintf( buf, bufsize, ::FILEFORMAT,
                ltm.tm_year+1900, ltm.tm_mon+1, ltm.tm_mday,
                ltm.tm_hour, ltm.tm_min, ltm.tm_sec, &pos );
}
 */


/*


    // new file has been opened
    {
        size_t lastSlash = newfileName.rfind('/');
        if ( lastSlash != std::string::npos ) {
            // remove directory part
            newfileName.erase(0,lastSlash+1);
        }
    }


    // change extension to .csv (and adding EOL)
    newfileName.erase(newfileName.size()-4);
    newfileName.append(".csv\n");

    ::snprintf( buf.get(), buf.getSize(), "STATS: CRC32=%08x LINES=%llu NEXTFILE: ",
                crc32_, lines_ );
    buf.Append(newfileName.c_str());
    file_.Write(buf.get(),buf.getSize());
    file_.Swap(newf);
    lines_ = 0;
    crc32_ = 0;

    return;

    }

    // if file is not opened
    //  1. look for unfinished file in prefix
    //  2. if found, open it
    //  3. else create one with current time.

    std::string dirname(pathPrefix);
    std::string filename;
    {
        const size_t lastSlash = dirname.rfind('/');
        if (lastSlash != std::string::npos) {
            std::string(dirname,lastSlash+1).swap(filename);
            dirname.erase(lastSlash);
        } else {
            filename = dirname;
            dirname = ".";
        }
        filename.append( FILEFORMAT );
    }

    std::vector< std::string > entries;
    smsc::core::buffers::File::ReadDir(dirname.c_str(),entries,rdfFilesOnly|rdfNoDots);
    time_t timeNow;
    time_t roundTimeNow;
    std::string*  goodName = 0;
    for ( std::vector<std::string>::const_iterator i = entries.begin();
          i != entries.end(); ++i ) {
        const size_t isize = i->size();
        if ( isize < 10 ) continue;
        // first of all check the extension
        if ( strcmp(i->c_str() + isize - 4,".log") ) {
            // not equal
            continue;
        }
        // good extension is found
        ::tm ltm;
        memset(&ltm,0,sizeof(ltm));
        int pos = 0;
        sscanf( i->c_str(), filename, &ltm.tm_year, &ltm.tm_mon, &ltm.tm_mday,
                &ltm.tm_hour, &ltm.tm_min, &ltm.tm_sec, &pos );
        if ( pos == 0 ) {
            // invalid name
            continue;
        }
        ltm.tm_year -= 1900;
        --ltm.tm_mon;
        ltm.tm_isdst = -1;
        time_t thetime = mktime(&ltm);
        if ( goodName ) {
            // the second occurrence
            if ( thetime < timeNow ) {
                continue;
            }
        }
        goodName = &*i;
        timeNow = thetime;
        roundTimeNow = roundTime(thetime,ltm);
    }
    
    dirname.push_back('/');
    if ( goodName ) {
        // the file is found, trying to open
        dirname.append(*goodName);
    } else {
        timeNow = time_t(now / 1000000);
        :tm ltm;
        ::localtime_r(&timeNow,&ltm);
        roundTimeNow = roundTime(timeNow,ltm);
        dirname = pathPrefix;
        char buf[64];
        ::snprintf( buf, sizeof(buf), FILEFORMAT,
                    ltm.tm_year+1900, ltm.tm_mon+1, ltm.tm_mday,
                    ltm.tm_hour, ltm.tm_min, ltm.tm_sec, &pos );
        dirname.append(buf);
    }
 
    file_.Append(dirname.c_str());
    nextRollTime_ = usectime_type(roundTimeNow + interval_) * 1000000U;
    crc32_ = smsc::util::crc32( crc32_, ... );
}
 */


// ========================================================================

ProfileLogRoller::ProfileLogRoller() :
    log_(smsc::logger::Logger::getInstance("logroller")),
    logMutex_(),
    mon_(),
    isStarted_(false),
    isStopping_(false),
    configReloadInterval_(60)
{
}


ProfileLogRoller::~ProfileLogRoller()
{
    // destroy all loggers
    stop();
    {
        MutexGuard mg(logMutex_);
        char* name;
        ProfileLog* logger;
        for ( Loggers::Iterator it(&loggers_); it.Next(name,logger); ) {
            delete logger;
        }
        loggers_.Empty();
    }
    MutexGuard mg(mon_);
    logStreams_.Empty();
}


ProfileLog* ProfileLogRoller::getLogger( const char* name )
{
    MutexGuard mg(logMutex_);
    ProfileLog** ptr = loggers_.GetPtr(name);
    if ( !ptr || !*ptr ) {
        throw smsc::util::Exception("logger '%s' is not configured",name);
    }
    return *ptr;
}


void ProfileLogRoller::start()
{
    {
        MutexGuard mg(mon_);
        if ( isStarted_ ) {
            return;
        }
        isStopping_ = false;
        isStarted_ = true;
    }
    try {
        readConfiguration();
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"log roller reading config, exc: %s",e.what());
        isStarted_ = false;
        throw;
    }
    Start();
}


void ProfileLogRoller::stop()
{
    {
        MutexGuard mg(mon_);
        if ( !isStarted_ ) { return; }
        isStopping_ = true;
        mon_.notify();
    }
    WaitFor();
}


int ProfileLogRoller::Execute()
{
    bool needPostfix = true;
    try {

        time_t nextRollTry = time(0);
        time_t nextConfigTry = nextRollTry + configReloadInterval_;
        const int sleepTime = 60;
        while ( ! isStopping_ ) {

            if ( needPostfix ) {
                // fix all unfinished streams
                char* streamName;
                EmbedRefPtr<ProfileLogStream> stream;
                for ( LogStreams::Iterator it(&logStreams_);
                      it.Next(streamName,stream); ) {
                    
                    try {
                        if ( stream->needPostfix_ ) {
                            stream->postInitFix( isStopping_ );
                        }
                    } catch ( std::exception& e ) {
                        smsc_log_error(log_,"postInitFix('%s') exc: %s",streamName,e.what());
                    }
                    if ( isStopping_ ) { break; }
                }
                needPostfix = false;
            }

            time_t now = time(0);
            time_t nextTry = std::min(nextRollTry, nextConfigTry);
            if ( now < nextTry ) {
                // not reached yet
                int sleep = int(nextTry - now);
                if ( sleep > sleepTime ) {
                    sleep = sleepTime;
                }
                MutexGuard mg(mon_);
                mon_.wait(sleep*1000);
                continue;
            }

            if ( now >= nextConfigTry ) {
                nextConfigTry = now + configReloadInterval_;
                try {
                    needPostfix = readConfiguration();
                    if (needPostfix) { nextRollTry = now; }
                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"logroller config exc: %s",e.what());
                }
            }

            if ( now < nextRollTry ) { continue; }

            // we have reached the rolling time
            now = time(0);
            nextRollTry = now + 10000000;
            char* streamName;
            EmbedRefPtr<ProfileLogStream> stream;
            for ( LogStreams::Iterator it(&logStreams_); it.Next(streamName,stream); ) {
                time_t next = stream->tryToRoll( now );
                if ( next < nextRollTry ) {
                    nextRollTry = next;
                }
            }

        }

    } catch ( std::exception& e ) {
        smsc_log_error( log_, "log roller exc: %s", e.what() );
    }
    MutexGuard mg(mon_);
    isStarted_ = false;
    isStopping_ = true;
    return 0;
}


void ProfileLogRoller::addLogStream( EmbedRefPtr<ProfileLogStream>& pls )
{
    if (!pls) return;
    EmbedRefPtr<ProfileLogStream> found;
    {
        MutexGuard mg(mon_);
        EmbedRefPtr<ProfileLogStream>* ptr = logStreams_.GetPtr(pls->getName());
        if (!ptr) {
            logStreams_.Insert(pls->getName(),pls);
        } else {
            found = *ptr;
        }
    }
    if ( !found ) {
        pls->init();
    } else {
        found->update(*pls);
    }
}


bool ProfileLogRoller::getLogStream( const char* name,
                                     EmbedRefPtr<ProfileLogStream>& pls )
{
    if (!name || !name[0]) return false;
    MutexGuard mg(mon_);
    EmbedRefPtr<ProfileLogStream>* ptr = logStreams_.GetPtr(name);
    if (!ptr) return false;
    pls = *ptr;
    return true;
}


ProfileLog* ProfileLogRoller::addLogger( const char* name,
                                         smsc::logger::Logger::LogLevel level,
                                         eyeline::informer::EmbedRefPtr<ProfileLogStream>& pls,
                                         bool useLog )
{
    if (!name || name[0] == '\0' ) { return 0; }
    ProfileLog* res;
    {
        MutexGuard mg(logMutex_);
        ProfileLog** ptr = loggers_.GetPtr(name);
        if (!ptr) {
            ptr = loggers_.SetItem( name, new ProfileLog(name) );
        } else if ( !*ptr ) {
            *ptr = new ProfileLog(name);
        }
        res = *ptr;
    }
    if ( res ) {
        res->init(level,pls.get(),useLog);
    }
    return res;
}


/*
void ProfileLogRoller::readConfiguration()
{
    using namespace smsc::util::config;

    std::auto_ptr<Config> cfg1;

    time_t now = time(0);
    {
        MutexGuard mg(mon_);
        do {
            if ( lastConfigTime_ == 0 ) break;
            // consecutive (not the first) attempt
            if ( configReloadInterval_ == 0 ) return;
            if ( time_t(lastConfigTime_+configReloadInterval_) > now ) return;
        } while (false);
        lastConfigTime_ = now;
    }

    try {
        cfg1.reset( Config::createFromFile("dataexport.xml",&oldmtime_) );
        if ( !cfg1.get() ) {
            // config has not been changed
            return;
        }
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"config exc: %s",e.what());
        throw;
    }

    // reading config
    //
    // <config>
    //    <section name="streams">
    //        <section name="null">
    //            <param name="type" type="string">
    //        </section>
    //        <section name="pvss.a">
    //            <param name="type" type="string">RollingFileStream</param>
    //            <param name="pathPrefix" type="string">logs/a.log</param>
    //        </section>
    //    </section>
    //    <section name="categories">
    //       <section name="pvss.abnt.a">
    //           <param name="stream" type="string">pvss.a</param>
    //           <param name="uselog" type="bool">true</param> <!-- opt -->
    //           <param name="level" type="string">debug</param>
    //       </section>
    //    </section>
    // </config>
    //

    eyeline::informer::ConfigWrapper cwrap1(*cfg1,log_);
    try {

        // 1. reading common params
        const unsigned reloadInterval = cwrap1.getInt("configReloadInterval",60,0,10000);
        if ( reloadInterval > 0 && reloadInterval < 30 ) {
            throw ConfigException("configReloadInterval is too small");
        }

        // 2. reading streams
        std::auto_ptr<CStrSet> streamNames(cfg1->getChildSectionNames("streams"));
        for ( CStrSet::const_iterator i = streamNames->begin();
              i != streamNames->end(); ++i ) {

            // processing the stream *i
            EmbedRefPtr<ProfileLogStream> pls;
            std::auto_ptr<Config> cfg2(cfg1->getSubConfig(("streams." + *i).c_str(),true));
            eyeline::informer::ConfigWrapper cwrap2(*cfg2,log_);
            const std::string streamType = cwrap2.getString("type","RollingFileStream");
            if ( streamType == "RollingFileStream" ) {
                pls.reset( new RollingFileStream(i->c_str()) );
            } else {
                throw smsc::util::config::ConfigException("wrong type '%s' of stream '%s'",
                                                          streamType.c_str(),
                                                          i->c_str() );
            }
            addLogStream(pls);

        }

        // 3. reading loggers
        std::auto_ptr<CStrSet> loggerNames(cfg1->getChildSectionNames("categories"));
        for ( CStrSet::const_iterator i = loggerNames->begin();
              i != loggerNames->end(); ++i ) {
            std::auto_ptr<Config> cfg3(cfg1->getSubConfig(("categories." + *i).c_str(),true));
            eyeline::informer::ConfigWrapper cwrap3(*cfg3,log_);
            const std::string streamName = cwrap3.getString("stream");
            const std::string strlevel = cwrap3.getString("level");
            smsc::logger::Logger::LogLevel level;
            if ( strlevel == "debug" ) {
                level = smsc::logger::Logger::LEVEL_DEBUG;
            } else if ( strlevel == "info" ) {
                level = smsc::logger::Logger::LEVEL_INFO;
            } else if ( strlevel == "warn" ) {
                level = smsc::logger::Logger::LEVEL_WARN;
            } else if ( strlevel == "error" ) {
                level = smsc::logger::Logger::LEVEL_ERROR;
            } else {
                throw smsc::util::config::ConfigException("category '%s' has unknown level '%s'",
                                                          i->c_str(), strlevel.c_str() );
            }
            const bool useLog = cwrap3.getBool("uselog",false);
            EmbedRefPtr<ProfileLogStream> pls;
            {
                MutexGuard mg(mon_);
                EmbedRefPtr<ProfileLogStream>* ptr = logStreams_.GetPtr(streamName.c_str());
                if ( !ptr || !*ptr ) {
                    throw smsc::util::config::ConfigException("category '%s' refers to unknown stream '%s'",
                                                              i->c_str(), streamName.c_str() );
                }
                pls = *ptr;
            }

            ProfileLog* ptr = addLogger( i->c_str() );
            if (ptr) { ptr->init(level, pls, useLog); }
        }

    } catch ( std::exception& e ) {
        smsc_log_error(log_,"config exc: %s", e.what());
        throw;
    }
}
 */

}
}
