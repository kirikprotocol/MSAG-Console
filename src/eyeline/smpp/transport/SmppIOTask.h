#ifndef _EYELINE_SMPP_SMPPIOTASK_H
#define _EYELINE_SMPP_SMPPIOTASK_H

#include <vector>
#include "informer/io/EmbedRefPtr.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/AtomicCounter.hpp"
#include "core/network/Pipe.hpp"
#include "core/network/Multiplexer.hpp"
#include "Socket.h"
#include "logger/Logger.h"

namespace eyeline {
namespace smpp {

class SocketCloser;

class SmppIOTask : public smsc::core::threads::ThreadedTask
{
    friend class eyeline::informer::EmbedRefPtr< SmppIOTask >;
    friend class eyeline::informer::EmbedRefPtr< SmppWriter >;

protected:
    struct PendingSocket {
        SocketPtr  socket;
        bool           add;
    };
    typedef smsc::core::buffers::CyclicQueue< PendingSocket > PendingQueue;

public:
    SmppIOTask( const char* taskName,
                SocketCloser& closer );

    ~SmppIOTask();

    virtual void stop();

    void addSocket( Socket& sock );

    void removeSocket( Socket& sock );

    size_t getSocketCount() {
        smsc::core::synchronization::MutexGuard mg(mon_);
        return sockets_.size();
    }

    virtual const char* taskName() {
        return taskName_.c_str();
    }

protected:
    /// sync
    void swapPendingSockets( PendingQueue& pq );
    /// not sync
    void handlePendingSockets( PendingQueue& pq );
    virtual void postSetSocket( bool add, Socket& sock ) {}

private:
    void ref() { ref_.inc(); }

    void unref() {
        if (ref_.dec()) return;
        delete this;
    }

private:
    smsc::core::synchronization::AtomicCounter<unsigned> ref_;

protected:
    std::string                                       taskName_;
    SocketCloser&                                     socketCloser_;
    smsc::logger::Logger*                             log_;

    smsc::core::synchronization::EventMonitor         mon_;
    PendingQueue                                      pending_;

    std::vector< SocketPtr >                          sockets_; // only modified from the execution thread
    smsc::core::network::Pipe                         wakePipe_;
    smsc::core::network::Multiplexer                  mul_;
};

typedef eyeline::informer::EmbedRefPtr< SmppIOTask > SmppIOTaskPtr;

}
}

#endif
