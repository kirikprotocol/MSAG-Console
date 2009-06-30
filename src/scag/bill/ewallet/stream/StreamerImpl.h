#ifndef SCAG_BILL_EWALLET_STREAM_STREAMERIMPL_H
#define SCAG_BILL_EWALLET_STREAM_STREAMERIMPL_H

#include "scag/bill/ewallet/Streamer.h"
#include "scag/bill/ewallet/RequestVisitor.h"
#include "scag/bill/ewallet/ResponseVisitor.h"
#include "scag/bill/ewallet/stream/generated-cpp/Protocol.hpp"

// #include "scag/bill/ewallet/Ping.h"
#include "scag/bill/ewallet/Open.h"
#include "scag/bill/ewallet/Commit.h"
#include "scag/bill/ewallet/Rollback.h"
// #include "scag/bill/ewallet/PingResp.h"
#include "scag/bill/ewallet/OpenResp.h"
#include "scag/bill/ewallet/CommitResp.h"
#include "scag/bill/ewallet/RollbackResp.h"
#include "scag/util/io/EndianConverter.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace stream {

class StreamerImpl : public ewallet::Streamer
{
private:
    struct DeserializeHandler : public stream::Protocol::Handler
    {
        DeserializeHandler() : streamer(this) {}
        /*
        virtual void handle( stream::Ping& o   ) {
            ewallet::Ping* p = new ewallet::Ping;
            packet.reset(p);
            p->setSeqNum(o.getSeqNum());
        }
        virtual void handle( stream::PingResp& o ) {
            ewallet::PingResp* p = new ewallet::PingResp;
            packet.reset(p);
            p->setSeqNum(o.getSeqNum());
        }
        virtual void handle( stream::Auth& obj ) {
            throw Exception( "auth is not supported", Status::NOT_SUPPORTED );
        }
        virtual void handle( stream::AuthResp& obj ) {
            throw Exception( "authresp is not supported", Status::NOT_SUPPORTED );
        }
         */
        virtual void handle( stream::Open& o ) {
            ewallet::Open* p = new ewallet::Open;
            packet.reset(p);
            p->setSeqNum(o.getSeqNum());
            if (o.hasSourceId()) p->setSourceId(o.getSourceId());
            p->setAgentId(o.getAgentId());
            p->setUserId(o.getUserId());
            p->setWalletType(o.getWalletType());
            p->setDescription(o.getDescription());
            p->setAmount(o.getAmount());
            if (o.hasExternalId()) p->setExternalId(o.getExternalId());
            p->setTimeout(o.getTimeout());
        }
        virtual void handle( stream::OpenResp& o ) {
            ewallet::OpenResp* p = new ewallet::OpenResp;
            packet.reset(p);
            p->setSeqNum(o.getSeqNum());
            p->setStatus(o.getStatusValue());
            if ( p->getStatus() == Status::OK ) {
                p->setTransId(o.getTransId());
                p->setAmount(o.getAmount());
                p->setChargeThreshold(o.getChargeThreshold());
            }
        }
        virtual void handle( stream::Commit& o ) {
            ewallet::Commit* p = new ewallet::Commit;
            packet.reset(p);
            p->setSeqNum(o.getSeqNum());
            p->setSourceId(o.getSourceId());
            p->setAgentId(o.getAgentId());
            p->setUserId(o.getUserId());
            p->setWalletType(o.getWalletType());
            p->setAmount(o.getAmount());
            p->setExternalId(o.getExternalId());
            p->setTransId(o.getTransId());
        }
        virtual void handle( stream::CommitResp& o ) {
            ewallet::CommitResp* p = new ewallet::CommitResp;
            packet.reset(p);
            p->setSeqNum(o.getSeqNum());
            p->setStatus(o.getStatusValue());
        }
        virtual void handle( stream::Rollback& o ) {
            ewallet::Rollback* p = new ewallet::Rollback;
            packet.reset(p);
            p->setSeqNum(o.getSeqNum());
            p->setSourceId(o.getSourceId());
            p->setAgentId(o.getAgentId());
            p->setUserId(o.getUserId());
            p->setWalletType(o.getWalletType());
            p->setExternalId(o.getExternalId());
            p->setTransId(o.getTransId());
        }
        virtual void handle( stream::RollbackResp& o ) {
            ewallet::RollbackResp* p = new ewallet::RollbackResp;
            packet.reset(p);
            p->setSeqNum(o.getSeqNum());
            p->setStatus(o.getStatusValue());
        }
        virtual void handle( stream::Info& o ) {
            throw Exception( "info is not supported", Status::NOT_SUPPORTED );
        }
        virtual void handle( stream::InfoResp& o ) {
            throw Exception( "inforesp is not supported", Status::NOT_SUPPORTED );
        }
        virtual void handle( stream::Transfer& o ) {
            throw Exception( "transfer is not supported", Status::NOT_SUPPORTED );
        }
        virtual void handle( stream::TransferResp& o ) {
            throw Exception( "transferresp is not supported", Status::NOT_SUPPORTED );
        }
        virtual void handle( stream::Check& o ) {
            throw Exception( "check is not supported", Status::NOT_SUPPORTED );
        }
        virtual void handle( stream::CheckResp& o ) {
            throw Exception( "checkresp is not supported", Status::NOT_SUPPORTED );
        }
        virtual void handle( stream::TransferCheck& o ) {
            throw Exception( "transfercheck is not supported", Status::NOT_SUPPORTED );
        }
        virtual void handle( stream::TransferCheckResp& o ) {
            throw Exception( "transfercheckresp is not supported", Status::NOT_SUPPORTED );
        }

