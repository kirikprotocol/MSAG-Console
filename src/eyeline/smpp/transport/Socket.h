#ifndef _EYELINE_SMPP_SOCKET_H
#define _EYELINE_SMPP_SOCKET_H

#include <memory>
#include "informer/io/TmpBuf.h"
#include "informer/io/EmbedRefPtr.h"
#include "eyeline/smpp/Utility.h"
#include "eyeline/smpp/BindMode.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/network/Socket.hpp"
#include "core/network/Multiplexer.hpp"
#include "core/synchronization/AtomicCounter.hpp"
#include "eyeline/smpp/pdu/PduBuffer.h"

namespace eyeline {
namespace smpp {

class SessionBase;
class SendSubscriber;
class SmppWriter;

/// a socket
class Socket
{
    friend class eyeline::informer::EmbedRefPtr< Socket >;
    friend class SmppWriter;

    static unsigned globalSocketId_;
    static unsigned getGlobalSocketId();

    static void initLog();

protected:
    static smsc::logger::Logger* log_;

public:
    /// 0 -- client socket, otherwise server socket
    Socket( smsc::core::network::Socket* s,
            size_t maxrsize, size_t maxwqueue );
    virtual ~Socket();

    inline unsigned getSocketId() const { return socketId_; }

    eyeline::informer::EmbedRefPtr< SessionBase > getSession();
    void setSession( SessionBase* session );
    void setWriter( SmppWriter* writer );
    eyeline::informer::EmbedRefPtr< SmppWriter > getWriter();

    void send( PduBuffer buffer, SendSubscriber* subs );

    /// return bindmode or BindMode(0)
    inline BindMode getBindMode() const { return bindmode_; }
    void setBindMode( BindMode bm );

    // used to remeber the timer time to be compared in elapsed() methods
    // inline void setTimerTime( time_type alarm ) { alarm_ = alarm; }

    inline msectime_type getLastReadTime() const { return lastread_; }
    // inline msectime_type getLastWriteTime() const { return lastwrite_; }

    inline static Socket* fromSocket( smsc::core::network::Socket* s ) {
        return reinterpret_cast<Socket*>(s ? s->getData(0) : 0);
    }
    inline void addTo( smsc::core::network::Multiplexer& mul ) {
        mul.add(socket_);
    }
    inline void removeFrom( smsc::core::network::Multiplexer& mul ) {
        mul.remove(socket_);
    }

    inline bool isConnected() const {
        return socket_->isConnected();
    }

    void processInput();

    /// set flag if socket should be shut down
    /// @return true if socket has no data to send
    bool setShutdown();

    bool isShutdown() const { return shutdown_; }

protected:
    void unsetShutdown();

private:

    /// NOTE: only to be invoked from SmppWriter
    bool hasWriteData();

    /// send data.  return true if there is more data to send.
    /// throw exception if socket should be closed.
    bool sendData();

    /// add gen_nack
    void addGNack( int status );

private:
    void ref() {
        ref_.inc();
        /*
        const unsigned refs = ref_.inc();
        if ( refs == 1 ) {
            const unsigned total = total_.inc();
            smsc_log_debug(log_,"Sk%u @%p total=%u",socketId_,this,total);
        }
         */
    }

    void unref() {
        const unsigned refs = ref_.dec();
        if ( !refs ) {
            // const unsigned total = total_.dec();
            // smsc_log_debug(log_,"Sk%u @%p total=%u",si,this,total);
            delete this;
        }
    }

private:
    struct BufferMark 
    {
        SendSubscriber* subs;
        PduBuffer       buf;
    };


protected:
    smsc::core::network::Socket* socket_;
private:
    smsc::core::synchronization::AtomicCounter<unsigned> ref_;
    static smsc::core::synchronization::AtomicCounter<unsigned> total_;

    unsigned socketId_;
    eyeline::informer::EmbedRefPtr< SessionBase > session_;
    size_t                       maxrsize_;
    size_t                       maxwqueue_;

    smsc::core::synchronization::AtomicCounter<unsigned> pducount_;

    msectime_type lastread_;
    BindMode      bindmode_;

    smsc::core::synchronization::Mutex writeMutex_;
    eyeline::informer::EmbedRefPtr< SmppWriter > writer_;
    bool          shutdown_; // is set under writemutex

    smsc::core::buffers::CyclicQueue< BufferMark > wbuf_[2];
    size_t   wpos_;   // current writing position

    unsigned writeIdx_; // 0, 1

    eyeline::informer::TmpBuf<char,1024> rbuf_;
    size_t   rpos_; // position in buffer;
    size_t   rlen_; // the length of the package or 0
};

typedef eyeline::informer::EmbedRefPtr< Socket >  SocketPtr;

}
}

#endif
