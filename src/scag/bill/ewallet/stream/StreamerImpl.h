#ifndef SCAG_BILL_EWALLET_STREAM_STREAMERIMPL_H
#define SCAG_BILL_EWALLET_STREAM_STREAMERIMPL_H

#include "scag/bill/ewallet/Streamer.h"
#include "scag/bill/ewallet/RequestVisitor.h"
#include "scag/bill/ewallet/ResponseVisitor.h"
#include "scag/bill/ewallet/stream/generated-cpp/Protocol.hpp"

#include "scag/bill/ewallet/Ping.h"
#include "scag/bill/ewallet/Open.h"
#include "scag/bill/ewallet/Commit.h"
#include "scag/bill/ewallet/Rollback.h"
#include "scag/bill/ewallet/PingResp.h"
#include "scag/bill/ewallet/OpenResp.h"
#include "scag/bill/ewallet/CommitResp.h"
#include "scag/bill/ewallet/RollbackResp.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace stream {

class StreamerImpl : public ewallet::Streamer
{
private:
    struct DeserializeHandler : public stream::Protocol::Handler
    {
    };

    struct SerializeVisitor : public ewallet::RequestVisitor, public ewallet::ResponseVisitor
    {
    public:
        SerializeVisitor( stream::Protocol& p, Buffer& b ) : proto_(p), writer_(b) {}
        virtual bool visitPing( ewallet::Ping& o ) {
            stream::Ping p;
            p.setSeqNum(o.getSeqNum());
            proto_.encodeMessage(p,writer_);
            return true;
        }
        virtual bool visitAuth( ewallet::Auth& o ) {
            /*
            stream::Auth p;
            p.setSeqNum(o.getSeqNum());
            p.
            proto_.encodeMessage(p,writer_);
             */
            return false;
        }
        virtual bool visitOpen( ewallet::Open& o ) {
            stream::Open p;
            p.setSeqNum(o.getSeqNum());
            p.setSourceId(o.getSourceId());
            p.setAgentId(o.getAgentId());
            p.setUserId(o.getUserId());
            p.setWalletType(o.getWalletType());
            p.setDescription(o.getDescription());
            p.setAmount(o.getAmount());
            p.setExternalId(o.getExternalId());
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
        virtual bool visitPingResp( ewallet::PingResp& o ) {
            stream::PingResp p;
            p.setSeqNum(o.getSeqNum());
            p.setStatusValue(o.getStatus());
            proto_.encodeMessage(p,writer_);
            return true;
        }
        /*
        virtual bool visitAuthResp( ewallet::AuthResp& o ) {
            return false;
        }
         */
        virtual bool visitOpenResp( ewallet::OpenResp& o ) {
            stream::OpenResp p;
            p.setSeqNum(o.getSeqNum());
            p.setStatusValue(o.getStatus());
            p.setTransId(o.getTransId());
            p.setAmount(o.getAmount());
            p.setChargeThreshold(o.getChargeThreshold());
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

    private:
        stream::Protocol& proto_;
        BufferWriter     writer_;
    };

public:
    virtual ~StreamerImpl() {}
    virtual void serialize( const Packet& packet, Buffer& buf ) {
        SerializeVisitor sv(proto_,buf);
        if ( packet.isRequest() ) {
            const_cast<Request&>(static_cast<const Request&>(packet)).visit(sv);
        } else {
            const_cast<Response&>(static_cast<const Response&>(packet)).visit(sv);
        }
    }
    virtual Packet* deserialize( Buffer& buf ) {
        /*
        DeserializeHandler dh;
        stream::Streamer p(&dh);
        p.decodeMessage(buf);
        return dh.packet;
         */
        return 0;
    }

private:
    stream::Protocol proto_;
};

} // namespace stream
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_STREAM_STREAMERIMPL_H */
