#ifndef _EYELINE_SMPP_CLIENTSOCKET_H
#define _EYELINE_SMPP_CLIENTSOCKET_H

#include "eyeline/smpp/transport/Socket.h"
#include "ClientSmeInfo.h"

namespace eyeline {
namespace smpp {

class ClientSocket : public Socket
{
public:
    ClientSocket( const ClientSmeInfo& info, size_t maxrsize, size_t maxwqueue ) :
    Socket(0,maxrsize,maxwqueue), info_(info), inited_(false) {}

    inline const ClientSmeInfo& getSmeInfo() const { return info_; }

    void connect();
    // void reconnect();

private:
    ClientSmeInfo info_;
    bool          inited_;
};

}
}

#endif
