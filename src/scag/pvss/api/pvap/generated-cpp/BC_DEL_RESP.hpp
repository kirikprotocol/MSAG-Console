// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_BC_DEL_RESP_HPP__
#define __SCAG_PVSS_PVAP_BC_DEL_RESP_HPP__

#include "util/int.h"
#include <string>
#include "scag/pvss/api/pvap/Exceptions.h"

#include "scag/pvss/api/packets/DelResponse.h"





namespace scag2{
namespace pvss{
namespace pvap{

class PVAPPROF;

class BC_DEL_RESP
{
protected:
    static const int statusValueTag = 1;

public:
    BC_DEL_RESP() :
    owned_(true),
    data_(new DelResponse)
    {
    }

    BC_DEL_RESP( DelResponse* other ) :
    owned_(false),
    data_(other)
    {
    }

    ~BC_DEL_RESP() {
        if ( data_ && owned_ ) delete data_;
    }

    void clear()
    {
        if ( ! data_ ) return;
        data_->clear();
    }

    DelResponse* pop() {
        DelResponse* rv = data_;
        data_ = 0;
        return rv;
    }

    std::string toString() const
    {
        return data_ ? data_->toString() : "";
    }

    template < class DataStream >
        void serialize( const PVAPPROF& proto, DataStream& writer ) const throw (PvapException)
    {
        if ( ! data_ ) return;
        checkFields();
        // mandatory fields
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), statusValueTag );
            writer.writeTag(statusValueTag);
            writer.writeByteLV(data_->getStatusValue());
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              getSeqNum(),
                                              "writing field statusValue in BC_DEL_RESP: %s",
                                              e.what() );
        }
        // optional fields
    }

    template <class DataStream> void deserialize( PVAPPROF& proto, DataStream& reader )
        throw (PvapException)
    {
        if ( ! data_ ) return;
        clear();
        int tag = -1;
        try {
            do {
                // int pos = int(reader.getPos());
                tag = reader.readTag();
                // printf( "read pos=%d field=%d\n", pos, tag );
                if ( tag == -1 ) break;
                switch(tag) {
                case statusValueTag: {
                    data_->setStatusValue(reader.readByteLV());
                    break;
                }
                default:
                    throw InvalidFieldTypeException(data_->isRequest(),"BC_DEL_RESP", getSeqNum(),tag);
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              getSeqNum(),
                                              "reading field tag=%d of BC_DEL_RESP: %s",
                                              tag, e.what() );
        }
        checkFields();
    }

    uint32_t getSeqNum() const {
        return
        uint32_t(-1);
    }


protected:
    void checkFields() const throw (PvapException)
    {
        // using parent check
        if ( !data_->isValid() ) {
            throw MessageIsBrokenException(data_->isRequest(), getSeqNum(), "message BC_DEL_RESP is broken: %s",data_->toString().c_str());
        }
    }

private:
    BC_DEL_RESP( const BC_DEL_RESP& );
    BC_DEL_RESP& operator = ( const BC_DEL_RESP& );

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    bool                owned_;
    DelResponse* data_;

};

} // namespace scag2
} // namespace pvss
} // namespace pvap
#endif
