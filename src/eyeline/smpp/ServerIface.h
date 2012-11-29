#ifndef _EYELINE_SMPP_SERVERIFACE_H
#define _EYELINE_SMPP_SERVERIFACE_H

#include "Session.h"

namespace eyeline {
namespace smpp {

class ServerIface
{
public:
    virtual ~ServerIface() {}

    /// get session by id
    virtual bool getSession( const char* sessionId, SessionPtr& ptr ) = 0;
};

}
}

#endif
