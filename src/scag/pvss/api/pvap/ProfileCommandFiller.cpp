#include "ProfileCommandFiller.h"
#include "scag/pvss/api/packets/SerializedProfileCommand.h"
#include "scag/pvss/api/pvap/generated-cpp/PVAP.hpp"
#include "scag/pvss/api/pvap/generated-cpp/PVAPPROF.hpp"

namespace {
using namespace scag2::pvss::pvap;
using namespace scag2::pvss;

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
        BC_BATCH msg(&cmd);
        pvapbc.encodeMessage(msg,writer);
        return true;
    }
    
    virtual bool visitGetProfileCommand( GetProfileCommand& cmd ) throw (PvapException) {
        BC_GETPROF msg(&cmd);
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
    Handler( ProfileCommandFiller& filler ) : filler_(filler) {}
    bool hasSeqNum( uint32_t ) const { return true; }
    void handle( BC_DEL& object )          { filler_.push(object.pop()); }
    void handle( BC_DEL_RESP& object )     { filler_.fail(); }
    void handle( BC_SET& object )          { filler_.push(object.pop()); }
    void handle( BC_SET_RESP& object )     { filler_.fail(); }
    void handle( BC_GET& object )          { filler_.push(object.pop()); }
    void handle( BC_GET_RESP& object )     { filler_.fail(); }
    void handle( BC_INC& object )          { filler_.push(object.pop()); }
    void handle( BC_INC_RESP& object )     { filler_.fail(); }
    void handle( BC_INC_MOD& object )      { filler_.push(object.pop()); }
    void handle( BC_BATCH& object )        { filler_.push(object.pop()); }
    void handle( BC_BATCH_RESP& object )   { filler_.fail(); }
    void handle( BC_GETPROF& object )      { filler_.push(object.pop()); }
    void handle( BC_GETPROF_RESP& object ) { filler_.fail(); }
private:
    ProfileCommandFiller& filler_;
};

}


namespace scag2 {
namespace pvss {
namespace pvap {

void ProfileCommandFiller::serialize( const PVAP&, BufferWriter& writer ) const throw (PvapException)
{
    if (!owner_->getCommand())
        throw PvapSerializationException( owner_->isRequest(),
                                          owner_->getSeqNum(),
                                          "command response is not attached" );
    BufferFiller filler;
    if (!owner_->getCommand()->visit(filler)) {
        /// db: it may be ugly to use dynamic_cast here but still it is as it is.
        SerializedProfileCommand* spc = dynamic_cast<SerializedProfileCommand*>(owner_->getCommand());
        if ( spc ) {
            Protocol::Buffer buf;
            spc->setupBuffer(buf);
            BufferWriter bw(buf);
            writer.write(bw);
        } else {
            throw PvapSerializationException( owner_->isRequest(),
                                              owner_->getSeqNum(),
                                              "unknown command response: %s",
                                              owner_->getCommand()->toString().c_str() );
        }
    } else {
        writer.write(filler.getWriter());
    }
}


void ProfileCommandFiller::deserialize( PVAP& pvap, BufferReader& reader ) throw (PvapException)
{
    try {
        BufferReader subreader;
        reader.read( subreader );
        if ( (pvap.getOptions() & Protocol::PASSBUFFER) != 0 ) {
            // pass buffer as is
            owner_->setCommand( new SerializedProfileCommand(subreader.getBuffer()) );
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
