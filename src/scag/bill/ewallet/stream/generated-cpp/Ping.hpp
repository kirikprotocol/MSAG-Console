// NOTE: Do NOT edit this file (it is auto-generated).
// Instead, edit message-cpp.tmpl and regenerate.

#ifndef SCAG_BILL_EWALLET_STREAM_PING_HPP
#define SCAG_BILL_EWALLET_STREAM_PING_HPP

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

class Ping
{
protected:

public:
    Ping()
    {
    }

    ~Ping() {
    }

    void clear()
    {
    }

    std::string toString() const
    {
        std::string res;
        res.reserve(200);
        res.append("Ping");
        {
            char buf[40];
            sprintf(buf," seq=%u",unsigned(getSeqNum()));
            res.append(buf);
        }
        return res;
    }


    template < class DataStream >
        void serialize( const Protocol& proto, DataStream& writer ) const
    {
        printf( "serializing %s\n", toString().c_str() );
        checkFields();
        // mandatory fields
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
                default:
                    throw Exception( excType(),
                                     "Ping has invalid field:%u", tag );
                }
            } while ( true );
        } catch ( exceptions::IOException e ) {
            throw Exception( excType(),
                             "reading field tag=%u of Ping: %s",
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
 
    }

private:
    Ping( const Ping& );
    Ping& operator = ( const Ping& );

    inline bool isRequest() const {
        return true;
    }

    inline uint8_t excType() const {
        return isRequest() ? Status::BAD_REQUEST : Status::BAD_RESPONSE;
    }

protected:
    int32_t seqNum_;


};

} // namespace scag2
} // namespace bill
} // namespace ewallet
} // namespace stream
#endif
