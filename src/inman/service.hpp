#ident "$Id$"

#ifndef __SMSC_INMAN_SERVICE__
#define __SMSC_INMAN_SERVICE__

#include <map>

#include "logger/Logger.h"
#include "inman/common/console.hpp"
#include "inman/inap/dispatcher.hpp"
#include "inman/inap/connect.hpp"
#include "inman/inap/server.hpp"
#include "inman/inap/factory.hpp"
#include "inman/inap/session.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/inap/inap.hpp"

#include "billing.hpp"

using smsc::inman::inap::Dispatcher;
using smsc::inman::inap::Server;
using smsc::inman::inap::ServerListener;
using smsc::inman::inap::Connect;
using smsc::inman::inap::ConnectListener;
using smsc::inman::inap::Factory;
using smsc::inman::inap::Session;
using smsc::inman::inap::SessionListener;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::Inap;

using smsc::logger::Logger;

namespace smsc  {
namespace inman {

class Service : public ServerListener, ConnectListener
{
		typedef std::map<int, Billing*> BillingMap;

	public:

		Service( const char* ssf_addr, const char* scf_addr, const char* host, int port, int SSN);
		virtual ~Service();
                //ServerListener interface
		virtual void onConnectOpened(Server*, Connect*);
		virtual void onConnectClosed(Server*, Connect*);
//		virtual void onCommandReceived(Connect*, InmanCommand*);
                //ConnectListener interface
		virtual void onCommandReceived(Connect*, SerializableObject*);

		virtual void billingFinished(Billing* bill);

		virtual void start();
		virtual void stop();

	private:

		BillingMap	workers;
		Logger*	 	logger;
		Session* 	session;
		Dispatcher* dispatcher;
		Server*	    server;

};

}
}


#endif
