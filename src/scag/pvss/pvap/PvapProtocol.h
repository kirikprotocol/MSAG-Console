#ifndef _SCAG_PVSS_PVAP_PVAPPROTOCOL_H
#define _SCAG_PVSS_PVAP_PVAPPROTOCOL_H

#include "scag/pvss/packets/Protocol.h"
#include "scag/pvss/packets/RequestVisitor.h"
#include "scag/pvss/packets/ProfileCommandVisitor.h"
#include "scag/pvss/packets/ResponseVisitor.h"
#include "scag/pvss/pvap/generated-cpp/PVAP.hpp"

namespace scag2 {
namespace pvss {
namespace pvap {

class PvapProtocol : public Protocol
{
private:
    class BufferWriterVisitor : public RequestVisitor, public ProfileCommandVisitor, public ResponseVisitor 
    {
    public:
        BufferWriterVisitor( Buffer& buf ) : writer(buf), packet(0) {
        }

        virtual bool visitPingRequest( PingRequest& req ) throw(PvapException) {
            PC_PING msg(&req);
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitAuthRequest( AuthRequest& req ) throw(PvapException) {
            PC_AUTH msg(&req);
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitProfileRequest( AbstractProfileRequest& req ) throw(PvapException) {
            packet = &req;
            AbstractCommand* cmd = req.getCommand();
            return cmd->visit( req.getProfileKey(), *this );
        }
        virtual bool visitDelCommand( const ProfileKey& key, DelCommand& cmd ) throw(PvapException) {
            PC_DEL msg( static_cast< ProfileRequest<DelCommand>* >( packet ) );
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitSetCommand( const ProfileKey& key, SetCommand& cmd ) throw(PvapException) {
            PC_SET msg( static_cast< ProfileRequest<SetCommand>* >( packet ) );
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitGetCommand( const ProfileKey& key, GetCommand& cmd ) throw(PvapException) {
            PC_GET msg( static_cast< ProfileRequest<GetCommand>* >( packet ) );
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitIncCommand( const ProfileKey& key, IncCommand& cmd ) throw(PvapException) {
            PC_INC msg( static_cast< ProfileRequest<IncCommand>* >( packet ) );
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitIncModCommand( const ProfileKey& key, IncModCommand& cmd ) throw(PvapException) {
            PC_INC_MOD msg( static_cast< ProfileRequest<IncModCommand>* >( packet ) );
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitBatchCommand( const ProfileKey& key, BatchCommand& cmd ) throw(PvapException) {
            PC_BATCH msg( static_cast< ProfileRequest< BatchCommand >* >( packet ) );
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitErrResponse( ErrorResponse& resp ) throw (PvapException) {
            PC_ERR_RESP msg(&resp);
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitDelResponse( DelResponse& resp ) throw (PvapException) {
            PC_DEL_RESP msg(&resp);
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitSetResponse( SetResponse& resp ) throw (PvapException) {
            PC_SET_RESP msg(&resp);
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitGetResponse( GetResponse& resp ) throw (PvapException) {
            PC_GET_RESP msg(&resp);
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitIncResponse( IncResponse& resp ) throw (PvapException) {
            PC_INC_RESP msg(&resp);
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitPingResponse( PingResponse& resp ) throw (PvapException) {
            PC_PING_RESP msg(&resp);
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitAuthResponse( AuthResponse& resp ) throw (PvapException) {
            PC_AUTH_RESP msg(&resp);
            pvap.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitBatchResponse( BatchResponse& resp ) throw (PvapException) {
            PC_BATCH_RESP msg(&resp);
            pvap.encodeMessage(msg,writer);
            return true;
        }

    private:
        BufferWriter writer;
        PVAP         pvap;
        Packet*      packet;
    };


    class Handler : public PVAP::Handler
    {
    public:
        Handler() : packet(0), pvap(this) {}
        
        Packet* decode( Buffer& buf ) throw(PvapException)
        {
            BufferReader reader(buf);
            pvap.decodeMessage(reader);
            return packet;
        }

        virtual bool hasSeqNum( uint32_t ) const { return true; }
        virtual void handle( PC_ERR_RESP& obj )     { packet = obj.pop(); }
        virtual void handle( PC_DEL& obj )          { packet = obj.pop(); }
        virtual void handle( PC_DEL_RESP& obj )     { packet = obj.pop(); }
        virtual void handle( PC_SET& obj )          { packet = obj.pop(); }
        virtual void handle( PC_SET_RESP& obj )     { packet = obj.pop(); }
        virtual void handle( PC_GET& obj )          { packet = obj.pop(); }
        virtual void handle( PC_GET_RESP& obj )     { packet = obj.pop(); }
        virtual void handle( PC_INC& obj )          { packet = obj.pop(); }
        virtual void handle( PC_INC_RESP& obj )     { packet = obj.pop(); }
        virtual void handle( PC_INC_MOD& obj )      { packet = obj.pop(); }
        virtual void handle( PC_INC_MOD_RESP& obj ) { packet = obj.pop(); }
        virtual void handle( PC_PING& obj )         { packet = obj.pop(); }
        virtual void handle( PC_PING_RESP& obj )    { packet = obj.pop(); }
        virtual void handle( PC_AUTH& obj )         { packet = obj.pop(); }
        virtual void handle( PC_AUTH_RESP& obj )    { packet = obj.pop(); }
        virtual void handle( PC_BATCH& obj )        { packet = obj.pop(); }
        virtual void handle( PC_BATCH_RESP& obj )   { packet = obj.pop(); }

    private:
        Packet* packet;
        PVAP    pvap;
    };

public:
    virtual ~PvapProtocol() {}
    
    /**
     * Serialize packet into the buffer.
     */
    virtual void serialize( const Packet& pack, Buffer& buf ) throw(PvapException) {
        BufferWriterVisitor visitor(buf);
        Packet& packet = const_cast<Packet&>(pack);
        if ( packet.isRequest() ) {
            static_cast< Request& >(packet).visit( visitor );
        } else {
            static_cast< Response& >(packet).visit( visitor );
        }
    }

    /**
     * NOTE: buf will be used only for reading, so it is safe to pass an extBuf.
     */
    virtual Packet* deserialize( Buffer& buf ) throw(PvapException) {
        Handler h;
        return h.decode(buf);
    }
    
private:
    PVAP pvap;
};

} // namespace pvap
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_PVAP_PVAPPROTOCOL_H */
