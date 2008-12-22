#ifndef _SCAG_PVSS_CLIENT_PVSSSTREAMCLIENT_H
#define _SCAG_PVSS_CLIENT_PVSSSTREAMCLIENT_H

#include <unistd.h>
#include "core/buffers/IntHash.hpp"
#include "core/buffers/XHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "scag/pvss/base/PersCall.h"
#include "scag/pvss/base/PersClient.h"

namespace {
struct PersCallPtrHFunc
{
public:
    static unsigned int CalcHash( const scag2::pvss::PersCall* key )
    {
        return unsigned(reinterpret_cast<int64_t>
                        (reinterpret_cast<const void*>(key)));
    }
};
}

namespace scag2 {
namespace pvss {

class PersClientTask;

class PvssStreamClient : public PersClient, public PersCallInitiator
{
public:
    PvssStreamClient();
    virtual ~PvssStreamClient();
    virtual void Stop();
    void init( const char *_host,
               int _port,
               int _timeout,
               int _pingTimeout,
               int _reconnectTimeout,
               unsigned _maxCallsCount,
               unsigned clients,
               bool     async );

    PersCall* getCall( int tmomsec, PersClientTask& task );

    /// increment connection count
    void incConnect();

    /// decrement connection count, if count == 0 then finish all current queued calls
    void decConnect();

    void wait( int msec ) {
        MutexGuard mg(queueMonitor_);
        queueMonitor_.wait( msec );
    }

private:
    // virtual void configChanged();
    void finishCalls( PersCall* call, bool drop );
    virtual bool callAsync( PersCall* call, PersCallInitiator& fromwho );
    virtual bool callSync( PersCall* call );
    virtual void continuePersCall( PersCall* call, bool drop );
    virtual int getClientStatus();

public:
    std::string host;
    int         port;
    int         timeout;
    int         pingTimeout;
    int         reconnectTimeout;

private:
    unsigned maxCallsCount_;
    unsigned clients_;
    bool     async_;

    bool isStopping;
    smsc::logger::Logger* log_;


    struct Pipe {
        Pipe() { ::pipe(fd); }
        inline void notify() const { ::write(fd[1], "0", 1); }
        inline int rpipe() const { return fd[0];}
        inline void close() { ::close(fd[0]); ::close(fd[1]); }
        bool operator == ( const Pipe& p ) const {
            return (fd[0] == p.fd[0]) && (fd[1] == p.fd[1]);
        }
    private:
        int fd[2];
    };

    EventMonitor              queueMonitor_;
    PersCall*                 headContext_;
    PersCall*                 tailContext_;
    unsigned                  callsCount_;
    unsigned                  connected_;
    std::list<Pipe>           waitingPipes_;   // a list of waiting pipes
    std::list<Pipe>           freePipes_;      // a list of free pipes (cached)
    unsigned                  waitingStreams_; // a number of streams waiting w/o pipe
    smsc::core::threads::ThreadPool tp_;
    
    // for sync calls
    EventMonitor                                     syncMonitor_;
    smsc::core::buffers::XHash< PersCall*, uint8_t, PersCallPtrHFunc > syncRequests_;
};

}
}

#endif /* !_SCAG_PVSS_CLIENT_PVSSSTREAMCLIENT_H */
