#ifndef SCAG_BILL_EWALLET_PROTO_SOCKETBASE_H
#define SCAG_BILL_EWALLET_PROTO_SOCKETBASE_H

#include <cassert>
#include <memory>
#include "Context.h"
#include "scag/bill/ewallet/Streamer.h"
#include "core/network/Socket.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "scag/util/Time.h"
#include "logger/Logger.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

class Core;
class Socket;

/// socket to connect to ewallet
class SocketBase
{
protected:
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
    SocketBase( Core& core );

    virtual Socket* castToSocket() { return 0; }

    virtual bool isConnected() const = 0;

    /// send packet to a sending queue and notify writer.
    /// NOTE: invoked from core.
    /// NOTE: if exception is thrown, context fields are not touched.
    virtual void send( std::auto_ptr< Context >& context, bool sendRequest ) = 0; // throw

    /// attach/detach a socket: refcounting
    void attach( const char* who );
    void detach( const char* who );

    /// this one is used to make sure that all other threads has detached from socket
    unsigned attachCount();

    // is used to set/unset writer which this socket should notify
    // void setWriter( SocketWriter* writer );

    // connect
    // void connect(); // throw

    /// close the socket
    virtual void close() = 0;

    virtual void updateActivity( util::msectime_type la = 0 ) {}

protected:
    virtual ~SocketBase();

protected:
    static smsc::logger::Logger*                 log_;
    Core&                                        core_;

    smsc::core::synchronization::Mutex           refMutex_;
    size_t                                       refCount_;

    smsc::core::synchronization::EventMonitor          queueMon_;
    smsc::core::buffers::CyclicQueue< WriteContext* >* queue_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_SOCKETBASE_H */
