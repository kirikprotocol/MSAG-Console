#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_CONNECT__
#define __SMSC_INMAN_INAP_CONNECT__

#include "dispatcher.hpp"

namespace smsc  {
namespace inman {
namespace inap  {


class Connect : public SocketListener
{
    public:
		Connect(Socket* socket);
		virtual ~Connect();

		SOCKET getHandle();
		void   process(Dispatcher*);

    protected:
    	Socket* socket;
        Logger*	logger;   	
};

}
}
}

#endif
