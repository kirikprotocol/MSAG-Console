#ifndef _EYELINE_SMPP_CLIENTIFACE_H
#define _EYELINE_SMPP_CLIENTIFACE_H

#include "Session.h"

namespace eyeline {
namespace smpp {

class ClientIface
{
public:
    virtual ~ClientIface() {}

    /// get session by id
    virtual bool getSession( const char* sessionId, SessionPtr& ptr ) = 0;
};

}
}

#endif
