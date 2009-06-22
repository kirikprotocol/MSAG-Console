#ifndef SCAG_BILL_EWALLET_PROTO_SOCKET_H
#define SCAG_BILL_EWALLET_PROTO_SOCKET_H

#include <cassert>
#include <memory>
#include "Context.h"
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
class Socket
{
private:
    class WriteContext
    {
    public:
        WriteContext( util::msectime_type ct ) : creationTime(ct), seqNum_(0), context_(0) {}
        ~WriteContext() {
            assert(!context_);
        }
        void setContext( Context* ctx ) {
            assert(!context_);
            context_ = ctx;
            if ( context_ ) { seqNum_ = context_->getSeqNum(); }
        }
        uint32_t getSeqNum() const {
            return seqNum_;
        }
        Context* popContext() {
            Context* c = context_;
            context_ = 0;
            return c;
        }
        Context* getContext() {
            return context_;
        }

    public:
        util::msectime_type creationTime;
        Streamer::Buffer    buffer;
    private:
        uint32_t            seqNum_;
        Context*            context_; // owned
    };

public:
    static Socket* fromSocket( smsc::core::network::Socket& sock );

public:
    Socket( Core& core, util::msectime_type activity = 0 );

    inline bool isConnected() const { return sock_->isConnected(); }

    /// send packet to a sending queue and notify writer.
    /// NOTE: invoked from core.
    /// NOTE: if exception is thrown, context fields are not touched.
    void send( std::auto_ptr< Context >& context, bool sendRequest ); // throw

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

    /// attach/detach a socket: refcounting
    void attach( const char* who );
    void detach( const char* who );

    /// this one is used to make sure that all other threads has detached from socket
    unsigned attachCount();

    /// is used to set/unset writer which this socket should notify
    void setWriter( SocketWriter* writer );

    /// connect
    void connect(); // throw

    /// close the socket
    void close() {
        sock_->Close();
    }

    util::msectime_type getLastActivity() const {
        MutexGuard mg(activityMutex_);
        return lastActivity_; 
    }
    void updateActivity( util::msectime_type la = 0 ) {
        if (!la) la = util::currentTimeMillis();
        MutexGuard mg(activityMutex_);
        lastActivity_ = la;
    }

protected:
    ~Socket();

private:
    void init();
    void reportPacket( int state );

private:
    static smsc::logger::Logger*                 log_;

private:
    Core&                                        core_;
    std::auto_ptr< smsc::core::network::Socket > sock_;

    smsc::core::synchronization::Mutex           refMutex_;
    size_t                                       refCount_;

    smsc::core::synchronization::Mutex                 queueMutex_;
    smsc::core::buffers::CyclicQueue< WriteContext* >* queue_;

    std::auto_ptr< WriteContext >                wrContext_;
    Streamer::Buffer                             wrBuffer_;
    
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
