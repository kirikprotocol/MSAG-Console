// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_BC_GET_RESP_HPP__
#define __SCAG_PVSS_PVAP_BC_GET_RESP_HPP__

#include "util/int.h"
#include <string>
#include "scag/pvss/api/pvap/Exceptions.h"


// #include "scag/pvss/api/pvap/TimePolicy.h"
#include "scag/pvss/api/pvap/TimePolicyFiller.h"





#include "scag/pvss/api/packets/GetResponse.h"





namespace scag2{
namespace pvss{
namespace pvap{

class PVAPBC;

class BC_GET_RESP
{
protected:
    static const int statusValueTag = 1;
    static const int varNameTag = 20;
    static const int timePolicyTag = 21;
    static const int intValueTag = 25;
    static const int stringValueTag = 26;
    static const int boolValueTag = 27;
    static const int dateValueTag = 28;

public:
    BC_GET_RESP( int seqNum ) :
    owned_(true),
    data_(new GetResponse(seqNum))
    {
        valueTypeFlag = false;
    }

    BC_GET_RESP( GetResponse* other ) :
    owned_(false),
    data_(other)
    {
        valueTypeFlag = false;
    }

    ~BC_GET_RESP() {
        if ( data_ && owned_ ) delete data_;
    }

    void clear()
    {
        if ( ! data_ ) return;
        data_->clear();
        valueTypeFlag=false;
    }

    GetResponse* pop() {
        GetResponse* rv = data_;
        data_ = 0;
        return rv;
    }

    std::string toString() const
    {
        return data_ ? data_->toString() : "";
    }

    template < class DataStream >
        void serialize( const PVAPBC& proto, DataStream& writer ) const /* throw (PvapException) */ 
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
                                              data_->getSeqNum(),
                                              "writing field statusValue in BC_GET_RESP: %s",
                                              e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), varNameTag );
            writer.writeTag(varNameTag);
            writer.writeAsciiLV(data_->getVarName());
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              data_->getSeqNum(),
                                              "writing field varName in BC_GET_RESP: %s",
                                              e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), timePolicyTag );
            writer.writeTag(timePolicyTag);
            {
                TimePolicyFiller helper( data_ );
                helper.serialize( proto, writer );
 
            }
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              data_->getSeqNum(),
                                              "writing field timePolicy in BC_GET_RESP: %s",
                                              e.what() );
        }
        // optional fields
        if ( data_->hasIntValue() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), intValueTag );
            try {
                writer.writeTag(intValueTag);
                writer.writeIntLV(data_->getIntValue());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  data_->getSeqNum(),
                                                  "writing field intValue in BC_GET_RESP:",
                                                  e.what() );
            }
        }
        if ( data_->hasStringValue() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), stringValueTag );
            try {
                writer.writeTag(stringValueTag);
                writer.writeUTFLV(data_->getStringValue());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  data_->getSeqNum(),
                                                  "writing field stringValue in BC_GET_RESP:",
                                                  e.what() );
            }
        }
        if ( data_->hasBoolValue() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), boolValueTag );
            try {
                writer.writeTag(boolValueTag);
                writer.writeBoolLV(data_->getBoolValue());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  data_->getSeqNum(),
                                                  "writing field boolValue in BC_GET_RESP:",
                                                  e.what() );
            }
        }
        if ( data_->hasDateValue() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), dateValueTag );
            try {
                writer.writeTag(dateValueTag);
                writer.writeIntLV(data_->getDateValue());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  data_->getSeqNum(),
                                                  "writing field dateValue in BC_GET_RESP:",
                                                  e.what() );
            }
        }
    }

    template <class DataStream> void deserialize( PVAPBC& proto, DataStream& reader )
        /* throw (PvapException) */ 
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
                case varNameTag: {
                    data_->setVarName(reader.readAsciiLV());
                    break;
                }
                case timePolicyTag: {
                    TimePolicyFiller helper(data_);
                    helper.deserialize(proto,reader);
                    break;
                }
                case intValueTag: {
                    if (valueTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field intValue of BC_GET_RESP", data_->getSeqNum());
                    }
                    valueTypeFlag = true;
                    data_->setIntValue(reader.readIntLV());
                    break;
                }
                case stringValueTag: {
                    if (valueTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field stringValue of BC_GET_RESP", data_->getSeqNum());
                    }
                    valueTypeFlag = true;
                    data_->setStringValue(reader.readUTFLV());
                    break;
                }
                case boolValueTag: {
                    if (valueTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field boolValue of BC_GET_RESP", data_->getSeqNum());
                    }
                    valueTypeFlag = true;
                    data_->setBoolValue(reader.readBoolLV());
                    break;
                }
                case dateValueTag: {
                    if (valueTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field dateValue of BC_GET_RESP", data_->getSeqNum());
                    }
                    valueTypeFlag = true;
                    data_->setDateValue(reader.readIntLV());
                    break;
                }
                default:
                    throw InvalidFieldTypeException(data_->isRequest(),"BC_GET_RESP", data_->getSeqNum(),tag);
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              data_->getSeqNum(),
                                              "reading field tag=%d of BC_GET_RESP: %s",
                                              tag, e.what() );
        }
        checkFields();
    }

    uint32_t getSeqNum() const {
        return data_ ? data_->getSeqNum() : uint32_t(-1);
    }

protected:
    void checkFields() const /* throw (PvapException) */ 
    {
        // using parent check
        if ( !data_->isValid() ) {
            throw MessageIsBrokenException(data_->isRequest(), data_->getSeqNum(), "message BC_GET_RESP is broken: %s",data_->toString().c_str());
        }
    }

private:
    BC_GET_RESP( const BC_GET_RESP& );
    BC_GET_RESP& operator = ( const BC_GET_RESP& );

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    bool                owned_;
    GetResponse* data_;
    uint8_t valueType;

    bool valueTypeFlag;
};

} // namespace scag2
} // namespace pvss
} // namespace pvap
#endif
