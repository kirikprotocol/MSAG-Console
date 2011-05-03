#include "Loopback.h"
#include "ClientCore.h"
#include "scag/bill/ewallet/RequestVisitor.h"
// #include "scag/bill/ewallet/Ping.h"
#include "scag/bill/ewallet/Open.h"
#include "scag/bill/ewallet/Info.h"
#include "scag/bill/ewallet/Commit.h"
#include "scag/bill/ewallet/Rollback.h"
#include "scag/bill/ewallet/Check.h"
#include "scag/bill/ewallet/Transfer.h"
// #include "scag/bill/ewallet/PingResp.h"
#include "scag/bill/ewallet/OpenResp.h"
#include "scag/bill/ewallet/CommitResp.h"
#include "scag/bill/ewallet/RollbackResp.h"
#include "scag/bill/ewallet/CheckResp.h"
#include "scag/bill/ewallet/TransferResp.h"
#include "scag/bill/ewallet/InfoResp.h"

using smsc::core::synchronization::MutexGuard;

namespace {
using namespace scag2::bill::ewallet;

class ReqProcessor : public RequestVisitor
{
public:
    /*
    virtual bool visitPing( Ping& req ) {
        resp_.reset( new PingResp() );
        resp_->setSeqNum(req.getSeqNum());
        resp_->setStatus(Status::OK);
        return true;
    }
     */
    /*
    virtual bool visitAuth( Auth& ) {
        resp_.reset( new AuthResp(req.getSeqNum()) );
        return true;
    }
     */
    virtual bool visitOpen( Open& req ) {
        OpenResp* o = new OpenResp();
        resp_.reset(o);
        o->setStatus(Status::OK);
        o->setSeqNum(req.getSeqNum());
        o->setTransId(10);
        o->setAmount(120);
        o->setChargeThreshold(90);
        return true;
    }
    virtual bool visitCommit( Commit& req ) {
        resp_.reset( new CommitResp() );
        resp_->setSeqNum(req.getSeqNum());
        resp_->setStatus(Status::OK);
        return true;
    }
    virtual bool visitRollback( Rollback& req ) {
        resp_.reset( new RollbackResp() );
        resp_->setSeqNum(req.getSeqNum());
        resp_->setStatus(Status::OK);
        return true;
    }
    virtual bool visitCheck( Check& req ) {
        CheckResp* o = new CheckResp();
        resp_.reset(o);
        o->setStatus(Status::NOT_SUPPORTED);
        o->setSeqNum(req.getSeqNum());
        return true;
    }
    virtual bool visitTransfer( Transfer& req ) {
        TransferResp* o = new TransferResp();
        resp_.reset(o);
        o->setStatus(Status::NOT_SUPPORTED);
        o->setSeqNum(req.getSeqNum());
        return true;
    }
    virtual bool visitInfo( Info& req ) {
        InfoResp* o = new InfoResp();
        resp_.reset(o);
        o->setStatus(Status::NOT_SUPPORTED);
        o->setSeqNum(req.getSeqNum());
        return true;
    }

    Response* popResponse() {
        return resp_.release();
    }
private:
    std::auto_ptr< Response > resp_;
};

}


