// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef SCAG_BILL_EWALLET_STREAM_TRANSFERCHECKRESP_HPP
#define SCAG_BILL_EWALLET_STREAM_TRANSFERCHECKRESP_HPP

#include "util/int.h"
#include <string>
#include "scag/bill/ewallet/Status.h"
#include "scag/exc/IOException.h"
#include "scag/bill/ewallet/Exception.h"


namespace scag2 {
namespace bill {
namespace ewallet {
namespace stream {

class Protocol;

class TransferCheckResp
{
protected:
    static const int statusValueTag = 1;
    static const int srcWalletTypeTag = 8;
    static const int dstWalletTypeTag = 17;
    static const int externalIdTag = 18;
    static const int amountTag = 10;
    static const int descriptionTag = 9;
    static const int transStatusTag = 20;
    static const int enddateTag = 15;

public:
    TransferCheckResp()
    {
        statusValueFlag = false;
        srcWalletTypeFlag = false;
        dstWalletTypeFlag = false;
        externalIdFlag = false;
        amountFlag = false;
        descriptionFlag = false;
        transStatusFlag = false;
        enddateFlag = false;
    }

    ~TransferCheckResp() {
    }

    void clear()
    {
        statusValueFlag=false;
        srcWalletTypeFlag=false;
        dstWalletTypeFlag=false;
        externalIdFlag=false;
        amountFlag=false;
        descriptionFlag=false;
        transStatusFlag=false;
        enddateFlag=false;
    }

    std::string toString() const
    {
        std::string res;
        res.reserve(200);
        res.append("TransferCheckResp");
        {
            char buf[40];
            sprintf(buf," seq=%u",unsigned(getSeqNum()));
            res.append(buf);
        }
        if ( statusValueFlag ) {
            res.append(" statusValue=");
            char buf[10]; sprintf(buf,"%u",unsigned(statusValue)); res.append(buf);
        }
        if ( srcWalletTypeFlag ) {
            res.append(" srcWalletType=");
            res.push_back('"'); res.append(srcWalletType); res.push_back('"');
        }
        if ( dstWalletTypeFlag ) {
            res.append(" dstWalletType=");
            res.push_back('"'); res.append(dstWalletType); res.push_back('"');
        }
        if ( externalIdFlag ) {
            res.append(" externalId=");
            res.push_back('"'); res.append(externalId); res.push_back('"');
        }
        if ( amountFlag ) {
            res.append(" amount=");
            char buf[20]; sprintf(buf,"%d",unsigned(amount)); res.append(buf);
        }
        if ( descriptionFlag ) {
            res.append(" description=");
            res.push_back('"'); res.append(description); res.push_back('"');
        }
        if ( transStatusFlag ) {
            res.append(" transStatus=");
            char buf[20]; sprintf(buf,"%d",unsigned(transStatus)); res.append(buf);
        }
        if ( enddateFlag ) {
            res.append(" enddate=");
            char buf[30]; sprintf(buf,"%lld",static_cast<long long>(enddate)); res.append(buf);
        }
        return res;
    }

