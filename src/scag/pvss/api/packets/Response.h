#ifndef _SCAG_PVSS_BASE_RESPONSE_H
#define _SCAG_PVSS_BASE_RESPONSE_H

#include "Packet.h"
#include "Request.h"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {

class ResponseVisitor;

///
/// Abstract class Request
///
class Response : public Packet
{
public:
    enum StatusType {
    // business logics messages (infos?): range [0..15]
            OK                 = 0x00,
            PROPERTY_NOT_FOUND = 0x01,
            TYPE_INCONSISTENCE = 0x02,
    // protocol messages (warnings): range [16..31]
            BAD_REQUEST        = 0x10,
    // generic severe failures (errors?): range [32..47]
            ERROR              = 0x20,
            NOT_SUPPORTED      = 0x21,
            SERVER_SHUTDOWN    = 0x22,
            SERVER_BUSY        = 0x23,
            REQUEST_TIMEOUT    = 0x24,
    // unknown
            UNKNOWN            = 0xff
    };

    static const char* statusToString( StatusType stat )
    {
#define STATSTRING(x) case (x) : return #x
        switch (stat) {
            STATSTRING(UNKNOWN);
            STATSTRING(OK);
            STATSTRING(ERROR);
            STATSTRING(PROPERTY_NOT_FOUND);
            STATSTRING(BAD_REQUEST);
            STATSTRING(TYPE_INCONSISTENCE);
            STATSTRING(NOT_SUPPORTED);
            STATSTRING(SERVER_SHUTDOWN);
            STATSTRING(SERVER_BUSY);
            STATSTRING(REQUEST_TIMEOUT);
#undef STATSTRING
        default : return "???";
        }
    }

    static const char* statusMessage( StatusType stat )
    {
        switch (stat) {
        case (UNKNOWN) : return "Unknown response";
        case (OK) :      return "Response Ok";
        case (ERROR) :   return "Response Error";
        case (PROPERTY_NOT_FOUND) : return "PropertyType not found";
        case (BAD_REQUEST) : return "Request is bad formed or invalid";
        case (TYPE_INCONSISTENCE) : return "Type inconsistence";
        case (NOT_SUPPORTED) : return "Feature is not supported";
        case (SERVER_SHUTDOWN) : return "Server is shutting down";
        case (SERVER_BUSY) : return "Server busy";
        case (REQUEST_TIMEOUT) : return "Request is timed out";
        default: return "???";
        }
    }


    static inline bool statusIsInfo( StatusType stat ) {
        return (uint8_t(stat) & 0xf0) == 0x00;
    }
    static inline bool statusIsWarn( StatusType stat ) {
        return (uint8_t(stat) & 0xf0) == 0x10;
    }
    static inline bool statusIsError( StatusType stat ) {
        return (uint8_t(stat) & 0xf0) == 0x20;
    }

protected:
    static smsc::logger::Logger* log_;

protected:
    Response() : seqNum_(uint32_t(-1)), status_(UNKNOWN) { initLogger(); }
    Response( uint32_t seqNum ) : seqNum_(seqNum), status_(UNKNOWN) { initLogger(); }
    Response( const Response& other ) : seqNum_(other.seqNum_), status_(other.status_) { initLogger(); }
    Response( uint32_t seqNum, StatusType status ) : seqNum_(seqNum), status_(status) { initLogger(); }
    void initLogger();

public:
    virtual ~Response();
    virtual bool isRequest() const { return false; }
    virtual uint32_t getSeqNum() const { return seqNum_; }
    virtual void setSeqNum( uint32_t seqNum ) { seqNum_ = seqNum; }

    StatusType getStatus() const { return status_; }
    void setStatus( StatusType status ) { status_ = status; }
    virtual std::string toString() const {
        char buf[64];
        snprintf( buf, sizeof(buf), "seqNum=%d %s status=%s",
                  seqNum_, typeToString(), statusToString(status_) );
        return buf;
    }
    virtual void clear() { status_ = UNKNOWN; }
    virtual bool isValid() const { return true; }
    virtual bool isPing() const { return false; }

    virtual bool visit( ResponseVisitor& visitor ) throw (PvapException) = 0;

    bool correspondsRequest( const Request& request ) const {
        return ( request.getSeqNum() == getSeqNum() && request.matchResponseType(*this) );
    }

    // for serialization
    uint8_t getStatusValue() const { return uint8_t(status_); }
    void setStatusValue( uint8_t val ) { status_ = StatusType(val); }

protected:
    virtual const char* typeToString() const = 0;

private:
    // Response( const Response& );
    // Response& operator = ( const Response& );

protected:
    uint32_t   seqNum_;
    StatusType status_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_RESPONSE_H */
