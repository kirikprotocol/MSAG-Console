#ifndef _SCAG_PVSS_CLIENT_PVSSSTREAMCLIENT_H
#define _SCAG_PVSS_CLIENT_PVSSSTREAMCLIENT_H

#include "core/buffers/XHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "scag/pvss/base/PersCall.h"
#include "scag/pvss/base/PersClient.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssReader;
class PvssWriter;
class PvssConnector;
class PvssConnection;

class PvssStreamClient : public PersClient
{
private:
    struct PersCallPtrHFunc
    {
    public:
        static unsigned int CalcHash( const scag2::pvss::PersCall* key )
        {
            return unsigned(reinterpret_cast<int64_t>
                            (reinterpret_cast<const void*>(key)));
        }
    };

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
               unsigned connPerThread,
               bool     async = true );

    // check for a call in queue
    void      waitForCalls( int msec );
    PersCall* getCall();
    PersCall* createPingCall();
    PersCall* createAuthCall();

    /// increment connection count
    void connected( PvssConnection& conn );
    
    /// decrement connection count, if count == 0 then finish all current queued calls
    void disconnected( PvssConnection& conn );

    /// check connections.
    /// This method is invoked from connector task after it tries to connect.
    /// So, if all connections failed, the client should empty the call queue.
    void checkConnections();

private:
    // virtual void configChanged();
    void finishCalls( PersCall* call, bool drop );
    virtual bool callAsync( PersCall* call, PersCallInitiator& fromwho );
    virtual bool callSync( PersCall* call );
    virtual int getClientStatus();

public:
    std::string host;
    int         port;
    int         timeout;
    int         pingTimeout;             // seconds
    int         reconnectTimeout;        // seconds
    bool        async;                   // async regime

private:
    unsigned maxCallsCount_;
    unsigned clients_;
    unsigned connPerThread_;

    bool isStopping;
    smsc::logger::Logger* log_;

    smsc::core::synchronization::EventMonitor queueMonitor_;
    PersCall*                       headContext_;
    PersCall*                       tailContext_;
    unsigned                        callsCount_;
    smsc::core::threads::ThreadPool tp_;
    
    smsc::core::buffers::Array< PvssReader* >     readers_;
    smsc::core::buffers::Array< PvssWriter* >     writers_;
    PvssConnector*                                connector_;
    smsc::core::buffers::Array< PvssConnection* > connections_; // owned

    smsc::core::synchronization::EventMonitor  connMonitor_;
    smsc::core::buffers::Array< PvssConnection* > connected_;   // not owned
};

}
}
}

#endif /* !_SCAG_PVSS_CLIENT_PVSSSTREAMCLIENT_H */
