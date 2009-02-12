// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_INC_RESP_HPP__
#define __SCAG_PVSS_PVAP_PC_INC_RESP_HPP__

#include "util/int.h"
#include <string>
#include "scag/pvss/pvap/Exceptions.h"


#include "scag/pvss/packets/IncResponse.h"





namespace scag2{
namespace pvss{
namespace pvap{

class PVAP;

class PC_INC_RESP
{
protected:
    static const int statusValueTag = 1;
    static const int resultTag = 25;

public:
    PC_INC_RESP( int seqNum ) :
    owned_(true),
    data_(new IncResponse(seqNum))
    {
    }

    PC_INC_RESP( IncResponse* other ) :
    owned_(false),
    data_(other)
    {
    }

    ~PC_INC_RESP() {
        if ( data_ && owned_ ) delete data_;
    }

    void clear()
    {
        if ( ! data_ ) return;
        data_->clear();
    }

    IncResponse* pop() {
        IncResponse* rv = data_;
        data_ = 0;
        return rv;
    }

    std::string toString() const
    {
        return data_ ? data_->toString() : "";
    }

    template < class DataStream >
        void serialize( const PVAP& proto, DataStream& writer ) const throw (PvapException)
    {
        if ( ! data_ ) return;
        checkFields();
        // mandatory fields
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), statusValueTag );
            writer.writeTag(statusValueTag);
            writer.writeByteLV(data_->getStatusValue());
        } catch ( IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              data_->getSeqNum(),
                                              "writing field statusValue in PC_INC_RESP: %s",
                                              e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), resultTag );
            writer.writeTag(resultTag);
            writer.writeIntLV(data_->getResult());
        } catch ( IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              data_->getSeqNum(),
                                              "writing field result in PC_INC_RESP: %s",
                                              e.what() );
        }
        // optional fields
    }

    template <class DataStream> void deserialize( PVAP& proto, DataStream& reader )
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
                case resultTag: {
                    data_->setResult(reader.readIntLV());
                    break;
                }
                default:
                    throw InvalidFieldTypeException(data_->isRequest(),"invalid field in PC_INC_RESP", data_->getSeqNum(),tag);
                }
            } while ( true );
        } catch ( IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              data_->getSeqNum(),
                                              "reading field tag=%d of PC_INC_RESP: %s",
                                              tag, e.what() );
        }
        checkFields();
    }

    uint32_t getSeqNum() const {
        return data_ ? data_->getSeqNum() : uint32_t(-1);
    }

protected:
    void checkFields() const throw (PvapException)
    {
        // using parent check
        if ( !data_->isValid() ) {
            throw MessageIsBrokenException(data_->isRequest(), "message PC_INC_RESP is broken: " + toString(), data_->getSeqNum() );
        }
    }

private:
    PC_INC_RESP( const PC_INC_RESP& );
    PC_INC_RESP& operator = ( const PC_INC_RESP& );

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    bool                owned_;
    IncResponse* data_;

};

} // namespace scag2
} // namespace pvss
} // namespace pvap
#endif
