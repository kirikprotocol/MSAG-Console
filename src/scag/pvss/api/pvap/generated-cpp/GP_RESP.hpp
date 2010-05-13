// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_GP_RESP_HPP__
#define __SCAG_PVSS_PVAP_GP_RESP_HPP__

#include "util/int.h"
#include <string>
#include "scag/pvss/api/pvap/Exceptions.h"

#include "scag/pvss/api/packets/GetProfileResponseComponent.h"





namespace scag2{
namespace pvss{
namespace pvap{

class PVAPGP;

class GP_RESP
{
protected:
    static const int varNameTag = 20;

public:
    GP_RESP() :
    owned_(true),
    data_(new GetProfileResponseComponent)
    {
    }

    GP_RESP( GetProfileResponseComponent* other ) :
    owned_(false),
    data_(other)
    {
    }

    ~GP_RESP() {
        if ( data_ && owned_ ) delete data_;
    }

    void clear()
    {
        if ( ! data_ ) return;
        data_->clear();
    }

    GetProfileResponseComponent* pop() {
        GetProfileResponseComponent* rv = data_;
        data_ = 0;
        return rv;
    }

    std::string toString() const
    {
        return data_ ? data_->toString() : "";
    }

    template < class DataStream >
        void serialize( const PVAPGP& proto, DataStream& writer ) const throw (PvapException)
    {
        if ( ! data_ ) return;
        checkFields();
        // mandatory fields
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), varNameTag );
            writer.writeTag(varNameTag);
            writer.writeAsciiLV(data_->getVarName());
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              getSeqNum(),
                                              "writing field varName in GP_RESP: %s",
                                              e.what() );
        }
        // optional fields
    }

    template <class DataStream> void deserialize( PVAPGP& proto, DataStream& reader )
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
                case varNameTag: {
                    data_->setVarName(reader.readAsciiLV());
                    break;
                }
                default:
                    throw InvalidFieldTypeException(data_->isRequest(),"GP_RESP", getSeqNum(),tag);
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              getSeqNum(),
                                              "reading field tag=%d of GP_RESP: %s",
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
            throw MessageIsBrokenException(data_->isRequest(), getSeqNum(), "message GP_RESP is broken: %s",data_->toString().c_str());
        }
    }

private:
    GP_RESP( const GP_RESP& );
    GP_RESP& operator = ( const GP_RESP& );

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    bool                owned_;
    GetProfileResponseComponent* data_;

};

} // namespace scag2
} // namespace pvss
} // namespace pvap
#endif
