#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_CONNECT__
#define __SMSC_INMAN_INAP_CONNECT__

#include "dispatcher.hpp"

namespace smsc  {
namespace inman {
namespace inap  {

class Server;

class Connect
{
    public:
		Connect(Socket* socket);
		virtual ~Connect();

		Socket* getSocket();
		bool    process(Server*);

    protected:
    	Socket* socket;
        Logger*	logger;   	
};

}
}
}

#endif
