#ifndef _EYELINE_SMPP_CLIENTSESSION_H
#define _EYELINE_SMPP_CLIENTSESSION_H

#include "eyeline/smpp/transport/SessionBase.h"
#include "ClientSmeInfo.h"

namespace eyeline {
namespace smpp {

class SmppConnector;
class ClientSocket;

class ClientSession : public SessionBase
{
public:
    ClientSession( const char* sessionId,
                   PduListener* listener = 0 );

    void connect( SmppConnector& conn, ClientSocket& socket );
    void addSmeInfo( SmppConnector& conn, const ClientSmeInfo& info,
                     size_t maxrsize, size_t maxwqueue );

    virtual void receivePdu( Socket& socket, const PduInputBuf& pdu );
};

}
}


#endif
