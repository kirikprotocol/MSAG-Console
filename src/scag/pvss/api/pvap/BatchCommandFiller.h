#ifndef _SCAG_PVSS_PVAP_BATCHCOMMANDFILLER_H
#define _SCAG_PVSS_PVAP_BATCHCOMMANDFILLER_H

#include <cassert>
#include <vector>
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/api/pvap/generated-cpp/PVAPBC.hpp"

namespace scag2 {
namespace pvss {
namespace pvap {

class PVAP;

class BatchCommandFiller : public PVAPBC::Handler
{
private:
    class BufferFiller : public ProfileCommandVisitor
    {
    public:
        BufferFiller() : writer(buffer) {}

        virtual bool visitDelCommand( DelCommand& cmd ) throw(PvapException) {
            BC_DEL msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitSetCommand( SetCommand& cmd ) throw(PvapException) {
            BC_SET msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitGetCommand( GetCommand& cmd ) throw(PvapException) {
            BC_GET msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitIncCommand( IncCommand& cmd ) throw(PvapException) {
            BC_INC msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitIncModCommand( IncModCommand& cmd ) throw(PvapException) {
            BC_INC_MOD msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitBatchCommand( BatchCommand& cmd ) throw(PvapException) {
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
    BatchCommandFiller( BatchCommand& theOwner ) : owner(theOwner) {}
    BatchCommandFiller( BatchCommand* theOwner ) : owner(*theOwner) {
        assert(theOwner);
    }

    void serialize( const PVAP&, BufferWriter& writer ) const throw (PvapException)
    {
        const std::vector< BatchRequestComponent* >& components = owner.getBatchContent();
        int cnt = components.size();
        if ( cnt > 0xffff ) {
            throw PvapSerializationException( true,
                                              owner.getSeqNum(),
                                              "cannot write more than %d components",
                                              0xffff );
        }
        writer.writeShort( (short) cnt );
        int idx = 0;
        BufferFiller filler;
        for ( std::vector< BatchRequestComponent* >::const_iterator i = components.begin();
              i != components.end();
              ++i ) {
            filler.clear();
            BatchRequestComponent* comp = const_cast<BatchRequestComponent*>(*i);
            if ( ! comp->visit( filler ) ) {
                throw PvapSerializationException( owner.isRequest(), 
                                                  owner.getSeqNum(),
                                                  "unknown batch request component #%d: %s",
                                                  idx,
                                                  comp->toString().c_str() );
            }
            printf( "batch component #%d :%s\n", idx, filler.getWriter().dump().c_str() );
            writer.write( filler.getWriter() );
            printf( "full writer dump: %s\n", writer.dump().c_str() );
            ++idx;
        }
    }

    void deserialize( PVAP&, BufferReader& reader ) throw (PvapException)
    {
        int idx = -1;
        try {
            int cnt = reader.readShort();
            BufferReader subreader;
            printf( "full reader (pos=%d) dump:%s\n", reader.getPos(), reader.dump().c_str() );
            PVAPBC subproto(this);
            for ( idx = 0; idx < cnt; ++idx ) {
                reader.read( subreader );
                printf( "batch component #%d (pos=%d) dump: %s\n",
                        idx, subreader.getPos(), subreader.dump().c_str() );
                subproto.decodeMessage( subreader );
            }
        } catch ( exceptions::IOException& e ) {
            throw PvapSerializationException( owner.isRequest(),
                                              owner.getSeqNum(),
                                              "decoding batch request component #%d: %s",
                                              idx, e.what() );
        }
    }

protected:
    bool hasSeqNum( uint32_t ) const { return true; }
    void handle( BC_DEL& object )          { push(object.pop()); }
    void handle( BC_DEL_RESP& object )     { fail(); }
    void handle( BC_SET& object )          { push(object.pop()); }
    void handle( BC_SET_RESP& object )     { fail(); }
    void handle( BC_GET& object )          { push(object.pop()); }
    void handle( BC_GET_RESP& object )     { fail(); }
    void handle( BC_INC& object )          { push(object.pop()); }
    void handle( BC_INC_RESP& object )     { fail(); }
    void handle( BC_INC_MOD& object )      { push(object.pop()); }
    void handle( BC_INC_MOD_RESP& object ) { fail(); }

    void push( BatchRequestComponent* comp ) {
        owner.addComponent( comp );
    }
    void fail() throw(PvapException)
    {
        throw PvapSerializationException( true,
                                          owner.getSeqNum(),
                                          "cannot decode response in batch content" );
    }

private:
    BatchCommand& owner;
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif
