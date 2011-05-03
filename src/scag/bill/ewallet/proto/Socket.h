#ifndef SCAG_BILL_EWALLET_PROTO_SOCKET_H
#define SCAG_BILL_EWALLET_PROTO_SOCKET_H

#include <cassert>
#include <memory>
#include "Context.h"
#include "SocketBase.h"
#include "scag/bill/ewallet/Streamer.h"
#include "core/network/Socket.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/Mutex.hpp"
#include "scag/util/Time.h"
#include "logger/Logger.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

class Core;
class SocketWriter;

/// socket to connect to ewallet
class Socket : public SocketBase
{
public:
    static Socket* fromSocket( smsc::core::network::Socket& sock );

public:
    Socket( Core& core, util::msectime_type activity = 0 );

    virtual Socket* castToSocket() { return this; }

    virtual bool isConnected() const { return sock_->isConnected(); }

    /// send packet to a sending queue and notify writer.
    /// NOTE: invoked from core.
    /// NOTE: if exception is thrown, context fields are not touched.
    virtual void send( std::auto_ptr< Context >& context, bool sendRequest ); // throw

    /// check that socket wants to send some data.
    /// invoked from writer.
    /// NOTE: not thread-safe
    bool wantToSend( util::msectime_type currentTime );

    /// send data and notify core when a packet is sent.
    void sendData();

    /// invoked from reader when there in some data on input.
    /// NOTE: not thread-safe.
    void processInput();

    smsc::core::network::Socket* socket() { return sock_.get(); }

    /// is used to set/unset writer which this socket should notify
    void setWriter( SocketWriter* writer );

    /// connect
    void connect(); // throw

    /// close the socket
    virtual void close() {
        sock_->Close();
    }

    util::msectime_type getLastActivity() const {
        smsc::core::synchronization::MutexGuard mg(activityMutex_);
        return lastActivity_; 
    }
    void updateActivity( util::msectime_type la = 0 ) {
        if (!la) la = util::currentTimeMillis();
        smsc::core::synchronization::MutexGuard mg(activityMutex_);
        lastActivity_ = la;
    }

protected:
    virtual ~Socket();

private:
    void init();
    void reportPacket( int state );

private:
    std::auto_ptr< smsc::core::network::Socket > sock_;

    std::auto_ptr< WriteContext >                wrContext_; // accessed via queueMon_
    Streamer::Buffer                             wrBuffer_;
    // we use external size as TmpBuf has a feature with size=0
    size_t                                       wrBufferSize_;
    
    unsigned                                     rdBuflen_;
    Streamer::Buffer                             rdBuffer_;

    smsc::core::synchronization::Mutex           writerMutex_;
    SocketWriter*                                writer_;

    mutable smsc::core::synchronization::Mutex   activityMutex_;
    util::msectime_type                          lastActivity_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_SOCKET_H */
