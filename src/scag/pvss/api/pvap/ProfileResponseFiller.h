#ifndef _SCAG_PVSS_PVAP_PROFILERESPONSEFILLER_H
#define _SCAG_PVSS_PVAP_PROFILERESPONSEFILLER_H

#include <cassert>
#include "Exceptions.h"
#include "scag/pvss/api/packets/ProfileResponse.h"
#include "scag/pvss/api/packets/CommandResponse.h"
// #include "scag/pvss/api/pvap/generated-cpp/PVAPPROF.hpp"

namespace scag2 {
namespace pvss {
namespace pvap {

class PVAP;
class BufferWriter;
class BufferReader;

class ProfileResponseFiller
{
private:
    /*
    class BufferFiller : public ProfileResponseVisitor
    {
    public:
        BufferFiller() : writer(buffer) {}

        virtual bool visitDelResponse( DelResponse& cmd ) throw(PvapException) {
            BC_DEL_RESP msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitSetResponse( SetResponse& cmd ) throw(PvapException) {
            BC_SET_RESP msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitGetResponse( GetResponse& cmd ) throw(PvapException) {
            BC_GET_RESP msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitIncResponse( IncResponse& cmd ) throw(PvapException) {
            BC_INC_RESP msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }
        virtual bool visitBatchResponse( BatchResponse& cmd ) throw(PvapException) {
            BC_BATCH_RESP msg(&cmd);
            pvapbc.encodeMessage(msg,writer);
            return true;
        }

        void clear() {
            buffer.SetPos(0);
        }

        const BufferWriter& getWriter() const { return writer; }
    private:
        Protocol::Buffer buffer;
        BufferWriter     writer;
        PVAPPROF         pvapbc;
    };
     */


public:
    ProfileResponseFiller( ProfileResponse* owner ) : owner_(owner) {
        assert(owner);
    }


    void serialize( const PVAP&, BufferWriter& writer ) const throw (PvapException);
    /*
    {
        if (!owner->getResponse())
            throw PvapSerializationException( owner->isRequest(),
                                              owner->getSeqNum(),
                                              "command response is not attached" );
        BufferFiller filler;
        if (!owner->getResponse()->visit(filler)) {
            throw PvapSerializationException( owner->isRequest(), 
                                              owner->getSeqNum(),
                                              "unknown command response: %s",
                                              owner->getResponse()->toString().c_str() );
        }
        writer.write(filler.getWriter());
    }
     */


    void deserialize( PVAP& pvap, BufferReader& reader ) throw (PvapException);
    /*
    {
        try {
            BufferReader subreader;
            PVAPPROF subproto(this);
            reader.read( subreader );
            subproto.decodeMessage( subreader );
        } catch ( exceptions::IOException& e ) {
            throw PvapSerializationException( owner->isRequest(),
                                              owner->getSeqNum(),
                                              "decoding command response: %s",
                                              e.what() );
        }
    }
     */

    void push( CommandResponse* cmd ) {
        owner_->setResponse(cmd);
    }
    void fail() throw(PvapException)
    {
        throw PvapSerializationException( owner_->isRequest(),
                                          owner_->getSeqNum(),
                                          "command request is not allowed in profileresponse" );
    }

protected:

    /*
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
    // void handle( BC_INC_MOD_RESP& object ) { push(object.pop()); }
    void handle( BC_BATCH& object )        { fail(); }            
    void handle( BC_BATCH_RESP& object )   { push(object.pop()); }
     */

private:
    ProfileResponse* owner_;  // not owned
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif
