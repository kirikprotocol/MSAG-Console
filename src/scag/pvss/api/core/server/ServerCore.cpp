#include "ServerCore.h"
#include "ContextQueue.h"
#include "ServerContext.h"
#include "Dispatcher.h"
#include "scag/pvss/api/packets/PingResponse.h"
#include "scag/pvss/api/packets/ErrorResponse.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {


ServerCore::ServerCore( ServerConfig& config, Protocol& protocol ) :
Core(config,protocol), Server(),
started_(false)
{
}


void ServerCore::receivePacket( std::auto_ptr<Packet> packet, PvssSocket& channel )
{
    uint32_t seqNum = uint32_t(-1);
    Response::StatusType status = Response::OK;
    try {

        if ( !packet.get() || !packet->isRequest() || !packet->isValid() )
            throw PvssException(PvssException::BAD_REQUEST,"Received packet isnt valid PVAP request");

        seqNum = packet->getSeqNum();
        updateChannelActivity(channel);
        if (!started_) {
            status = Response::SERVER_SHUTDOWN;
            throw PvssException(PvssException::SERVER_BUSY,"Server is down");
        }

        Request* req = static_cast<Request*>(packet.get());
        if ( req->isPing() ) {
            seqNum = uint32_t(-1);
            sendResponse( new PingResponse(packet->getSeqNum()),channel );
            return;
        }

        const int idx = dispatcher_->getIndex(*req,channel);
        if ( idx > queues_.Count() ) {
            status = Response::ERROR;
            throw PvssException(PvssException::UNKNOWN,"cannot dispatch");
        }
        ContextQueue* queue = queues_[idx];
        if (queue->getSize() >= getConfig().getQueueSizeLimit()) {
            status = Response::SERVER_BUSY;
            throw PvssException(PvssException::SERVER_BUSY,"try later");
        }

        status = Response::SERVER_BUSY;
        std::auto_ptr<ServerContext> ctx(new ServerContext(static_cast<Request*>(packet.release()),channel));
        queue->requestReceived(ctx);
        // seqNum = uint32_t(-1);

    } catch (PvssException& e) {
        smsc_log_error(logger, "exception: %s",e.what());
        if ( seqNum != uint32_t(-1) ) {
            try {
                sendResponse(new ErrorResponse(seqNum,status,e.what()),channel);
            } catch (PvssException& e) {
                smsc_log_error(logger,"exception: %s", e.what());
            }
        }
    }
}


void ServerCore::startup() throw (PvssException)
{
    if (started_) return;

    startupIO();
    try {
        acceptor_.reset(new Acceptor(getConfig().getHost(), getConfig().getPort()));
        threadPool_.startTask(acceptor_.get(),false);
    } catch (PvssException& exc) {
        smsc_log_error(logger,"Acceptor start error: %s",exc.what());
        shutdownIO();
        throw exc;
    }

    started_ = true;
    threadPool_.startTask(this,false);
}





} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
