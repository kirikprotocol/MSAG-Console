#include "GetProfileResponseFiller.h"
#include "scag/pvss/api/pvap/generated-cpp/PVAPPROF.hpp"
#include "scag/pvss/api/pvap/generated-cpp/PVAPGP.hpp"

namespace {
using namespace scag2::pvss;
using namespace scag2::pvss::pvap;

class BufferFiller : public ProfileResponseVisitor
{
public:
    BufferFiller() : writer(buffer) {}

    virtual bool visitDelResponse( DelResponse& cmd ) throw(PvapException) {
        /*
        BC_DEL_RESP msg(&cmd);
        pvapbc.encodeMessage(msg,writer);
         */
        return false;
    }
    virtual bool visitSetResponse( SetResponse& cmd ) throw(PvapException) {
        /*
        BC_SET_RESP msg(&cmd);
        pvapbc.encodeMessage(msg,writer);
         */
        return false;
    }
    virtual bool visitGetResponse( GetResponse& cmd ) throw(PvapException) {
        /*
        BC_GET_RESP msg(&cmd);
        pvapbc.encodeMessage(msg,writer);
         */
        return false;
    }
    virtual bool visitIncResponse( IncResponse& cmd ) throw(PvapException) {
        /*
        BC_INC_RESP msg(&cmd);
        pvapbc.encodeMessage(msg,writer);
         */
        return false;
    }
    virtual bool visitBatchResponse( BatchResponse& cmd ) throw(PvapException) {
        return false;
    }
    virtual bool visitGetProfileResponse( GetProfileResponse& cmd ) throw (PvapException) {
        BC_GETPROF_RESP msg(&cmd);
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


class Handler : public PVAPGP::Handler
{
public:
    Handler( GetProfileResponseFiller& filler ) : filler_(filler) {}
    bool hasSeqNum( uint32_t ) const { return true; }
    void handle( GP_RESP& object ) { filler_.push(object.pop()); }
private:
    GetProfileResponseFiller& filler_;
};

}


namespace scag2 {
namespace pvss {
namespace pvap {

void GetProfileResponseFiller::serialize( const PVAPPROF&, BufferWriter& writer ) const // throw (PvapException)
{
    const std::vector< GetProfileResponseComponent* >& components = owner_->getContent();
    const size_t cnt = components.size();
    if ( cnt > 0xffff ) {
        throw PvapSerializationException( true,
                                          uint32_t(-1),
                                          "cannot write more than %d components",
                                          0xffff );
    }
    writer.writeShort( short(cnt) );
    int idx = 0;
    // BufferFiller filler;
    Protocol::Buffer buf;
    BufferWriter subwr(buf);
    PVAPGP subproto;
    for ( std::vector< GetProfileResponseComponent* >::const_iterator i = components.begin();
          i != components.end();
          ++i ) {
        // filler.clear();
        buf.SetPos(0);
        GetProfileResponseComponent* comp = *i;
        GP_RESP msg(comp);
        try {
            subproto.encodeMessage(msg,subwr);
        } catch (...) {
            throw PvapSerializationException( owner_->isRequest(), 
                                              uint32_t(-1),
                                              "unknown get_prof response component #%d: %s",
                                              idx,
                                              comp->toString().c_str() );
        }
        // printf( "batch component #%d :%s\n", idx, filler.getWriter().dump().c_str() );
        writer.write( subwr );
        // printf( "full writer dump: %s\n", writer.dump().c_str() );
        ++idx;
    }
}


void GetProfileResponseFiller::deserialize( PVAPPROF&, BufferReader& reader ) // throw (PvapException)
{
    int idx = -1;
    try {
        int cnt = reader.readShort();
        BufferReader subreader;
        // printf( "full reader (pos=%d) dump:%s\n", reader.getPos(), reader.dump().c_str() );
        ::Handler h(*this);
        PVAPGP subproto(&h);
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
