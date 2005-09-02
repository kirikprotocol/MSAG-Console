#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_SERVER__
#define __SMSC_INMAN_INAP_SERVER__

#include "dispatcher.hpp"
#include "connect.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

class Server : public SocketListener
{
    public:
    	Server(const char* szHost, int nPort);
    	virtual ~Server();

    	SOCKET 	getHandle();
    	void   	process(Dispatcher*);

    protected:
    	Socket 				socket;
        Logger*				logger;

};

}
}
}

#endif
