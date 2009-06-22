#ifndef SCAG_BILL_EWALLET_PROTO_CORE_H
#define SCAG_BILL_EWALLET_PROTO_CORE_H

#include <memory>
#include <vector>
#include "Socket.h"
#include "Context.h"
#include "Config.h"
#include "SocketInactivityTracker.h"
#include "logger/Logger.h"
#include "scag/bill/ewallet/Streamer.h"
#include "scag/bill/ewallet/Exception.h"
#include "scag/util/WatchedThreadedTask.h"
#include "core/threads/ThreadPool.hpp"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

class Config;
class SocketReader;
class SocketWriter;

class Core : public util::WatchedThreadedTask
{
public:
    Core( Config* config, Streamer* streamer ) :
    log_(0), config_(config), streamer_(streamer) {
        log_ = smsc::logger::Logger::getInstance("ewall.core");
    }

    virtual ~Core() {}

    /// receive a packet from network.
    /// invoked from reader.
    virtual void receivePacket( Socket& socket, std::auto_ptr< Packet > packet ) = 0;

    /// report when a packet is sent.
    /// invoked from writer.
    /// context gets owned.
    virtual void reportPacket( Socket& socket, uint32_t seqNum, Context* context, Context::ContextState state ) = 0;

    /// handle error occured at socket.
    virtual void handleError( Socket& socket, const Exception& exc ) = 0;

    /// close socket and detach from it.
    virtual void closeSocket( Socket& socket );

    /// get the configuration
    const Config& getConfig() const { return *config_.get(); }

    /// get the streamer
    Streamer& getStreamer() { return *streamer_.get(); }

    /// register socket for processing, socket must be connected.
    virtual bool registerSocket( Socket& socket );

    /// invoked from tracker
    virtual void inactivityTimeout( Socket& socket ) = 0;

protected:
    void startupIO();
    void shutdownIO( bool serverLike );
    void stopReaders();
    void stopWriters( bool serverLike );
    // virtual void updateSocketActivity( Socket& socket );
    virtual void stopCoreLogic() = 0;

protected:
    smsc::logger::Logger*                    log_;
    smsc::core::threads::ThreadPool          threadPool_;
    std::auto_ptr< Config >                  config_;
private:
    std::auto_ptr< Streamer >                streamer_;
    std::auto_ptr< SocketInactivityTracker > tracker_;

    smsc::core::synchronization::Mutex writersMutex_;
    smsc::core::synchronization::Mutex readersMutex_;
    std::vector< SocketReader* >       readers_;
    std::vector< SocketWriter* >       writers_;

    smsc::core::synchronization::Mutex deadMutex_;
    std::vector< SocketTask* >         deadTasks_;
};

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PROTO_CORE_H */
