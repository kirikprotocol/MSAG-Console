// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef SCAG_BILL_EWALLET_STREAM_CHECK_HPP
#define SCAG_BILL_EWALLET_STREAM_CHECK_HPP

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

class Check
{
protected:
    static const int agentIdTag = 6;
    static const int userIdTag = 7;
    static const int walletTypeTag = 8;
    static const int externalIdTag = 18;
    static const int transIdTag = 12;

public:
    Check()
    {
        agentIdFlag = false;
        userIdFlag = false;
        walletTypeFlag = false;
        externalIdFlag = false;
        transIdFlag = false;
    }

    ~Check() {
    }

    void clear()
    {
        agentIdFlag=false;
        userIdFlag=false;
        walletTypeFlag=false;
        externalIdFlag=false;
        transIdFlag=false;
    }

    std::string toString() const
    {
        std::string res;
        res.reserve(200);
        res.append("Check");
        {
            char buf[40];
            sprintf(buf," seq=%u",unsigned(getSeqNum()));
            res.append(buf);
        }
        if ( agentIdFlag ) {
            res.append(" agentId=");
            char buf[20]; sprintf(buf,"%d",unsigned(agentId)); res.append(buf);
        }
        if ( userIdFlag ) {
            res.append(" userId=");
            res.push_back('"'); res.append(userId); res.push_back('"');
        }
        if ( walletTypeFlag ) {
            res.append(" walletType=");
            res.push_back('"'); res.append(walletType); res.push_back('"');
        }
        if ( externalIdFlag ) {
            res.append(" externalId=");
            res.push_back('"'); res.append(externalId); res.push_back('"');
        }
        if ( transIdFlag ) {
            res.append(" transId=");
            char buf[20]; sprintf(buf,"%d",unsigned(transId)); res.append(buf);
        }
        return res;
    }

    bool hasAgentId() const {
        return agentIdFlag;
    }
    int32_t getAgentId() const {
        if (!agentIdFlag) {
            throw Exception( excType(), "Check has no field agentId");
        }
        return agentId;
    }
    void setAgentId( int32_t v ) {
        this->agentIdFlag = true;
        this->agentId = v;
    }
    bool hasUserId() const {
        return userIdFlag;
    }
    const std::string& getUserId() const {
        if (!userIdFlag) {
            throw Exception( excType(), "Check has no field userId");
        }
        return userId;
    }
    void setUserId( const std::string& v ) {
        this->userIdFlag = true;
        this->userId = v;
    }
    bool hasWalletType() const {
        return walletTypeFlag;
    }
    const std::string& getWalletType() const {
        if (!walletTypeFlag) {
            throw Exception( excType(), "Check has no field walletType");
        }
        return walletType;
    }
    void setWalletType( const std::string& v ) {
        this->walletTypeFlag = true;
        this->walletType = v;
    }
    bool hasExternalId() const {
        return externalIdFlag;
    }
    const std::string& getExternalId() const {
        if (!externalIdFlag) {
            throw Exception( excType(), "Check has no field externalId");
        }
        return externalId;
    }
    void setExternalId( const std::string& v ) {
        this->externalIdFlag = true;
        this->externalId = v;
    }
    bool hasTransId() const {
        return transIdFlag;
    }
    int32_t getTransId() const {
        if (!transIdFlag) {
            throw Exception( excType(), "Check has no field transId");
        }
        return transId;
    }
    void setTransId( int32_t v ) {
        this->transIdFlag = true;
        this->transId = v;
    }

    template < class DataStream >
        void serialize( const Protocol& proto, DataStream& writer ) const
    {
        // printf( "serializing %s\n", toString().c_str() );
        checkFields();
        // mandatory fields
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), agentIdTag );
            writer.writeTag(agentIdTag);
            writer.writeIntLV(agentId);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field agentId in Check: %s",
                             e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), userIdTag );
            writer.writeTag(userIdTag);
            writer.writeAsciiLV(userId);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field userId in Check: %s",
                             e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), walletTypeTag );
            writer.writeTag(walletTypeTag);
            writer.writeAsciiLV(walletType);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field walletType in Check: %s",
                             e.what() );
        }
        // optional fields
        if ( externalIdFlag ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), externalIdTag );
            try {
                writer.writeTag(externalIdTag);
                writer.writeAsciiLV(externalId);
            } catch ( exceptions::IOException e ) {
                throw Exception( excType(),
                                 "writing field externalId in Check: %s",
                                 e.what() );
            }
        }
        if ( transIdFlag ) {
            // printf( "write pos=%d field=%d\n", writer.getPos(), transIdTag );
            try {
                writer.writeTag(transIdTag);
                writer.writeIntLV(transId);
            } catch ( exceptions::IOException e ) {
                throw Exception( excType(),
                                 "writing field transId in Check: %s",
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
                case agentIdTag: {
                    if (agentIdFlag) {
                        throw Exception( excType(),
                                         "duplicate field agentId of Check" );
                    }
                    agentId = reader.readIntLV();
                    agentIdFlag = true;
                    break;
                }
                case userIdTag: {
                    if (userIdFlag) {
                        throw Exception( excType(),
                                         "duplicate field userId of Check" );
                    }
                    userId = reader.readAsciiLV();
                    userIdFlag = true;
                    break;
                }
                case walletTypeTag: {
                    if (walletTypeFlag) {
                        throw Exception( excType(),
                                         "duplicate field walletType of Check" );
                    }
                    walletType = reader.readAsciiLV();
                    walletTypeFlag = true;
                    break;
                }
                case externalIdTag: {
                    if (externalIdFlag) {
                        throw Exception( excType(),
                                         "duplicate field externalId of Check" );
                    }
                    externalId = reader.readAsciiLV();
                    externalIdFlag = true;
                    break;
                }
                case transIdTag: {
                    if (transIdFlag) {
                        throw Exception( excType(),
                                         "duplicate field transId of Check" );
                    }
                    transId = reader.readIntLV();
                    transIdFlag = true;
                    break;
                }
                default:
                    throw Exception( excType(),
                                     "Check has invalid field:%u", tag );
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "reading field tag=%u of Check: %s",
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
        if ( !agentIdFlag ) {
            throw Exception( excType(),
                             "message Check does not have required field agentId" );
        }
        if ( !userIdFlag ) {
            throw Exception( excType(),
                             "message Check does not have required field userId" );
        }
        if ( !walletTypeFlag ) {
            throw Exception( excType(),
                             "message Check does not have required field walletType" );
        }
 
    }

private:
    Check( const Check& );
    Check& operator = ( const Check& );

    inline bool isRequest() const {
        return true;
    }

    inline uint8_t excType() const {
        return isRequest() ? Status::BAD_REQUEST : Status::BAD_RESPONSE;
    }

protected:
    int32_t seqNum_;

    int32_t agentId;
    std::string userId;
    std::string walletType;
    std::string externalId;
    int32_t transId;

    bool agentIdFlag;
    bool userIdFlag;
    bool walletTypeFlag;
    bool externalIdFlag;
    bool transIdFlag;
};

} // namespace scag2
} // namespace bill
} // namespace ewallet
} // namespace stream
#endif
