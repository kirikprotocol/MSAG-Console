#ifndef _SCAG_PVSS_COMMON_PVSSEXCEPTION_H
#define _SCAG_PVSS_COMMON_PVSSEXCEPTION_H

#include "scag/exc/SCAGExceptions.h"
#include "StatusType.h"

namespace scag2 {
namespace pvss {

class PvssException : public exceptions::SCAGException, public StatusType
{
protected:
    PvssException( Type type ) : type_(type) {}

public:
    PvssException() : type_(UNKNOWN) {}
    PvssException( const std::string& msg, Type t ) : exceptions::SCAGException(msg), type_(t) {}
    PvssException( Type type, const char* fmt, ... ) : type_(type) {
        SMSC_UTIL_EX_FILL(fmt);
    }
    // default copy ctor is ok
    // PvssException( const PvssException );

    Type getType() const { return type_; }
    
    virtual const char* what() const throw () {
        if ( message.empty() ) return StatusType::statusMessage(type_);
        return message.c_str();
    }

private:
    Type type_;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_COMMON_PVAPEXCEPTION_H */
