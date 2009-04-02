// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_AUTH_RESP_HPP__
#define __SCAG_PVSS_PVAP_PC_AUTH_RESP_HPP__

#include "util/int.h"
#include <string>
#include "scag/pvss/api/pvap/Exceptions.h"



#include "scag/pvss/api/packets/AuthResponse.h"





namespace scag2{
namespace pvss{
namespace pvap{

class PVAP;

class PC_AUTH_RESP
{
protected:
    static const int statusTag = 1;
    static const int clientTypeTag = 35;
    static const int sidTag = 36;

public:
    PC_AUTH_RESP() :
    owned_(true),
    data_(new AuthResponse)
    {
    }

    PC_AUTH_RESP( AuthResponse* other ) :
    owned_(false),
    data_(other)
    {
    }

    ~PC_AUTH_RESP() {
        if ( data_ && owned_ ) delete data_;
    }

    void clear()
    {
        if ( ! data_ ) return;
        data_->clear();
    }

    AuthResponse* pop() {
        AuthResponse* rv = data_;
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
            // printf( "write pos=%d field=%d\n", ds.getPos(), statusTag );
            writer.writeTag(statusTag);
            writer.writeByteLV(data_->getStatus());
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              getSeqNum(),
                                              "writing field status in PC_AUTH_RESP: %s",
                                              e.what() );
        }
        // optional fields
        if ( data_->hasClientType() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), clientTypeTag );
            try {
                writer.writeTag(clientTypeTag);
                writer.writeByteLV(data_->getClientType());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  getSeqNum(),
                                                  "writing field clientType in PC_AUTH_RESP:",
                                                  e.what() );
            }
        }
        if ( data_->hasSid() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), sidTag );
            try {
                writer.writeTag(sidTag);
                writer.writeByteLV(data_->getSid());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  getSeqNum(),
                                                  "writing field sid in PC_AUTH_RESP:",
                                                  e.what() );
            }
        }
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
                case statusTag: {
                    data_->setStatus(reader.readByteLV());
                    break;
                }
                case clientTypeTag: {
                    data_->setClientType(reader.readByteLV());
                    break;
                }
                case sidTag: {
                    data_->setSid(reader.readByteLV());
                    break;
                }
                default:
                    throw InvalidFieldTypeException(data_->isRequest(),"PC_AUTH_RESP", getSeqNum(),tag);
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              getSeqNum(),
                                              "reading field tag=%d of PC_AUTH_RESP: %s",
                                              tag, e.what() );
        }
        checkFields();
    }

    uint32_t getSeqNum() const {
        return
            data_ ? data_->getSeqNum() :
        uint32_t(-1);
    }

    void setSeqNum( uint32_t seqNum ) {
        if (data_) data_->setSeqNum(seqNum);
    }

protected:
    void checkFields() const throw (PvapException)
    {
        // using parent check
        if ( !data_->isValid() ) {
            throw MessageIsBrokenException(data_->isRequest(), getSeqNum(), "message PC_AUTH_RESP is broken: %s",data_->toString().c_str());
        }
    }

private:
    PC_AUTH_RESP( const PC_AUTH_RESP& );
    PC_AUTH_RESP& operator = ( const PC_AUTH_RESP& );

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    bool                owned_;
    AuthResponse* data_;

};

} // namespace scag2
} // namespace pvss
} // namespace pvap
#endif
