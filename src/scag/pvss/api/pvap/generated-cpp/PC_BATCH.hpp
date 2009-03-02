// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef __SCAG_PVSS_PVAP_PC_BATCH_HPP__
#define __SCAG_PVSS_PVAP_PC_BATCH_HPP__

#include "util/int.h"
#include <string>
#include "scag/pvss/api/pvap/Exceptions.h"






// #include "scag/pvss/api/pvap/BatchCmdArray.h"
#include "scag/pvss/api/pvap/BatchCommandFiller.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/BatchCommand.h"




namespace scag2{
namespace pvss{
namespace pvap{

class PVAP;

class PC_BATCH
{
protected:
    static const int abonentKeyTag = 3;
    static const int operatorKeyTag = 4;
    static const int providerKeyTag = 5;
    static const int serviceKeyTag = 6;
    static const int transactionalTag = 37;
    static const int batchContentTag = 38;

public:
    PC_BATCH( int seqNum ) :
    owned_(true),
    data_(new ProfileRequest< BatchCommand >(new BatchCommand(seqNum)))
    {
        profileTypeFlag = false;
    }

    PC_BATCH( ProfileRequest< BatchCommand >* other ) :
    owned_(false),
    data_(other)
    {
        profileTypeFlag = false;
    }

    ~PC_BATCH() {
        if ( data_ && owned_ ) delete data_;
    }

    void clear()
    {
        if ( ! data_ ) return;
        data_->clear();
        profileTypeFlag=false;
    }

    ProfileRequest< BatchCommand >* pop() {
        ProfileRequest< BatchCommand >* rv = data_;
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
            // printf( "write pos=%d field=%d\n", ds.getPos(), transactionalTag );
            writer.writeTag(transactionalTag);
            writer.writeBoolLV(data_->getCommand()->isTransactional());
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              data_->getSeqNum(),
                                              "writing field transactional in PC_BATCH: %s",
                                              e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), batchContentTag );
            writer.writeTag(batchContentTag);
            {
                BatchCommandFiller helper( data_->getCommand() );
                helper.serialize( proto, writer );
 
            }
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              data_->getSeqNum(),
                                              "writing field batchContent in PC_BATCH: %s",
                                              e.what() );
        }
        // optional fields
        if ( data_->getProfileKey().hasAbonentKey() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), abonentKeyTag );
            try {
                writer.writeTag(abonentKeyTag);
                writer.writeAsciiLV(data_->getProfileKey().getAbonentKey());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  data_->getSeqNum(),
                                                  "writing field abonentKey in PC_BATCH:",
                                                  e.what() );
            }
        }
        if ( data_->getProfileKey().hasOperatorKey() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), operatorKeyTag );
            try {
                writer.writeTag(operatorKeyTag);
                writer.writeIntLV(data_->getProfileKey().getOperatorKey());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  data_->getSeqNum(),
                                                  "writing field operatorKey in PC_BATCH:",
                                                  e.what() );
            }
        }
        if ( data_->getProfileKey().hasProviderKey() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), providerKeyTag );
            try {
                writer.writeTag(providerKeyTag);
                writer.writeIntLV(data_->getProfileKey().getProviderKey());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  data_->getSeqNum(),
                                                  "writing field providerKey in PC_BATCH:",
                                                  e.what() );
            }
        }
        if ( data_->getProfileKey().hasServiceKey() ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), serviceKeyTag );
            try {
                writer.writeTag(serviceKeyTag);
                writer.writeIntLV(data_->getProfileKey().getServiceKey());
            } catch ( exceptions::IOException e ) {
                throw PvapSerializationException( data_->isRequest(),
                                                  data_->getSeqNum(),
                                                  "writing field serviceKey in PC_BATCH:",
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
                case abonentKeyTag: {
                    if (profileTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field abonentKey of PC_BATCH", data_->getSeqNum());
                    }
                    profileTypeFlag = true;
                    data_->getProfileKey().setAbonentKey(reader.readAsciiLV());
                    break;
                }
                case operatorKeyTag: {
                    if (profileTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field operatorKey of PC_BATCH", data_->getSeqNum());
                    }
                    profileTypeFlag = true;
                    data_->getProfileKey().setOperatorKey(reader.readIntLV());
                    break;
                }
                case providerKeyTag: {
                    if (profileTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field providerKey of PC_BATCH", data_->getSeqNum());
                    }
                    profileTypeFlag = true;
                    data_->getProfileKey().setProviderKey(reader.readIntLV());
                    break;
                }
                case serviceKeyTag: {
                    if (profileTypeFlag) {
                        throw DuplicateFieldException(data_->isRequest(),
                                                      "duplicate field serviceKey of PC_BATCH", data_->getSeqNum());
                    }
                    profileTypeFlag = true;
                    data_->getProfileKey().setServiceKey(reader.readIntLV());
                    break;
                }
                case transactionalTag: {
                    data_->getCommand()->setTransactional(reader.readBoolLV());
                    break;
                }
                case batchContentTag: {
                    BatchCommandFiller helper(data_->getCommand());
                    helper.deserialize(proto,reader);
                    break;
                }
                default:
                    throw InvalidFieldTypeException(data_->isRequest(),"invalid field in PC_BATCH", data_->getSeqNum(),tag);
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw PvapSerializationException( data_->isRequest(),
                                              data_->getSeqNum(),
                                              "reading field tag=%d of PC_BATCH: %s",
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
            throw MessageIsBrokenException(data_->isRequest(), "message PC_BATCH is broken: " + toString(), data_->getSeqNum() );
        }
    }

private:
    PC_BATCH( const PC_BATCH& );
    PC_BATCH& operator = ( const PC_BATCH& );

protected:
    //static const uint8_t versionMajor=2;
    //static const uint8_t versionMinor=0;

    bool                owned_;
    ProfileRequest< BatchCommand >* data_;
    uint8_t profileType;

    bool profileTypeFlag;
};

} // namespace scag2
} // namespace pvss
} // namespace pvap
#endif
