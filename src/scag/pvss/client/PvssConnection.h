#ifndef _SCAG_PVSS_CLIENT_PVSSCONNECTION_H
#define _SCAG_PVSS_CLIENT_PVSSCONNECTION_H

#include <list>
#include "util/int.h"
#include "scag/pvss/base/PersCall.h"
#include "core/buffers/IntHash.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssStreamClient;

/// a connection to a pvss
class PvssConnection
{
private:
    /// current time (msec) counting from some reference point.
    typedef int msectime_type;

    /// a request sent to a server
    struct Call 
    {
        Call( int32_t s, PersCall* c, msectime_type t) :
        seqnum(s), ctx(c), stamp(t) {}
        int32_t                   seqnum;
        PersCall*                 ctx;
        msectime_type             stamp;
    };

    typedef std::list< Call > Callqueue;
    typedef smsc::core::buffers::IntHash< Callqueue::iterator > Callhash;
    
public:

    static PvssConnection* fromSocket( smsc::core::network::Socket* s ) {
        return (PvssConnection*) s->getData(0);
    }

    PvssConnection( PvssStreamClient& pers );

    // these methods are invoked
    // in synchronous manner
    // from PvssReader/PvssWriter respectively.
    void processInput( bool hasSeqnum = true );
    void sendData();
    bool wantToSend();
    inline bool isConnected() { return connected_; }
    inline smsc::core::network::Socket* socket() { return &sock_; }

    void connect();
    void disconnect();
    void dropCalls();
    void dropExpiredCalls();

protected:
    void prepareWrBuffer( PersCall* ctx );

    // registry access
    msectime_type msectime() const;
    void addCall( int32_t seqnum, PersCall* ctx );
    PersCall* getCall( int32_t seqnum );

private:
    smsc::logger::Logger*       log_;
    smsc::logger::Logger*       logd_;
    smsc::core::network::Socket sock_;
    bool                        connected_;
    time_t                      lastActivity_;
    time_t                      lastConnect_;
    PvssStreamClient*           pers_;
    struct timeval              time0_;

    int32_t                     seqnum_;

    // reading
    int32_t                     rdToRead;
    SerialBuffer                rdBuffer;

    // writing
    int32_t                     wrBufSent;
    SerialBuffer                wrBuffer;

    // registry
    smsc::core::synchronization::Mutex regmtx_;
    Callqueue                          callqueue_;
    Callhash                           callhash_;
};

} // namespace client
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CLIENT_PVSSCONNECTION_H */
