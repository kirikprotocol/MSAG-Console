#ifndef _SCAG_PVSS_PVAP_BATCHRESPONSEFILLER_H
#define _SCAG_PVSS_PVAP_BATCHRESPONSEFILLER_H

#include <cassert>
#include <vector>
#include "scag/pvss/api/pvap/generated-cpp/PVAPBC.hpp"
#include "scag/pvss/api/packets/BatchResponse.h"

namespace scag2 {
namespace pvss {
namespace pvap {

class PVAP;

class BatchResponseFiller : public PVAPBC::Handler
{
private:
    class BufferFiller : public ResponseVisitor
    {
    public:
        BufferFiller() : writer(buffer) {}

        virtual bool visitErrResponse( ErrorResponse& resp ) throw (PvapException) {
            return false;
        }

        virtual bool visitDelResponse( DelResponse& resp ) throw (PvapException) {
            BC_DEL_RESP msg(&resp);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitSetResponse( SetResponse& resp ) throw (PvapException) {
            BC_SET_RESP msg(&resp);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitGetResponse( GetResponse& resp ) throw (PvapException) {
            BC_GET_RESP msg(&resp);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitIncResponse( IncResponse& resp ) throw (PvapException) {
            BC_INC_RESP msg(&resp);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitPingResponse( PingResponse& resp ) throw (PvapException) {
            return false;
        }
        virtual bool visitAuthResponse( AuthResponse& resp ) throw (PvapException) {
            return false;
        }
        virtual bool visitBatchResponse( BatchResponse& resp ) throw (PvapException) {
            return false;
        }
        void clear() {
            buffer.SetPos(0);
        }

        const BufferWriter& getWriter() const { return writer; }
    private:
        Protocol::Buffer buffer;
        BufferWriter     writer;
        PVAPBC           pvapbc;
    };

public:
    BatchResponseFiller( BatchResponse& theOwner ) : owner(theOwner) {}
    BatchResponseFiller( BatchResponse* theOwner ) : owner(*theOwner) {
        assert(theOwner);
    }

    void serialize( const PVAP&, BufferWriter& writer ) const throw (PvapException)
    {
        const std::vector< BatchResponseComponent* >& components = owner.getBatchContent();
        int cnt = components.size();
        if ( cnt > 0xffff ) {
            throw PvapSerializationException( true, "cannot write more than 65536 components", owner.getSeqNum() );
        }
        writer.writeShort( (short) cnt );
        int idx = 0;
        BufferFiller filler;
        for ( std::vector< BatchResponseComponent* >::const_iterator i = components.begin();
              i != components.end();
              ++i ) {
            filler.clear();
            BatchResponseComponent* comp = const_cast<BatchResponseComponent*>(*i);
            if ( ! comp->visit(filler) ) {
                throw PvapSerializationException( owner.isRequest(),
                                                  owner.getSeqNum(),
                                                  "unknown batch response component #%d: %s",
                                                  idx, comp->toString().c_str() );
            }
            writer.write( filler.getWriter() );
            ++idx;
        }
    }

    void deserialize( PVAP&, BufferReader& reader ) throw (PvapException)
    {
        int idx = -1;
        try {
            int cnt = reader.readShort();
            BufferReader subreader;
            PVAPBC subproto(this);
            for ( idx = 0; idx < cnt; ++idx ) {
                reader.read( subreader );
                subproto.decodeMessage( subreader );
            }
        } catch ( exceptions::IOException& e ) {
            throw PvapSerializationException( owner.isRequest(),
                                              owner.getSeqNum(),
                                              "decoding batch_resp component #%d: %s",
                                              idx, e.what() );
        }
    }

protected:
    bool hasSeqNum( uint32_t ) const { return true; }
    void handle( BC_DEL& object )          { fail(); }                
    void handle( BC_DEL_RESP& object )     { push(object.pop()); }
    void handle( BC_SET& object )          { fail(); }                
    void handle( BC_SET_RESP& object )     { push(object.pop()); }
    void handle( BC_GET& object )          { fail(); }                
    void handle( BC_GET_RESP& object )     { push(object.pop()); }
    void handle( BC_INC& object )          { fail(); }                
    void handle( BC_INC_RESP& object )     { push(object.pop()); }
    void handle( BC_INC_MOD& object )      { fail(); }                
    void handle( BC_INC_MOD_RESP& object ) { push(object.pop()); }

    void push( BatchResponseComponent* comp ) {
        owner.addComponent( comp );
    }
    void fail() throw(PvapException)
    {
        throw new PvapSerializationException( false,
                                              owner.getSeqNum(),
                                              "cannot decode request as batch_resp content" );
    }

private:
    BatchResponse& owner;
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif
