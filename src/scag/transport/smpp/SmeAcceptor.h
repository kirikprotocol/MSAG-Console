#ifndef SCAG_TRANSPORT_SMPP_ACCEPTOR
#define SCAG_TRANSPORT_SMPP_ACCEPTOR

#include "SmppProxy.h"

namespace scag { namespace transport { namespace smpp 
{

    class SmeAcceptor : public Thread
    {
    private:

        int     port;

    public:

        SmeAcceptor(int _port) : Thread(), port(_port) {};
        ~SmeAcceptor();
    };

}}}

#endif // SCAG_TRANSPORT_SMPP_ACCEPTOR

