#include "ProfileResponseFiller.h"
#include "scag/pvss/api/packets/SerializedCommandResponse.h"
#include "scag/pvss/api/pvap/generated-cpp/PVAP.hpp"
#include "scag/pvss/api/pvap/generated-cpp/PVAPPROF.hpp"

namespace {
using namespace scag2::pvss::pvap;
using namespace scag2::pvss;

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
    virtual bool visitGetProfileResponse( GetProfileResponse& cmd ) throw (PvapException) {
        BC_GETPROF_RESP msg(&cmd);
        pvapbc.encodeMessage(msg,writer);
        return true;
    }
    
    const BufferWriter& getWriter() const { return writer; }
private:
    Protocol::Buffer buffer;
    BufferWriter     writer;
    PVAPPROF         pvapbc;
};


class Handler : public PVAPPROF::Handler
{
public:
    Handler( ProfileResponseFiller& filler ) : filler_(filler) {}
    bool hasSeqNum( uint32_t ) const { return true; }
    void handle( BC_DEL& object )          { filler_.fail(); }
    void handle( BC_DEL_RESP& object )     { filler_.push(object.pop()); }
    void handle( BC_SET& object )          { filler_.fail(); }
    void handle( BC_SET_RESP& object )     { filler_.push(object.pop()); }
    void handle( BC_GET& object )          { filler_.fail(); }
    void handle( BC_GET_RESP& object )     { filler_.push(object.pop()); }
    void handle( BC_INC& object )          { filler_.fail(); }
    void handle( BC_INC_RESP& object )     { filler_.push(object.pop()); }
    void handle( BC_INC_MOD& object )      { filler_.fail(); }
    void handle( BC_BATCH& object )        { filler_.fail(); }
    void handle( BC_BATCH_RESP& object )   { filler_.push(object.pop()); }
    void handle( BC_GETPROF& object )      { filler_.fail(); }
    void handle( BC_GETPROF_RESP& object ) { filler_.push(object.pop()); }
private:
    ProfileResponseFiller& filler_;
};

}


namespace scag2 {
namespace pvss {
namespace pvap {

void ProfileResponseFiller::serialize( const PVAP&, BufferWriter& writer ) const throw (PvapException)
{
    if (!owner_->getResponse())
        throw PvapSerializationException( owner_->isRequest(),
                                          owner_->getSeqNum(),
                                          "command response is not attached" );
    BufferFiller filler;
    if (!owner_->getResponse()->visit(filler)) {
        SerializedCommandResponse* spc = dynamic_cast<SerializedCommandResponse*>(owner_->getResponse());
        if ( spc ) {
            Protocol::Buffer buf;
            spc->setupBuffer(buf);
            BufferWriter bw(buf);
            writer.write(bw);
        } else {
            throw PvapSerializationException( owner_->isRequest(), 
                                              owner_->getSeqNum(),
                                              "unknown command response: %s",
                                              owner_->getResponse()->toString().c_str() );
        }
    } else {
        writer.write(filler.getWriter());
    }
}


void ProfileResponseFiller::deserialize( PVAP& pvap, BufferReader& reader ) throw (PvapException)
{
    try {
        BufferReader subreader;
        reader.read( subreader );
        if ( (pvap.getOptions() & Protocol::PASSBUFFER) != 0 ) {
            owner_->setResponse( new SerializedCommandResponse(subreader.getBuffer()) );
        } else {
            ::Handler h(*this);
            PVAPPROF subproto(&h);
            subproto.decodeMessage( subreader );
        }
    } catch ( exceptions::IOException& e ) {
        throw PvapSerializationException( owner_->isRequest(),
                                          owner_->getSeqNum(),
                                          "decoding command response: %s",
                                          e.what() );
    }
}

} // namespace pvap
} // namespace pvss
} // namespace scag
