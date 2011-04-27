#ifndef PVSS_PROFILELOG_H
#define PVSS_PROFILELOG_H

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/File.hpp"
#include "core/buffers/Hash.hpp"
#include "informer/io/EmbedRefPtr.h"

namespace eyeline {
namespace informer {
template <class T> class EmbedRefPtr;
}
}

namespace scag2 {
namespace pvss {

class ProfileLogRoller;

class ProfileLogStream
{
    friend class ProfileLogRoller;
    friend class eyeline::informer::EmbedRefPtr< ProfileLogStream >;

protected:
    ProfileLogStream( const char* name );

public:
    virtual ~ProfileLogStream() {}
    
    inline const char* getName() const throw() { return name_.c_str(); }

    virtual size_t formatPrefix( char* buf, size_t bufsize ) const throw() = 0;
    virtual void write( const char* buf, size_t bufsize ) = 0;

protected:
    /// initialization, opening all necessary files, etc.
    virtual void init() = 0;

    /// update of configuration based on the other instance.
    virtual void update( ProfileLogStream& ps ) = 0;

    virtual void postInitFix( bool& isStopping ) = 0;
    virtual time_t tryToRoll( time_t now ) = 0;

private:
    inline void ref() throw() {
        smsc::core::synchronization::MutexGuard mg(reflock_);
        ++ref_;
    }
    inline void unref() throw() {
        {
            smsc::core::synchronization::MutexGuard mg(reflock_);
            if (ref_>1) {
                --ref_;
                return;
            }
        }
        delete this;
    }

private:
    smsc::core::synchronization::Mutex reflock_;
    unsigned                           ref_;
protected:
    smsc::core::synchronization::Mutex lock_;
    std::string                        name_;
    bool                               needPostfix_;
};


/// concrete category
class ProfileLog
{
public:
    ProfileLog( const char* name );
    ~ProfileLog();

    // mimic smsc::logger::Logger
    inline bool isInfoEnabled() const throw() {
        return level_ >= int(smsc::logger::Logger::LEVEL_INFO);
    }
    inline bool isLogLevelEnabled( int lev ) const throw() {
        return level_ >= lev;
    }

    inline void log_( int level, const char* const format, ... ) throw()
    {
        if ( isLogLevelEnabled(level) && stream_.get() ) { /* fast check w/o locking */
            va_list args;
            va_start(args,format);
            logva(level,format,args);
            va_end(args);
        }
    }
    inline void log( int level, const char* const format, ... ) throw()
    {
        if ( isLogLevelEnabled(level) && stream_.get() ) {
            va_list args;
            va_start(args,format);
            logva(level,format,args);
            va_end(args);
        }
    }

    inline const char* getName() const throw() { return name_.c_str(); }

    // to be invoked only from ProfileLogRoller
    void init( smsc::logger::Logger::LogLevel level,
               ProfileLogStream*              stream,
               bool                           useLog );

protected:
    void logva( int level, const char* const format, va_list args ) throw();

protected:
    std::string                                      name_;
    int                                              level_;
    bool                                             uselog_;
    eyeline::informer::EmbedRefPtr<ProfileLogStream> stream_;
    eyeline::informer::EmbedRefPtr<ProfileLogStream> toset_;
};





class ProfileLogRoller : protected smsc::core::threads::Thread
{
public:
    ProfileLogRoller();
    virtual ~ProfileLogRoller();

    ProfileLog* getLogger( const char* logger );

    void start();
    void stop();

protected:
    // 0. read configuration
    // 1. first of all, fix all unfinished logs
    // 2. enter standard loop:
    //   a. wait
    //   b. check config, if changed roll all files
    //   c. otherwise, if rolltime achieved, then roll
    virtual int Execute();

    // helpers for readConfiguration
    void addLogStream( eyeline::informer::EmbedRefPtr<ProfileLogStream>& pls );
    bool getLogStream( const char* name,
                       eyeline::informer::EmbedRefPtr<ProfileLogStream>& pls );
    ProfileLog* addLogger( const char* name,
                           smsc::logger::Logger::LogLevel level,
                           eyeline::informer::EmbedRefPtr<ProfileLogStream>& stream,
                           bool useLog );

    // read the configuration and fill the core using addLogStream, addLogger
    // @return true if the configuration was updated.
    virtual bool readConfiguration() = 0;

    typedef smsc::core::buffers::Hash< ProfileLog* >  Loggers;
    typedef smsc::core::buffers::Hash< eyeline::informer::EmbedRefPtr<ProfileLogStream> > LogStreams;

protected:
    smsc::logger::Logger*               log_;

    smsc::core::synchronization::Mutex  logMutex_;
    Loggers                             loggers_;

    smsc::core::synchronization::EventMonitor mon_;
    LogStreams                          logStreams_;
    bool                                isStarted_;
    bool                                isStopping_;

    // a period of config reload, 0 - only once
    unsigned                            configReloadInterval_;
};

}
}

#endif