    bool hasStatusValue() const {
        return statusValueFlag;
    }
    uint8_t getStatusValue() const {
        if (!statusValueFlag) {
            throw Exception( excType(), "TransferCheckResp has no field statusValue");
        }
        return statusValue;
    }
    void setStatusValue( uint8_t v ) {
        this->statusValueFlag = true;
        this->statusValue = v;
    }
    bool hasSrcWalletType() const {
        return srcWalletTypeFlag;
    }
    const std::string& getSrcWalletType() const {
        if (!srcWalletTypeFlag) {
            throw Exception( excType(), "TransferCheckResp has no field srcWalletType");
        }
        return srcWalletType;
    }
    void setSrcWalletType( const std::string& v ) {
        this->srcWalletTypeFlag = true;
        this->srcWalletType = v;
    }
    bool hasDstWalletType() const {
        return dstWalletTypeFlag;
    }
    const std::string& getDstWalletType() const {
        if (!dstWalletTypeFlag) {
            throw Exception( excType(), "TransferCheckResp has no field dstWalletType");
        }
        return dstWalletType;
    }
    void setDstWalletType( const std::string& v ) {
        this->dstWalletTypeFlag = true;
        this->dstWalletType = v;
    }
    bool hasExternalId() const {
        return externalIdFlag;
    }
    const std::string& getExternalId() const {
        if (!externalIdFlag) {
            throw Exception( excType(), "TransferCheckResp has no field externalId");
        }
        return externalId;
    }
    void setExternalId( const std::string& v ) {
        this->externalIdFlag = true;
        this->externalId = v;
    }
    bool hasAmount() const {
        return amountFlag;
    }
    int32_t getAmount() const {
        if (!amountFlag) {
            throw Exception( excType(), "TransferCheckResp has no field amount");
        }
        return amount;
    }
    void setAmount( int32_t v ) {
        this->amountFlag = true;
        this->amount = v;
    }
    bool hasDescription() const {
        return descriptionFlag;
    }
    const std::string& getDescription() const {
        if (!descriptionFlag) {
            throw Exception( excType(), "TransferCheckResp has no field description");
        }
        return description;
    }
    void setDescription( const std::string& v ) {
        this->descriptionFlag = true;
        this->description = v;
    }
    bool hasTransStatus() const {
        return transStatusFlag;
    }
    int32_t getTransStatus() const {
        if (!transStatusFlag) {
            throw Exception( excType(), "TransferCheckResp has no field transStatus");
        }
        return transStatus;
    }
    void setTransStatus( int32_t v ) {
        this->transStatusFlag = true;
        this->transStatus = v;
    }
    bool hasEnddate() const {
        return enddateFlag;
    }
    int64_t getEnddate() const {
        if (!enddateFlag) {
            throw Exception( excType(), "TransferCheckResp has no field enddate");
        }
        return enddate;
    }
    void setEnddate( int64_t v ) {
        this->enddateFlag = true;
        this->enddate = v;
    }

