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
	smsc_log_debug( logger, "Create service" );
	
	Factory* factory = Factory::getInstance();
	assert( factory );

	dispatcher = new Dispatcher();

	server = new Server( host, port );
	server->addListener( this );

	session = factory->openSession(SSN, ssf_addr, scf_addr );
	assert( session );
}

Service::~Service()
{
	smsc_log_debug( logger, "Release service" );
	Factory* factory = Factory::getInstance();
	assert( factory );
	smsc_log_debug( logger, "ReleaseSession" );
	factory->closeSession( session );
	smsc_log_debug( logger, "Remove lisetener" );
	server->removeListener( this );
	smsc_log_debug( logger, "Delete server" );
	delete server;
	smsc_log_debug( logger, "Delete dispatcher" );
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

void Service::billingFinished(Billing* bill)
{
	assert( bill );
	for( BillingMap::iterator it = workers.begin(); it != workers.end(); it++)
	{
		if( (*it).second == bill )
		{
			workers.erase( it );
			delete bill;
			smsc_log_error( logger, "Billing id=0x%X deleted", bill->getId() );
			return;
		}
	}
	smsc_log_error( logger, "Billing id=0x%X missed in list", bill->getId() );
}

void Service::onCommandReceived(Connect* conn, InmanCommand* cmd)
{
	assert( cmd );	
	int dlgId = cmd->getObjectId();
	smsc_log_debug( logger, "Command for billing id=0x%X", dlgId );
	BillingMap::iterator it = workers.find( dlgId );
	if( it == workers.end() )
	{
		Billing* bill = new Billing( this, dlgId, session, conn );
		workers.insert( BillingMap::value_type( dlgId, bill ) );
		cmd->handle( bill );
	}
	else
	{
		Billing* bill = (*it).second;
		cmd->handle( bill );
	}
}

void Service::start()
{
	smsc_log_debug( logger, "Start dispatcher" );
	dispatcher->Start();

	smsc_log_debug( logger, "Start server" );
	server->Start();
}

void Service::stop()
{
	smsc_log_debug( logger, "Stop server" );
	server->Stop();
	server->WaitFor();

	smsc_log_debug( logger, "Stop dispatcher" );
	dispatcher->Stop();
	dispatcher->WaitFor();
}

} // namespace inmgr
} // namespace smsc