namespace scag2 {
namespace bill {
namespace ewallet {
namespace client {

Loopback::Loopback( ClientCore& core ) :
proto::SocketBase(core),
processor_(new Processor(*this))
{
}


Loopback::~Loopback()
{
    close();
}


bool Loopback::isConnected() const
{
    return processor_->isStarted();
}


void Loopback::close()
{
    if ( processor_->isStarted() ) {
        processor_->stop();
        processor_->waitUntilReleased();
    }
}


/*
void Loopback::shutdown()
{
 stop();
 waitUntilReleased();
}
 */


void Loopback::send( std::auto_ptr< proto::Context >& context, bool sendRequest )
{
    // the content is taken from Socket::send
    const Packet* packet;
    const char* what;
    if ( sendRequest ) {
        packet = context->getRequest().get();
        what = "request";
    } else {
        packet = context->getResponse().get();
        what = "response";
    }
    if ( !packet ) {
        throw Exception( sendRequest ? Status::BAD_REQUEST : Status::BAD_RESPONSE,
                         "%s is null", what );
    } else if ( !packet->isValid() ) {
        throw Exception( sendRequest ? Status::BAD_REQUEST : Status::BAD_RESPONSE,
                         "%s is bad formed", what );
    }

    // if ( ! isConnected() ) {
    // throw Exception( "socket is not connected", Status::NOT_CONNECTED );
    // }
    // if ( stopping() ) {
    // throw Exception( "loopback is stopped", Status::NOT_CONNECTED );
    // }
    // if ( writer_->stopping() ) {
    // throw Exception( "writer is not started", Status::NOT_CONNECTED );
    // }
    // fast check for queue limit (w/o locking)
    if ( sendRequest && size_t(queue_->Count()) > core_.getConfig().getSocketQueueSizeLimit() ) {
        throw Exception( "queue size limit exceeded", Status::CLIENT_BUSY );
    }

    std::auto_ptr< WriteContext > writeContext
        ( new WriteContext( sendRequest ? util::currentTimeMillis() : 0 ) );

    // serialization
    smsc_log_debug(log_,"serializing %s",packet->toString().c_str());
    core_.getStreamer().serialize( *packet, writeContext->buffer );

    { // pushing to the queue, checking its size again
        MutexGuard mgq(queueMon_);
        if ( processor_->stopping() ) {
            throw Exception( "loopback is stopped", Status::NOT_CONNECTED );
        }
        
        if ( sendRequest && size_t(queue_->Count()) > core_.getConfig().getSocketQueueSizeLimit() ) {
            throw Exception("queue size limit exceeded", Status::CLIENT_BUSY);
        }
        writeContext->setContext( context.release() );
        queue_->Push( writeContext.release() );
        queueMon_.notify();
    }
    // writer_->wakeup();
}


proto::SocketBase::WriteContext* Loopback::getContext()
{
    MutexGuard mg(queueMon_);
    while ( true ) {
        if ( queue_->Count() > 0 ) {
            WriteContext* ctx;
            if (queue_->Pop(ctx)) return ctx;
        } else if ( processor_->stopping() ) {
            return 0;
        } else {
            queueMon_.wait(200);
        }
    }
}


int Loopback::Processor::doExecute()
{
    smsc_log_info(log_,"loopback is started");
    while ( true ) {

        WriteContext* ctx = loopback_->getContext();

        if ( ctx ) {

            std::auto_ptr< WriteContext > wrContext(ctx);
            // processing ctx
            if ( stopping() ) {
                loopback_->core_.reportPacket(*loopback_,ctx->getSeqNum(),ctx->popContext(),proto::Context::FAILED);
                continue;
            }

            util::msectime_type currentTime = util::currentTimeMillis();
            if ( ctx->creationTime == 0 ||
                 ctx->creationTime + 
                 loopback_->core_.getConfig().getIOTimeout() >= currentTime ) {
                // sending
                loopback_->core_.reportPacket(*loopback_,ctx->getSeqNum(),ctx->popContext(),proto::Context::SENDING);
                processBuffer( ctx->getSeqNum(), ctx->buffer.get(), ctx->buffer.GetPos() );
            } else {
                loopback_->core_.reportPacket(*loopback_,ctx->getSeqNum(),ctx->popContext(),proto::Context::EXPIRED);
            }

        } else if ( stopping() ) {
            // last context when stopping
            break;
        }

    } // while
    smsc_log_info(log_,"loopback is stopped");
    return 0;
}


void Loopback::Processor::processBuffer( uint32_t seqNum, char* buffer, size_t buflen )
{
    std::auto_ptr<Packet> resp;
    do {

        if (log_->isDebugEnabled()) {
            util::HexDump hd;
            util::HexDump::string_type dump;
            hd.hexdump(dump,buffer,buflen);
            smsc_log_debug(log_,"buffer to send: %s",hd.c_str(dump));
        }

        // packet is sent
        loopback_->core_.reportPacket(*loopback_,seqNum,0,proto::Context::SENT);
        
        std::auto_ptr< Packet > packet;
        try {
            Streamer::Buffer buf(buffer,buflen);
            buf.SetPos(buflen);
            packet.reset( loopback_->core_.getStreamer().deserialize(buf) );
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"exc on buffer input on server-side: %s", e.what());
            break;
        }

        if ( ! packet.get() ) {
            smsc_log_error(log_,"deserialized packet is NULL");
            break;
        }

        smsc_log_debug(log_,"packet %s is received by server",packet->toString().c_str());
        if ( ! packet->isRequest() ) {
            smsc_log_error(log_,"packet is not a request");
            break;
        }
        if ( ! packet->isValid() ) {
            smsc_log_error(log_,"request is not valid");
            break;
        }

        // valid request
        Request* req = static_cast< Request* >( packet.get() );
        ReqProcessor proc;
        if ( ! req->visit(proc) ) {
            smsc_log_warn(log_,"packet is not supported");
            break;
        }

        resp.reset(proc.popResponse());

        if ( ! resp.get() ) {
            smsc_log_warn(log_,"response is not filled");
            break;
        }

        Streamer::Buffer buf;
        try {
            loopback_->core_.getStreamer().serialize(*resp.get(),buf);
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"exc forming an outgoing server buffer: %s", e.what());
            break;
        }

        if (log_->isDebugEnabled()) {
            util::HexDump hd;
            util::HexDump::string_type dump;
            hd.hexdump(dump,buf.get(),buf.GetPos());
            smsc_log_debug(log_,"buffer to receive: %s",hd.c_str(dump));
        }

        try {
            resp.reset(loopback_->core_.getStreamer().deserialize(buf));
        } catch ( std::exception& e ) {
            smsc_log_debug(log_,"exc receiving packet: %s",e.what());
            break;
        }

    } while ( false );

    if ( resp.get() ) {
        // response is received
        loopback_->core_.receivePacket( *loopback_, resp );
    }
}

} // namespace client
} // namespace ewallet
} // namespace bill
} // namespace scag2