    template < class DataStream >
        void serialize( const Protocol& proto, DataStream& writer ) const
    {
        printf( "serializing %s\n", toString().c_str() );
        checkFields();
        // mandatory fields
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), statusValueTag );
            writer.writeTag(statusValueTag);
            writer.writeByteLV(statusValue);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field statusValue in TransferCheckResp: %s",
                             e.what() );
        }
        // optional fields
        if ( srcWalletTypeFlag ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), srcWalletTypeTag );
            try {
                writer.writeTag(srcWalletTypeTag);
                writer.writeAsciiLV(srcWalletType);
            } catch ( exceptions::IOException e ) {
                throw Exception( excType(),
                                 "writing field srcWalletType in TransferCheckResp: %s",
                                 e.what() );
            }
        }
        if ( dstWalletTypeFlag ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), dstWalletTypeTag );
            try {
                writer.writeTag(dstWalletTypeTag);
                writer.writeAsciiLV(dstWalletType);
            } catch ( exceptions::IOException e ) {
                throw Exception( excType(),
                                 "writing field dstWalletType in TransferCheckResp: %s",
                                 e.what() );
            }
        }
        if ( externalIdFlag ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), externalIdTag );
            try {
                writer.writeTag(externalIdTag);
                writer.writeAsciiLV(externalId);
            } catch ( exceptions::IOException e ) {
                throw Exception( excType(),
                                 "writing field externalId in TransferCheckResp: %s",
                                 e.what() );
            }
        }
        if ( amountFlag ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), amountTag );
            try {
                writer.writeTag(amountTag);
                writer.writeIntLV(amount);
            } catch ( exceptions::IOException e ) {
                throw Exception( excType(),
                                 "writing field amount in TransferCheckResp: %s",
                                 e.what() );
            }
        }
        if ( descriptionFlag ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), descriptionTag );
            try {
                writer.writeTag(descriptionTag);
                writer.writeAsciiLV(description);
            } catch ( exceptions::IOException e ) {
                throw Exception( excType(),
                                 "writing field description in TransferCheckResp: %s",
                                 e.what() );
            }
        }
        if ( transStatusFlag ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), transStatusTag );
            try {
                writer.writeTag(transStatusTag);
                writer.writeIntLV(transStatus);
            } catch ( exceptions::IOException e ) {
                throw Exception( excType(),
                                 "writing field transStatus in TransferCheckResp: %s",
                                 e.what() );
            }
        }
        if ( enddateFlag ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), enddateTag );
            try {
                writer.writeTag(enddateTag);
                writer.writeLongLV(enddate);
            } catch ( exceptions::IOException e ) {
                throw Exception( excType(),
                                 "writing field enddate in TransferCheckResp: %s",
                                 e.what() );
            }
        }
    }


    template <class DataStream>
    void deserialize( Protocol& proto, DataStream& reader )
    {
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
                    if (statusValueFlag) {
                        throw Exception( excType(),
                                         "duplicate field statusValue of TransferCheckResp" );
                    }
                    statusValue = reader.readByteLV();
                    statusValueFlag = true;
                    break;
                }
                case srcWalletTypeTag: {
                    if (srcWalletTypeFlag) {
                        throw Exception( excType(),
                                         "duplicate field srcWalletType of TransferCheckResp" );
                    }
                    srcWalletType = reader.readAsciiLV();
                    srcWalletTypeFlag = true;
                    break;
                }
                case dstWalletTypeTag: {
                    if (dstWalletTypeFlag) {
                        throw Exception( excType(),
                                         "duplicate field dstWalletType of TransferCheckResp" );
                    }
                    dstWalletType = reader.readAsciiLV();
                    dstWalletTypeFlag = true;
                    break;
                }
                case externalIdTag: {
                    if (externalIdFlag) {
                        throw Exception( excType(),
                                         "duplicate field externalId of TransferCheckResp" );
                    }
                    externalId = reader.readAsciiLV();
                    externalIdFlag = true;
                    break;
                }
                case amountTag: {
                    if (amountFlag) {
                        throw Exception( excType(),
                                         "duplicate field amount of TransferCheckResp" );
                    }
                    amount = reader.readIntLV();
                    amountFlag = true;
                    break;
                }
                case descriptionTag: {
                    if (descriptionFlag) {
                        throw Exception( excType(),
                                         "duplicate field description of TransferCheckResp" );
                    }
                    description = reader.readAsciiLV();
                    descriptionFlag = true;
                    break;
                }
                case transStatusTag: {
                    if (transStatusFlag) {
                        throw Exception( excType(),
                                         "duplicate field transStatus of TransferCheckResp" );
                    }
                    transStatus = reader.readIntLV();
                    transStatusFlag = true;
                    break;
                }
                case enddateTag: {
                    if (enddateFlag) {
                        throw Exception( excType(),
                                         "duplicate field enddate of TransferCheckResp" );
                    }
                    enddate = reader.readLongLV();
                    enddateFlag = true;
                    break;
                }
                default:
                    throw Exception( excType(),
                                     "TransferCheckResp has invalid field:%u", tag );
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "reading field tag=%u of TransferCheckResp: %s",
                             tag, e.what() );
        }
        printf( "deserialized %s\n", toString().c_str() );
        checkFields();
    }

    int32_t getSeqNum() const { return seqNum_; }
    void setSeqNum( int32_t seqnum ) { seqNum_ = seqnum; }

protected:
    void checkFields() const
    {
        if ( !statusValueFlag ) {
            throw Exception( excType(),
                             "message TransferCheckResp does not have required field statusValue" );
        }
 
    }

private:
    TransferCheckResp( const TransferCheckResp& );
    TransferCheckResp& operator = ( const TransferCheckResp& );

    inline bool isRequest() const {
        return false;
    }

    inline uint8_t excType() const {
        return isRequest() ? Status::BAD_REQUEST : Status::BAD_RESPONSE;
    }

protected:
    int32_t seqNum_;

    uint8_t statusValue;
    std::string srcWalletType;
    std::string dstWalletType;
    std::string externalId;
    int32_t amount;
    std::string description;
    int32_t transStatus;
    int64_t enddate;

    bool statusValueFlag;
    bool srcWalletTypeFlag;
    bool dstWalletTypeFlag;
    bool externalIdFlag;
    bool amountFlag;
    bool descriptionFlag;
    bool transStatusFlag;
    bool enddateFlag;
};

} // namespace scag2
} // namespace bill
} // namespace ewallet
} // namespace stream
#endif
