#ifndef _EYELINE_SMPP_PDULISTENER_H
#define _EYELINE_SMPP_PDULISTENER_H

#include <memory>

namespace eyeline {
namespace smpp {

class Socket;
class PduInputBuf;

/// an instance of the class will be notified when pdu is received on the socket
class PduListener
{
protected:
    virtual ~PduListener() {}
public:
    virtual void receivePdu( Socket& socket, const PduInputBuf& pdu ) = 0;
};

}
}


#endif
