// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef SCAG_BILL_EWALLET_STREAM_AUTH_HPP
#define SCAG_BILL_EWALLET_STREAM_AUTH_HPP

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

class Auth
{
protected:
    static const int protocolVersionTag = 2;
    static const int loginTag = 3;
    static const int passwordTag = 4;
    static const int nameTag = 5;

public:
    Auth()
    {
        protocolVersionFlag = false;
        loginFlag = false;
        passwordFlag = false;
        nameFlag = false;
    }

    ~Auth() {
    }

    void clear()
    {
        protocolVersionFlag=false;
        loginFlag=false;
        passwordFlag=false;
        nameFlag=false;
    }

    std::string toString() const
    {
        std::string res;
        res.reserve(200);
        res.append("Auth");
        {
            char buf[40];
            sprintf(buf," seq=%u",unsigned(getSeqNum()));
            res.append(buf);
        }
        if ( protocolVersionFlag ) {
            res.append(" protocolVersion=");
            char buf[10]; sprintf(buf,"%u",unsigned(protocolVersion)); res.append(buf);
        }
        if ( loginFlag ) {
            res.append(" login=");
            res.push_back('"'); res.append(login); res.push_back('"');
        }
        if ( passwordFlag ) {
            res.append(" password=");
            res.push_back('"'); res.append(password); res.push_back('"');
        }
        if ( nameFlag ) {
            res.append(" name=");
            res.push_back('"'); res.append(name); res.push_back('"');
        }
        return res;
    }

    bool hasProtocolVersion() const {
        return protocolVersionFlag;
    }
    uint8_t getProtocolVersion() const {
        if (!protocolVersionFlag) {
            throw Exception( excType(), "Auth has no field protocolVersion");
        }
        return protocolVersion;
    }
    void setProtocolVersion( uint8_t v ) {
        this->protocolVersionFlag = true;
        this->protocolVersion = v;
    }
    bool hasLogin() const {
        return loginFlag;
    }
    const std::string& getLogin() const {
        if (!loginFlag) {
            throw Exception( excType(), "Auth has no field login");
        }
        return login;
    }
    void setLogin( const std::string& v ) {
        this->loginFlag = true;
        this->login = v;
    }
    bool hasPassword() const {
        return passwordFlag;
    }
    const std::string& getPassword() const {
        if (!passwordFlag) {
            throw Exception( excType(), "Auth has no field password");
        }
        return password;
    }
    void setPassword( const std::string& v ) {
        this->passwordFlag = true;
        this->password = v;
    }
    bool hasName() const {
        return nameFlag;
    }
    const std::string& getName() const {
        if (!nameFlag) {
            throw Exception( excType(), "Auth has no field name");
        }
        return name;
    }
    void setName( const std::string& v ) {
        this->nameFlag = true;
        this->name = v;
    }

    template < class DataStream >
        void serialize( const Protocol& proto, DataStream& writer ) const
    {
        // printf( "serializing %s\n", toString().c_str() );
        checkFields();
        // mandatory fields
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), protocolVersionTag );
            writer.writeTag(protocolVersionTag);
            writer.writeByteLV(protocolVersion);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field protocolVersion in Auth: %s",
                             e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), loginTag );
            writer.writeTag(loginTag);
            writer.writeAsciiLV(login);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field login in Auth: %s",
                             e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), passwordTag );
            writer.writeTag(passwordTag);
            writer.writeAsciiLV(password);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field password in Auth: %s",
                             e.what() );
        }
        try {
            // printf( "write pos=%d field=%d\n", ds.getPos(), nameTag );
            writer.writeTag(nameTag);
            writer.writeAsciiLV(name);
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "writing field name in Auth: %s",
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
                case protocolVersionTag: {
                    if (protocolVersionFlag) {
                        throw Exception( excType(),
                                         "duplicate field protocolVersion of Auth" );
                    }
                    protocolVersion = reader.readByteLV();
                    protocolVersionFlag = true;
                    break;
                }
                case loginTag: {
                    if (loginFlag) {
                        throw Exception( excType(),
                                         "duplicate field login of Auth" );
                    }
                    login = reader.readAsciiLV();
                    loginFlag = true;
                    break;
                }
                case passwordTag: {
                    if (passwordFlag) {
                        throw Exception( excType(),
                                         "duplicate field password of Auth" );
                    }
                    password = reader.readAsciiLV();
                    passwordFlag = true;
                    break;
                }
                case nameTag: {
                    if (nameFlag) {
                        throw Exception( excType(),
                                         "duplicate field name of Auth" );
                    }
                    name = reader.readAsciiLV();
                    nameFlag = true;
                    break;
                }
                default:
                    throw Exception( excType(),
                                     "Auth has invalid field:%u", tag );
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "reading field tag=%u of Auth: %s",
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
        if ( !protocolVersionFlag ) {
            throw Exception( excType(),
                             "message Auth does not have required field protocolVersion" );
        }
        if ( !loginFlag ) {
            throw Exception( excType(),
                             "message Auth does not have required field login" );
        }
        if ( !passwordFlag ) {
            throw Exception( excType(),
                             "message Auth does not have required field password" );
        }
        if ( !nameFlag ) {
            throw Exception( excType(),
                             "message Auth does not have required field name" );
        }
 
    }

private:
    Auth( const Auth& );
    Auth& operator = ( const Auth& );

    inline bool isRequest() const {
        return true;
    }

    inline uint8_t excType() const {
        return isRequest() ? Status::BAD_REQUEST : Status::BAD_RESPONSE;
    }

protected:
    int32_t seqNum_;

    uint8_t protocolVersion;
    std::string login;
    std::string password;
    std::string name;

    bool protocolVersionFlag;
    bool loginFlag;
    bool passwordFlag;
    bool nameFlag;
};

} // namespace scag2
} // namespace bill
} // namespace ewallet
} // namespace stream
#endif
