#ifndef SCAG_BILL_EWALLET_EXCEPTION_H
#define SCAG_BILL_EWALLET_EXCEPTION_H

#include "scag/exc/SCAGExceptions.h"
#include "Status.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class Exception : public exceptions::SCAGException
{
public:
    Exception( const std::string& msg, uint8_t status ) :
    SCAGException( msg ), status_(status) {}
    
    Exception( uint8_t status, const char* fmt, ... ) :
    SCAGException(), status_(status) {
        SMSC_UTIL_EX_FILL(fmt);
    }

    uint8_t getStatus() const { return status_; }

private:
    uint8_t status_;
};

}
}
}

#endif
