#include "BatchCommandFiller.h"
#include "BufferWriter.h"
#include "BufferReader.h"
#include "Exceptions.h"
// #include "scag/pvss/api/packets/ProfileCommandVisitor.h"
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
        return false;
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


class Handler : public PVAPPROF::Handler
{
public:
    Handler( BatchCommandFiller& filler ) : filler_(filler) {}
    // bool hasSeqNum( uint32_t ) const { return true; }
    void handle( BC_DEL& object )          { filler_.push(object.pop()); }
    void handle( BC_DEL_RESP& object )     { filler_.fail(); }
    void handle( BC_SET& object )          { filler_.push(object.pop()); }
    void handle( BC_SET_RESP& object )     { filler_.fail(); }
    void handle( BC_GET& object )          { filler_.push(object.pop()); }
    void handle( BC_GET_RESP& object )     { filler_.fail(); }
    void handle( BC_INC& object )          { filler_.push(object.pop()); }
    void handle( BC_INC_RESP& object )     { filler_.fail(); }
    void handle( BC_INC_MOD& object )      { filler_.push(object.pop()); }
    void handle( BC_BATCH& object )        { filler_.fail(); }
    void handle( BC_BATCH_RESP& object )   { filler_.fail(); }
    void handle( BC_GETPROF& object )      { filler_.fail(); }
    void handle( BC_GETPROF_RESP& object ) { filler_.fail(); }
private:
    BatchCommandFiller& filler_;
};

}


namespace scag2 {
namespace pvss {
namespace pvap {

void BatchCommandFiller::serialize( const PVAPPROF&, BufferWriter& writer ) const throw (PvapException)
{
    const std::vector< BatchRequestComponent* >& components = owner_->getBatchContent();
    const size_t cnt = components.size();
    if ( cnt > 0xffff ) {
        throw PvapSerializationException( true,
                                          uint32_t(-1),
                                          "cannot write more than %d components",
                                          0xffff );
    }
    writer.writeShort( short(cnt) );
    int idx = 0;
    BufferFiller filler;
    for ( std::vector< BatchRequestComponent* >::const_iterator i = components.begin();
          i != components.end();
          ++i ) {
        filler.clear();
        BatchRequestComponent* comp = const_cast<BatchRequestComponent*>(*i);
        if ( ! comp->visit( filler ) ) {
            throw PvapSerializationException( owner_->isRequest(), 
                                              uint32_t(-1),
                                              "unknown batch request component #%d: %s",
                                              idx,
                                              comp->toString().c_str() );
        }
        // printf( "batch component #%d :%s\n", idx, filler.getWriter().dump().c_str() );
        writer.write( filler.getWriter() );
        // printf( "full writer dump: %s\n", writer.dump().c_str() );
        ++idx;
    }
}


void BatchCommandFiller::deserialize( PVAPPROF&, BufferReader& reader ) throw (PvapException)
{
    int idx = -1;
    try {
        int cnt = reader.readShort();
        BufferReader subreader;
        // printf( "full reader (pos=%d) dump:%s\n", reader.getPos(), reader.dump().c_str() );
        ::Handler h(*this);
        PVAPPROF subproto(&h);
        for ( idx = 0; idx < cnt; ++idx ) {
            reader.read( subreader );
            // printf( "batch component #%d (pos=%d) dump: %s\n",
            // idx, subreader.getPos(), subreader.dump().c_str() );
            subproto.decodeMessage(subreader);
        }
    } catch ( exceptions::IOException& e ) {
        throw PvapSerializationException( owner_->isRequest(),
                                          uint32_t(-1),
                                          "decoding batch request component #%d: %s",
                                          idx, e.what() );
    }
}

} // namespace pvap
} // namespace pvss
} // namespace scag
