#ifndef _SCAG_PVSS_CLIENT_PVSSCONNECTION_H
#define _SCAG_PVSS_CLIENT_PVSSCONNECTION_H

#include <list>
#include "util/int.h"
#include "scag/pvss/base/PersCall.h"
#include "core/buffers/IntHash.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"
#include "scag/util/MsecTime.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssStreamClient;

/// a connection to a pvss
class PvssConnection
{
public:
    /// current time (msec) counting from some reference point.
    typedef util::MsecTime::time_type msectime_type;

private:
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
    /// only for sync mode: connection is in reading state
    bool isReading() const;

    inline bool isConnected() { return connected_; }
    inline smsc::core::network::Socket* socket() { return &sock_; }

    void connect();
    void disconnect();
    void dropCalls();
    void dropExpiredCalls();

    inline msectime_type msectime() const { return time0_.msectime(); }

protected:
    void prepareWrBuffer( PersCall* ctx );

    // registry access
    void addCall( int32_t seqnum, PersCall* ctx );
    PersCall* getCall( int32_t seqnum );

    void setReading( bool state );

private:
    smsc::logger::Logger*       log_;
    smsc::logger::Logger*       logd_;
    smsc::core::network::Socket sock_;
    bool                        connected_;
    time_t                      lastActivity_;
    time_t                      lastConnect_;
    PvssStreamClient*           pers_;
    util::MsecTime              time0_;

    int32_t                     seqnum_;

    // reading
    int32_t                     rdToRead;
    SerialBuffer                rdBuffer;

    // writing
    int32_t                     wrBufSent;
    SerialBuffer                wrBuffer;

    // registry
    mutable smsc::core::synchronization::Mutex regmtx_;
    Callqueue                          callqueue_;
    Callhash                           callhash_;

    bool                               isReading_; // for sync mode
};

} // namespace client
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CLIENT_PVSSCONNECTION_H */
