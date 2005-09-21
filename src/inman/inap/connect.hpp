#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_CONNECT__
#define __SMSC_INMAN_INAP_CONNECT__

#include "dispatcher.hpp"
#include "inman/interaction/messages.hpp"
#include "inman/interaction/serializer.hpp"

using smsc::inman::interaction::InmanHandler;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ObjectPipe;
using smsc::inman::interaction::DeliverySmsResult;

namespace smsc  {
namespace inman {
namespace inap  {

class Server;

class Connect : public InmanHandler
{
    public:
		Connect(Socket* socket);
		virtual ~Connect();

		Socket* getSocket();
		bool    process(Server*);

		virtual void onChargeSms(ChargeSms* sms);	
		virtual void onDeliverySmsResult(DeliverySmsResult* sms);

    protected:
    	Socket* 	socket;
    	ObjectPipe* pipe;
        Logger*		logger;   	
};

}
}
}

#endif
