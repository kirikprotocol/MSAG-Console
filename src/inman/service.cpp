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
	server = new Server(host, port);
	server->Start();

	smsc_log_debug( logger, "Open session" );
	session = factory->openSession(SSN, ssf_addr, scf_addr );
	assert( session );

	session->addListener( this );
}

Service::~Service()
{
	Factory* factory = Factory::getInstance();
	assert( factory );

	smsc_log_debug( logger, "Close session" );
	session->removeListener( this );
	factory->closeSession( session );

	smsc_log_debug( logger, "Stop server" );
	server->Stop();
	delete server;

	smsc_log_debug( logger, "Stop dispatcher" );
	dispatcher->Stop();
	delete dispatcher;
}

void Service::startOriginating()
{
	smsc_log_info( logger, "Start originating" );
	TcapDialog*  dialog  = session->openDialog( 0 );
	assert( dialog );
	Billing* billing = new Billing( dialog, smsc::inman::comp::DeliveryMode_Originating );
	assert( billing );
	add( billing );
	billing->initialDPSMS();
}

void Service::startTerminating()
{
	smsc_log_info( logger, "Start terminating" );
	TcapDialog*  dialog  = session->openDialog( 0 );
	assert( dialog );
	Billing* billing = new Billing( dialog, smsc::inman::comp::DeliveryMode_Terminating );
	assert( billing );
	add( billing );
	billing->initialDPSMS();
}

void Service::add(Billing* worker)
{
	assert( worker );
	workers.push_back( worker );
}

void Service::onDialogBegin(TcapDialog* dlg)
{
	assert( dlg );
	smsc_log_info( logger, "Dialog 0x%X created", dlg->getId() );
};

void Service::onDialogEnd(TcapDialog* dlg)
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

} // namespace inmgr
} // namespace smsc
