#ifndef _SCAG_PVSS_PVAP_PROFILECOMMANDFILLER_H
#define _SCAG_PVSS_PVAP_PROFILECOMMANDFILLER_H

#include <cassert>
#include "Exceptions.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/ProfileCommand.h"
// #include "scag/pvss/api/pvap/generated-cpp/PVAPPROF.hpp"

namespace scag2 {
namespace pvss {
namespace pvap {

class PVAP;
class BufferWriter;
class BufferReader;

class ProfileCommandFiller
{
private:
    /*
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
    ProfileCommandFiller( ProfileRequest* owner ) : owner_(owner) {
        assert(owner);
    }

    void serialize( const PVAP&, BufferWriter& writer ) const throw (PvapException);
    /*
    {
        if (!owner->getCommand())
            throw PvapSerializationException( owner->isRequest(),
                                              owner->getSeqNum(),
                                              "profile command is not attached" );
        BufferFiller filler;
        if (!owner->getCommand()->visit(filler)) {
            throw PvapSerializationException( owner->isRequest(), 
                                              owner->getSeqNum(),
                                              "unknown profile command: %s",
                                              owner->getCommand()->toString().c_str() );
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
                                              "decoding profile command: %s",
                                              e.what() );
        }
    }
     */

    void push( ProfileCommand* cmd ) {
        owner_->setCommand(cmd);
    }
    void fail() throw(PvapException)
    {
        throw PvapSerializationException( owner_->isRequest(),
                                          owner_->getSeqNum(),
                                          "command response is not allowed in profilerequest" );
    }

protected:

    /*
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
    void handle( BC_BATCH& object )        { push(object.pop()); }
    void handle( BC_BATCH_RESP& object )   { fail(); }
     */


private:
    ProfileRequest* owner_;  // not owned
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif
