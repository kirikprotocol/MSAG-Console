#ifndef _SCAG_PVSS_CORE_PVSSSOCKET_H
#define _SCAG_PVSS_CORE_PVSSSOCKET_H

#include <memory>
#include <list>
#include "core/network/Socket.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/exc/IOException.h"
#include "scag/pvss/api/packets/Protocol.h"
#include "Context.h"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {
namespace core {

class Core;
class PacketWriter;
class PacketReader;

class PvssSocket
{
private:
    static smsc::logger::Logger*                      log_;

public:
    static PvssSocket* fromSocket( smsc::core::network::Socket* s ) {
        return static_cast<PvssSocket*>(s->getData(0));
    }

public:
    struct WriteContext 
    {
        WriteContext(uint32_t seqnum,util::msectime_type cTime) : seqNum(seqnum), creationTime(cTime) {}
        uint32_t                 seqNum;
        util::msectime_type      creationTime;
        Protocol::Buffer         buffer;
    };

public:
    /// ctor for client-side
    PvssSocket( const std::string& host,
                short port,
                int   connectionTimeout );

    /// ctor for server-side
    PvssSocket( smsc::core::network::Socket* socket );

    ~PvssSocket();

    /// Accept packet (request/response) to a sending queue
    /// and notify attached writer (in case queue was empty).
    /// NOTE: if exception is thrown ctx is not released.
    void send( const Packet* ctx, bool isRequest, bool force ) /* throw (PvssException) */ ;

    /// checks if this socket want to send data
    /// NOTE: not a thread-safe
    bool wantToSend();

    /// send data and report to core when context has been sent/expired/failed.
    /// NOTE: call wantToSend before.
    void sendData( Core& core );

    /// checks if socket is connected
    bool isConnected() const { return sock_->isConnected(); }

    /// set the next connect attempt time
    void setConnectTime( util::msectime_type connectTime ) {
        connectTime_ = connectTime;
    }

    /// get the next connect time
    util::msectime_type getConnectTime() const {
        return connectTime_;
    }

    /// return underlying socket (for multiplexing)
    inline smsc::core::network::Socket* socket() { return sock_.get(); }

    void processInput( Core& core );

    void connect() throw (exceptions::IOException);
    void disconnect();

    void registerWriter( PacketWriter* writer );
    void registerReader( PacketReader* reader );
    bool isInUse() const;

private:
    void init(); // called from ctor

    PvssSocket( const PvssSocket& );
    PvssSocket& operator = ( const PvssSocket& );

private:
    std::auto_ptr<smsc::core::network::Socket> sock_;
    std::string host_;
    short       port_;
    int         connectionTmo_;   // sec

    mutable smsc::core::synchronization::EventMonitor pendingContextMon_;
    smsc::core::buffers::CyclicQueue< WriteContext* > pendingContexts_;

    // those are for write methods
    // NOTE: they are not guarded by any mutexes
    std::auto_ptr<WriteContext>               wrContext_; // current write context
    Protocol::Buffer                          wrBuffer_;

    unsigned                                  rdBuflen_;
    Protocol::Buffer                          rdBuffer_;

    // writer/reader
    smsc::core::synchronization::Mutex        writerMutex_;
    PacketWriter*                             writer_;
    PacketReader*                             reader_;

    util::msectime_type                       connectTime_;
    util::msectime_type                       lastActivity_;
};

} // namespace core
} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {
namespace core {

using scag2::pvss::core::PvssSocket;

}
}
}

#endif /* !_SCAG_PVSS_CORE_PVSSSOCKET_H */
