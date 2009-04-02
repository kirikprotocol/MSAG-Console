#ifndef _SCAG_PVSS_BASE_STATUSTYPE_H
#define _SCAG_PVSS_BASE_STATUSTYPE_H

namespace scag2 {
namespace pvss {

///
/// Response status
///
class StatusType 
{
public:
    enum {
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


protected:    
    static const char* statusToString( uint8_t stat )
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

    static const char* statusMessage( uint8_t stat )
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

    static inline bool statusIsInfo( uint8_t stat ) {
        return (stat & 0xf0) == 0x00;
    }
    static inline bool statusIsWarn( uint8_t stat ) {
        return (stat & 0xf0) == 0x10;
    }
    static inline bool statusIsError( uint8_t stat ) {
        return (stat & 0xf0) == 0x20;
    }

protected:
    /// NOTE: they are all protected to prevent nasty things
    StatusType() {}
    ~StatusType() {}
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_STATUSTYPE_H */
