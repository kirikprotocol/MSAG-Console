#ifndef _SCAG_EXC_IOEXCEPTION_H_
#define _SCAG_EXC_IOEXCEPTION_H_

#include "SCAGExceptions.h"

namespace scag2 {
namespace exceptions {

class IOException : public SCAGException
{
public:
    IOException(const std::exception& exc) : SCAGException(exc.what()) {}
    IOException() : SCAGException() {}
    IOException( const std::string& exc ) : SCAGException(exc) {}
    IOException( const char* fmt,... ) : SCAGException() {
        SMSC_UTIL_EX_FILL(fmt);
    }
    virtual ~IOException() throw() {}
};

}
}

namespace scag {
namespace exceptions {
using scag2::exceptions::IOException;
}
}

#endif 

