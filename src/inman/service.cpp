static char const ident[] = "$Id$";
#include <assert.h>

#include "service.hpp"

namespace smsc  {
namespace inman {

Service::Service( const char* ssf_addr, const char* scf_addr, const char* host, int port, int SSN)
	: logger( Logger::getInstance("smsc.inman.Service") )
	, session( 0 )
	, dispatcher( 0 )
	, server( 0 )
{
	Factory* factory = Factory::getInstance();
	assert( factory );

	smsc_log_debug( logger, "Start dispatcher" );
	dispatcher = new Dispatcher();
	dispatcher->Start();

	smsc_log_debug( logger, "Start server" );
	server = new Server( host, port );
	server->addListener( this );
	server->Start();

	smsc_log_debug( logger, "Open session" );
	session = factory->openSession(SSN, ssf_addr, scf_addr );
	assert( session );

//	session->addListener( this );
}

Service::~Service()
{
	Factory* factory = Factory::getInstance();
	assert( factory );

	smsc_log_debug( logger, "Close session" );
//	session->removeListener( this );
	factory->closeSession( session );

	smsc_log_debug( logger, "Stop server" );
	server->removeListener( this );
	server->Stop();
	delete server;

	smsc_log_debug( logger, "Stop dispatcher" );
	dispatcher->Stop();
	delete dispatcher;
}

void Service::onConnectOpened(Server*, Connect* connect)
{
	assert( connect );
	smsc_log_debug( logger, "New connection opened" );
	connect->addListener( this );
}

void Service::onConnectClosed(Server*, Connect* connect)
{
	assert( connect );
	smsc_log_debug( logger, "Connection closed" );
	connect->removeListener( this );
}

void Service::onCommandReceived(Connect* conn, InmanCommand* cmd)
{
	assert( cmd );	
	int dlgId = cmd->getDialogId();
	smsc_log_debug( logger, "Command for dialog 0x%X", dlgId );
	BillingMap::iterator it = workers.find( dlgId );
	if( it == workers.end() )
	{
		Billing* bill = new Billing( session, conn );
		workers.insert( BillingMap::value_type( dlgId, bill ) );
		cmd->handle( bill );
	}
	else
	{
		cmd->handle( (*it).second );
	}
}


/*
void Service::onDialogEnd(Dialog* dlg)
{
	assert( dlg );
	for( WorkersList::iterator i = workers.begin(); i != workers.end(); i++ )
	{
		Billing* billing = (*i);
		assert( billing );
		if( billing->getDialog() == dlg )
		{
			//workers.erase( i );
			//delete billing;
			smsc_log_info( logger, "Dialog 0x%X released", dlg->getId() );
			break;
		}
	}
};
*/
} // namespace inmgr
} // namespace smsc
