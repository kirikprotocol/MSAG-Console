// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_BC_SET_HPP__
#define __SCAG_PVSS_PVAP_BC_SET_HPP__

#include "util/int.h"
#include <string>
#include "scag/pvss/api/pvap/Exceptions.h"

// #include "scag/pvss/api/pvap/TimePolicy.h"
#include "scag/pvss/api/pvap/TimePolicyFiller.h"





#include "scag/pvss/api/packets/SetCommand.h"





namespace scag2{
namespace pvss{
namespace pvap{

class PVAPPROF;

class BC_SET
{
protected:
    static const int varNameTag = 20;
    static const int timePolicyTag = 21;
    static const int intValueTag = 25;
    static const int stringValueTag = 26;
    static const int boolValueTag = 27;
    static const int dateValueTag = 28;

public:
    BC_SET() :
    owned_(true),
    data_(new SetCommand)
    {
        valueTypeFlag = false;
    }

    BC_SET( SetCommand* other ) :
    owned_(false),
    data_(other)
    {
        valueTypeFlag = false;
    }

    ~BC_SET() {
        if ( data_ && owned_ ) delete data_;
    }

    void clear()
    {
        if ( ! data_ ) return;
        data_->clear();
        valueTypeFlag=false;
    }

    SetCommand* pop() {
        SetCommand* rv = data_;
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
            // printf( "write pos=%d field=%d\n", ds.getPos(), varNameTag );
            writer.writeTag(varNameTag);
            writer.writeAsciiLV(data_->getVarName());
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              getSeqNum(),
                                              "writing field varName in BC_SET: %s",
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
                                              getSeqNum(),
                                              "writing field timePolicy in BC_SET: %s",
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
                                                  getSeqNum(),
                                                  "writing field intValue in BC_SET:",
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
                                                  getSeqNum(),
                                                  "writing field stringValue in BC_SET:",
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
                                                  getSeqNum(),
                                                  "writing field boolValue in BC_SET:",
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
                                                  getSeqNum(),
                                                  "writing field dateValue in BC_SET:",
                                                  e.what() );
            }
        }
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
                                                      "duplicate field intValue of BC_SET", getSeqNum());
                    }
                    valueTypeFlag = true;
                    data_->setIntValue(reader.readIntLV());
                    break;
                }
                case stringValueTag: {
                    if (valueTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field stringValue of BC_SET", getSeqNum());
                    }
                    valueTypeFlag = true;
                    data_->setStringValue(reader.readUTFLV());
                    break;
                }
                case boolValueTag: {
                    if (valueTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field boolValue of BC_SET", getSeqNum());
                    }
                    valueTypeFlag = true;
                    data_->setBoolValue(reader.readBoolLV());
                    break;
                }
                case dateValueTag: {
                    if (valueTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field dateValue of BC_SET", getSeqNum());
                    }
                    valueTypeFlag = true;
                    data_->setDateValue(reader.readIntLV());
                    break;
                }
                default:
                    throw InvalidFieldTypeException(data_->isRequest(),"BC_SET", getSeqNum(),tag);
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              getSeqNum(),
                                              "reading field tag=%d of BC_SET: %s",
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
            throw MessageIsBrokenException(data_->isRequest(), getSeqNum(), "message BC_SET is broken: %s",data_->toString().c_str());
        }
    }

private:
    BC_SET( const BC_SET& );
    BC_SET& operator = ( const BC_SET& );

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    bool                owned_;
    SetCommand* data_;
    uint8_t valueType;

    bool valueTypeFlag;
};

} // namespace scag2
} // namespace pvss
} // namespace pvap
#endif
