#ifndef _SCAG_PVSS_COMMON_PVSSEXCEPTION_H
#define _SCAG_PVSS_COMMON_PVSSEXCEPTION_H

#include "scag/exc/SCAGExceptions.h"

namespace scag2 {
namespace pvss {

class PvssException : public exceptions::SCAGException
{
public:
    enum Type {
            UNKNOWN            = 0,
            CONNECT_FAILED,
            NOT_CONNECTED,
            SERVER_BUSY,
            CLIENT_BUSY,

            IO_ERROR,
            REQUEST_TIMEOUT,

            UNKNOWN_RESPONSE,
            UNEXPECTED_RESPONSE,
            BAD_REQUEST,
            BAD_RESPONSE,

            INVALID_KEY,
            INVALID_SCOPE,

            CONFIG_INVALID
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
