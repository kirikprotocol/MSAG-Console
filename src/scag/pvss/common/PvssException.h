#ifndef _SCAG_PVSS_COMMON_PVSSEXCEPTION_H
#define _SCAG_PVSS_COMMON_PVSSEXCEPTION_H

#include "scag/exc/SCAGExceptions.h"

namespace scag2 {
namespace pvss {

class PvssException : public exceptions::SCAGException
{
public:
    /// see also pvss/api/packets/StatusType.h
    enum Type {

            // business logic messages (infos?)
            INVALID_KEY         = 0x03,
            INVALID_SCOPE       = 0x04,

            // protocol messages
            BAD_REQUEST         = 0x10,
            BAD_RESPONSE        = 0x11,
            UNKNOWN_RESPONSE    = 0x12,
            UNEXPECTED_RESPONSE = 0x13,

            // severe failures
            IO_ERROR            = 0x20,
            SERVER_BUSY         = 0x23,
            REQUEST_TIMEOUT     = 0x24,
            CONNECT_FAILED      = 0x25,
            NOT_CONNECTED       = 0x26,
            CLIENT_BUSY         = 0x27,
            CONFIG_INVALID      = 0x28,

            UNKNOWN             = 0xff
    };

    static const char* typeToString( Type t );

protected:
    PvssException( Type type ) : type_(type) {}

public:
    PvssException( Type type, const char* fmt, ... ) : type_(type) {
        SMSC_UTIL_EX_FILL(fmt);
    }
    // default copy ctor is ok
    // PvssException( const PvssException );

    Type getType() const { return type_; }
    
private:
    Type type_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_COMMON_PVAPEXCEPTION_H */
