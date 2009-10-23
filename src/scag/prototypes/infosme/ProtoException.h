#ifndef _SCAG_PROTOTYPES_INFOSME_PROTOEXCEPTION_H
#define _SCAG_PROTOTYPES_INFOSME_PROTOEXCEPTION_H

#include "util/Exception.hpp"

namespace scag2 {
namespace prototypes {
namespace infosme {

class ProtoException : public smsc::util::Exception
{
public:
    ProtoException( const char* fmt, ... ) {
        SMSC_UTIL_EX_FILL(fmt);
    }
};


class MessageState
{
public:
    enum {
            UNKNOWN = -1,
            OK = 0,
            LIMITED,
            FAIL
    };

    static const char* stateToString( int state ) {
        switch (state) {
        case UNKNOWN : return "UNK";
        case OK : return "SENT";
        case LIMITED : return "LIMIT";
        case FAIL : return "FAIL";
        default : return "???";
        }
    }
};

}
}
}

#endif