        stream::Protocol      streamer;
        std::auto_ptr<Packet> packet;
    };

    struct SerializeVisitor : public ewallet::RequestVisitor, public ewallet::ResponseVisitor
    {
    public:
        SerializeVisitor( stream::Protocol& p, Buffer& b ) : proto_(p), writer_(b) {}
        /*
        virtual bool visitPing( ewallet::Ping& o ) {
            stream::Ping p;
            p.setSeqNum(o.getSeqNum());
            proto_.encodeMessage(p,writer_);
            return true;
        }
        virtual bool visitAuth( ewallet::Auth& o ) {
            return false;
        }
         */
        virtual bool visitOpen( ewallet::Open& o ) {
            stream::Open p;
            p.setSeqNum(o.getSeqNum());
            if (!o.getSourceId().empty()) p.setSourceId(o.getSourceId());
            p.setAgentId(o.getAgentId());
            p.setUserId(o.getUserId());
            p.setWalletType(o.getWalletType());
            p.setDescription(o.getDescription());
            p.setAmount(o.getAmount());
            if (!o.getExternalId().empty()) p.setExternalId(o.getExternalId());
            p.setTimeout(o.getTimeout());
            proto_.encodeMessage(p,writer_);
            return true;
        }
        virtual bool visitCommit( ewallet::Commit& o ) {
            stream::Commit p;
            p.setSeqNum(o.getSeqNum());
            p.setSourceId(o.getSourceId());
            p.setAgentId(o.getAgentId());
            p.setUserId(o.getUserId());
            p.setWalletType(o.getWalletType());
            p.setAmount(o.getAmount());
            p.setExternalId(o.getExternalId());
            p.setTransId(o.getTransId());
            proto_.encodeMessage(p,writer_);
            return true;
        }
        virtual bool visitRollback( ewallet::Rollback& o ) {
            stream::Rollback p;
            p.setSeqNum(o.getSeqNum());
            p.setSourceId(o.getSourceId());
            p.setAgentId(o.getAgentId());
            p.setUserId(o.getUserId());
            p.setWalletType(o.getWalletType());
            p.setExternalId(o.getExternalId());
            p.setTransId(o.getTransId());
            proto_.encodeMessage(p,writer_);
            return true;
        }
        /*
        virtual bool visitPingResp( ewallet::PingResp& o ) {
            stream::PingResp p;
            p.setSeqNum(o.getSeqNum());
            p.setStatusValue(o.getStatus());
            proto_.encodeMessage(p,writer_);
            return true;
        }
         */
        /*
        virtual bool visitAuthResp( ewallet::AuthResp& o ) {
            return false;
        }
         */
        virtual bool visitOpenResp( ewallet::OpenResp& o ) {
            stream::OpenResp p;
            p.setSeqNum(o.getSeqNum());
            p.setStatusValue(o.getStatus());
            if ( o.getStatus() == Status::OK ) {
                p.setTransId(o.getTransId());
                p.setAmount(o.getAmount());
                p.setChargeThreshold(o.getChargeThreshold());
            }
            proto_.encodeMessage(p,writer_);
            return true;
        }
        virtual bool visitCommitResp( ewallet::CommitResp& o ) {
            stream::CommitResp p;
            p.setSeqNum(o.getSeqNum());
            p.setStatusValue(o.getStatus());
            proto_.encodeMessage(p,writer_);
            return true;
        }
        virtual bool visitRollbackResp( ewallet::RollbackResp& o ) {
            stream::RollbackResp p;
            p.setSeqNum(o.getSeqNum());
            p.setStatusValue(o.getStatus());
            proto_.encodeMessage(p,writer_);
            return true;
        }

        void fixLength( Buffer& buf )
        {
            size_t buflen = buf.GetPos();
            if (buflen<4) return;
            util::io::EndianConverter::set32(buf.get(),buflen);
        }

    private:
        stream::Protocol& proto_;
        BufferWriter     writer_;
    };

public:
    virtual ~StreamerImpl() {}
    virtual void serialize( const Packet& packet, Buffer& buf ) {
        SerializeVisitor sv(proto_,buf);
        buf.SetPos(4);
        if ( packet.isRequest() ) {
            const_cast<Request&>(static_cast<const Request&>(packet)).visit(sv);
        } else {
            const_cast<Response&>(static_cast<const Response&>(packet)).visit(sv);
        }
        sv.fixLength(buf);
    }
    virtual Packet* deserialize( Buffer& buf ) {
        DeserializeHandler dh;
        BufferReader reader(buf);
        const size_t buflen = buf.GetPos();
        const size_t rbuflen = reader.readInt();
        if (buflen != rbuflen) {
            throw Exception(Status::IO_ERROR,"buffer size mismatch: len=%u readlen=%u",
                            unsigned(buflen),unsigned(rbuflen));
        }
        dh.streamer.decodeMessage(reader);
        return dh.packet.release();
    }

private:
    stream::Protocol proto_;
};

} // namespace stream
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_STREAM_STREAMERIMPL_H */
