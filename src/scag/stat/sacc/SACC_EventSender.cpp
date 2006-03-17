#include "SACC_Defs.h"
#include "SACC_SyncQueue.h"
#include "SACC_EventSender.h"
#include "SACC_Events.h"
#include "stat/Statistics.h"

#include <logger/Logger.h>

using namespace scag::stat;
using namespace scag::stat::Counters;

namespace scag{
namespace stat{
namespace sacc{


EventSender::EventSender()
{
	bStarted  = 0;
	logger = 0;
	bConnected=false;
	Host="";
	Port=0;
}

EventSender::~EventSender()
{

}

void EventSender::makeTransportEvent(SaccStatistics * st,SACC_TRAFFIC_INFO_EVENT_t * ev)
{

}
void EventSender::init(std::string& host,int port,int timeout,bool * bf,smsc::logger::Logger * lg)
{

	if(!lg)
		throw Exception("EventSender::init logger is 0");

	if(!bf)
		throw Exception("EventSender::init start-stop flag is 0");

	bStarted  = bf;
	logger = lg;
	Host = host;
	Port= port;
	bConnected=false;
	smsc_log_debug(logger,"EventSender::init confuration succsess.");
}

bool EventSender::processEvent(SaccStatistics *ev)
{

	smsc_log_debug(logger,"EventSender::Execute Sacc stat event processed from queue addr=%s",ev->abonent_addr);
	SaccSocket.Write((char*)&ev->command_id,2);

return true;
}

bool EventSender::checkQueue()
{
	SaccStatistics * ev;
	if(eventsQueue.Pop(ev,1000))
	{
		if(ev)
		{	
			processEvent(ev);
			delete ev;
		}

		return true;
	}
	else
	{
		return false;
	}
	

}

bool EventSender::retrieveConnect()
{
	fd_set read;
    FD_ZERO( &read );
    FD_SET( SaccSocket.getSocket(), &read );

    struct timeval tv;
    tv.tv_sec = 0; 
    tv.tv_usec = 500;

    int n = select(  SaccSocket.getSocket()+1, &read, 0, 0, &tv );
	
	if(n>0)
	{
		bConnected=true;
		return true;
	}
	else
	{

		if(connect(Host,Port,100))
		{
			bConnected=true;
			return true;
		}
	}

	bConnected=false;
	return false;

		
}

int EventSender::Execute()
{
	while( *bStarted)
	{
		if(retrieveConnect())
		{
			checkQueue();
		}
		else
		{
			sleep(500);
		}
	}
	return 1;

}

bool EventSender::connect(std::string host, int port,int timeout)
{

	if(SaccSocket.Init(host.c_str(),port,timeout)==-1)
	{
		smsc_log_error(logger,"EventSender::connect Failed to init socket\n");
		return false;
	}
		  
	if(SaccSocket.Connect()==-1)
	{
		smsc_log_error(logger,"EventSender::connect Failed to connect\n");
		return false;
	}

	bConnected=true;

	smsc_log_debug(logger,"EventSender::connect succsess to %s:%d\n",host.c_str(),port);
	return true;
}

bool EventSender::isActive()
{
   return bConnected;
}


void EventSender::Start()
{
	Thread::Start();
}

void EventSender::Put(const SaccStatistics& ev)
{
	SaccStatistics * pSt= new SaccStatistics(ev);
	eventsQueue.Push(pSt); 
}


}//sacc
}//stat
}//scag



