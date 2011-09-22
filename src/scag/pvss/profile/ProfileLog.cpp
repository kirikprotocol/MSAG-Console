#include <cassert>
#include <string.h>
#include "ProfileLog.h"
// #include "RollingFileStream.h"
#include "util/Exception.hpp"
#include "informer/io/InfosmeException.h"

using eyeline::informer::EmbedRefPtr;
using smsc::core::synchronization::MutexGuard;

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
    if (level < level_) return;
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
        if ( ! configReloadInterval_ ) {
            nextConfigTry = nextRollTry + 3600*24*365*10;
        }
        const int sleepTime = 60;
        while ( ! isStopping_ ) {

            time_t now = time(0);
            smsc_log_debug(log_,"pass at %llu",uint64_t(now));

            if ( needPostfix ) {
                // fix all unfinished streams
                smsc_log_debug(log_,"need postfix");
                needPostfix = false;
                char* streamName;
                EmbedRefPtr<ProfileLogStream> stream;
                MutexGuard mg(mon_);
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
                continue;
            }

            time_t nextTry = std::min(nextRollTry, nextConfigTry);
            if ( now < nextTry ) {
                // not reached yet
                int sleep = int(nextTry - now);
                if ( sleep > sleepTime ) {
                    sleep = sleepTime;
                }
                smsc_log_debug(log_,"sleep=%d",sleep);
                MutexGuard mg(mon_);
                mon_.wait(sleep*1000);
                continue;
            }

            if ( configReloadInterval_ && now >= nextConfigTry ) {
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
            MutexGuard mg(mon_);
            for ( LogStreams::Iterator it(&logStreams_); it.Next(streamName,stream); ) {
                time_t next = stream->tryToRoll( now );
                smsc_log_debug(log_,"rolling at %ld, next=%ld",long(now),long(next));
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

}
}
