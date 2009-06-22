// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef SCAG_BILL_EWALLET_STREAM_INFO_HPP
#define SCAG_BILL_EWALLET_STREAM_INFO_HPP

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

class Info
{
protected:
    static const int agentIdTag = 6;
    static const int userIdTag = 7;
    static const int walletTypeTag = 8;

public:
    Info()
    {
        agentIdFlag = false;
        userIdFlag = false;
        walletTypeFlag = false;
    }

    ~Info() {
    }

    void clear()
    {
        agentIdFlag=false;
        userIdFlag=false;
        walletTypeFlag=false;
    }

    std::string toString() const
    {
        std::string res;
        res.reserve(200);
        res.append("Info");
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
        return res;
    }

    bool hasAgentId() const {
        return agentIdFlag;
    }
    int32_t getAgentId() const {
        if (!agentIdFlag) {
            throw Exception( excType(), "Info has no field agentId");
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
            throw Exception( excType(), "Info has no field userId");
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
            throw Exception( excType(), "Info has no field walletType");
        }
        return walletType;
    }
    void setWalletType( const std::string& v ) {
        this->walletTypeFlag = true;
        this->walletType = v;
    }

    template < class DataStream >
        void serialize( const Protocol& proto, DataStream& writer ) const
    {
        printf( "serializing %s\n", toString().c_str() );
        checkFields();
        // mandatory fields
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), agentIdTag );
            writer.writeTag(agentIdTag);
            writer.writeIntLV(agentId);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field agentId in Info: %s",
                             e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), userIdTag );
            writer.writeTag(userIdTag);
            writer.writeAsciiLV(userId);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field userId in Info: %s",
                             e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), walletTypeTag );
            writer.writeTag(walletTypeTag);
            writer.writeAsciiLV(walletType);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field walletType in Info: %s",
                             e.what() );
        }
        // optional fields
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
                                         "duplicate field agentId of Info" );
                    }
                    agentId = reader.readIntLV();
                    agentIdFlag = true;
                    break;
                }
                case userIdTag: {
                    if (userIdFlag) {
                        throw Exception( excType(),
                                         "duplicate field userId of Info" );
                    }
                    userId = reader.readAsciiLV();
                    userIdFlag = true;
                    break;
                }
                case walletTypeTag: {
                    if (walletTypeFlag) {
                        throw Exception( excType(),
                                         "duplicate field walletType of Info" );
                    }
                    walletType = reader.readAsciiLV();
                    walletTypeFlag = true;
                    break;
                }
                default:
                    throw Exception( excType(),
                                     "Info has invalid field:%u", tag );
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "reading field tag=%u of Info: %s",
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
                             "message Info does not have required field agentId" );
        }
        if ( !userIdFlag ) {
            throw Exception( excType(),
                             "message Info does not have required field userId" );
        }
        if ( !walletTypeFlag ) {
            throw Exception( excType(),
                             "message Info does not have required field walletType" );
        }
 
    }

private:
    Info( const Info& );
    Info& operator = ( const Info& );

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

    bool agentIdFlag;
    bool userIdFlag;
    bool walletTypeFlag;
};

} // namespace scag2
} // namespace bill
} // namespace ewallet
} // namespace stream
#endif
