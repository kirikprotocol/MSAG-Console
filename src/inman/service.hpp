#ident "$Id$"

#ifndef __SMSC_INMAN_SERVICE__
#define __SMSC_INMAN_SERVICE__

#include <map>

#include "logger/Logger.h"
#include "inman/common/console.hpp"
#include "inman/inap/dispatcher.hpp"
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
using smsc::inman::inap::Factory;
using smsc::inman::inap::Session;
using smsc::inman::inap::SessionListener;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::Inap;

using smsc::logger::Logger;

namespace smsc  {
namespace inman {

class Service : public SessionListener, public ServerListener
{
		typedef std::list<Billing*> WorkersList;

	public:

		Service( const char* ssf_addr, const char* scf_addr, const char* host, int port, int SSN);
		virtual ~Service();

		virtual void startOriginating();
		virtual void startTerminating();

		virtual void onConnectOpened(Connect* connect);
		virtual void onConnectClosed(Connect* connect);

		virtual void onDialogBegin(Dialog* dlg);
		virtual void onDialogEnd(Dialog* dlg);

	protected:
		virtual void add(Billing* worker);

	private:

		WorkersList workers;
		Logger*	 	logger;
		Session* 	session;
		Dispatcher* dispatcher;
		Server*	    server;

};

}
}


#endif
