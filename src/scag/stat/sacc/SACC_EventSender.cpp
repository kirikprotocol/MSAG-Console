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
	pQueue = 0;
	bStarted  = 0;
	logger = 0;
}

EventSender::~EventSender()
{

}

void EventSender::init(std::string& host,int port,int timeout, 
					   SyncQueue<SaccStatistics*> * q,bool * bf,smsc::logger::Logger * lg)
{

	if(!q)
		throw Exception("EventSender::init Event queue is 0!");

	if(!lg)
		throw Exception("EventSender::init logger is 0");

	if(!bf)
		throw Exception("EventSender::init start-stop flag is 0");

	pQueue = q;
	bStarted  = bf;
	logger = lg;

	smsc_log_debug(logger,"...connecting to SACC server host:%s port%d",host.c_str(),port);
	
	if(!connect(host,port,timeout))
	{
		throw Exception("Cannot connect to SACC ");
	}

	smsc_log_debug(logger,"EventSender::init confuration and connection succsess.");
}

bool EventSender::processEvent(void *ev)
{

	smsc_log_debug(logger,"EventSender::Execute Sacc stat event processed from queue");

return true;
}

bool EventSender::checkQueue()
{
	SaccStatistics * ev;
	if(pQueue->Pop(ev,1000))
	{
		processEvent(ev);
		return true;
	}
	else
	{
		return false;
	}
	

}

int EventSender::Execute()
{
	while( *bStarted)
	{
	 	checkQueue();
	}
	return 1;

}

bool EventSender::connect(std::string host, int port,int timeout)
{
/*	if(SaccSocket.Init(host.c_str(),port,timeout)==-1)
	{
		smsc_log_error(logger,"EventSender::connect Failed to init socket\n");
		return false;
	}
		  
	if(SaccSocket.Connect()==-1)
	{
		smsc_log_error(logger,"EventSender::connect Failed to connect\n");
		return false;
	}

*/	
	return true;
}



void EventSender::Start()
{
	Thread::Start();
}

}//sacc
}//stat
}//scag


