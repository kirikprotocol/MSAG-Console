#ifndef _SCAG_PVSS_PVAP_BATCHCMDARRAY_H
#define _SCAG_PVSS_PVAP_BATCHCMDARRAY_H

#include <vector>
#include "Exceptions.h"
#include "BC_CMD.h"
#include "PVAPBC.hpp"

namespace scag {
namespace pvss {
namespace pvap {

class BC_CMD;
class PVAP;

class BatchCmdArray
{
private:
    class BatchCmdArrayFiller : public PVAPBC::Handler
    {
    public:
        BatchCmdArrayFiller( BatchCmdArray& a ) : array_(&a) {}
        virtual bool hasSeqNum( uint32_t ) const { return true; }
        virtual void handle( std::auto_ptr<BC_DEL> obj ) { push(obj.release()); }
        virtual void handle( std::auto_ptr<BC_DEL_RESP> obj ) { push(obj.release()); }
        virtual void handle( std::auto_ptr<BC_SET> obj ) { push(obj.release()); }
        virtual void handle( std::auto_ptr<BC_SET_RESP> obj ) { push(obj.release()); }
        virtual void handle( std::auto_ptr<BC_GET> obj ) { push(obj.release()); }
        virtual void handle( std::auto_ptr<BC_GET_RESP> obj ) { push(obj.release()); }
        virtual void handle( std::auto_ptr<BC_INC> obj ) { push(obj.release()); }
        virtual void handle( std::auto_ptr<BC_INC_RESP> obj ) { push(obj.release()); }
        virtual void handle( std::auto_ptr<BC_INC_MOD> obj ) { push(obj.release()); }
        virtual void handle( std::auto_ptr<BC_INC_MOD_RESP> obj ) { push(obj.release()); }
    private:
        void push( BC_CMD* obj ) {
            array_->push(obj);
        }
    private:
        BatchCmdArray* array_;
    };

public:
    BatchCmdArray() {}

    // attention: this method would grab elements from a
    BatchCmdArray( const BatchCmdArray& a )
    {
        BatchCmdArray& that = const_cast<BatchCmdArray&>(a);
        commands_ = that.commands_;
        that.commands_.clear();
    }

    BatchCmdArray& operator = ( const BatchCmdArray& a )
    {
        if ( &a != this ) {
            BatchCmdArray& that = const_cast<BatchCmdArray&>(a);
            commands_ = that.commands_;
            that.commands_.clear();
        }
        return *this;
    }

    ~BatchCmdArray() {
        clear();
    }

    void clear() {
        for ( std::vector<BC_CMD*>::iterator i = commands_.begin();
              i != commands_.end(); ++i ) {
            delete *i;
        }
        commands_.clear();
    }

    /// gets owned
    void push( BC_CMD* cmd ) {
        commands_.push_back( cmd );
    }

    std::string toString() const {
        std::string rv = "[";
        bool comma = false;
        for ( std::vector<BC_CMD*>::const_iterator i = commands_.begin();
              i != commands_.end();
              ++i ) {
            if (comma) rv += ", ";
            else comma = true;
            rv += (*i)->toString();
        }
        rv += "]";
        return rv;
    }

    template <class DataStream> void serialize( const PVAP& proto, DataStream& writer ) const throw (PvapException)
    {
        PVAPBC subproto;
        const uint16_t sz = size();
        writer.writeInt16( sz );
        DataStream subwriter;
        for ( std::vector< BC_CMD* >::const_iterator i = commands_.begin();
              i != commands_.end();
              ++i ) {
            subwriter.clear();
            subproto.encodeCastMessage( **i, subwriter );
            subwriter.write(writer);
        }
    }

    template <class DataStream> void deserialize( const PVAP& proto, DataStream& reader ) throw (PvapException)
    {
        clear();
        BatchCmdArrayFiller filler(*this);
        PVAPBC subproto( &filler );
        const uint16_t cmdCount = reader.readInt16();
        for ( uint16_t i = 0; i < cmdCount; ++i ) {
            DataStream subreader;
            subreader.read(reader);
            subproto.decodeMessage( subreader );
        }
    }

    inline uint16_t size() const {
        return uint16_t(commands_.size());
    }

    BC_CMD* getCommand( uint16_t i ) {
        return ( i < size() ? commands_[i] : 0 );
    }
    const BC_CMD* getCommand( uint16_t i ) const {
        return const_cast<BatchCmdArray*>(this)->getCommand(i);
    }

private:
    std::vector< BC_CMD* > commands_; // owned
};

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif /* ! _SCAG_PVSS_PVAP_BATCHCMDARRAY_H */
