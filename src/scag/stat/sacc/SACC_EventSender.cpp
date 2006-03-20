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

bool EventSender::processEvent(void *ev)
{

 smsc_log_debug(logger,"EventSender::Execute Sacc stat event processed from queue addr=0x%X",ev);
	
 uint16_t evType=0;

 memcpy(&evType,ev,sizeof(uint16_t)); 		

	switch(evType) 
	{
	case SaccEventsCommandIds::sec_transport:
		{
			SACC_TRAFFIC_INFO_EVENT_t e;
			memcpy(&e,ev,sizeof(SACC_TRAFFIC_INFO_EVENT_t));
			performTransportEvent(e);
		}
 		break;
	case SaccEventsCommandIds::sec_bill:
		{
			SACC_BILLING_INFO_EVENT_t e;
			memcpy(&e,ev,sizeof(SACC_BILLING_INFO_EVENT_t));
			performBillingEvent(e);
		}
 		break;
	case SaccEventsCommandIds::sec_alarm:
		{
			SACC_ALARM_MESSAGE_t e;
			memcpy(&e,ev,sizeof(SACC_ALARM_MESSAGE_t));
			performAlarmEvent(e);
		}
		break;
	case SaccEventsCommandIds::sec_session_expired:
		{
			SACC_SESSION_EXPIRATION_TIME_ALARM_t e;
			memcpy(&e,ev,sizeof(SACC_SESSION_EXPIRATION_TIME_ALARM_t));
			performSessionExpiredEvent(e);
		}
 		break;
	case SaccEventsCommandIds::sec_operator_not_found:
		{
			SACC_OPERATOR_NOT_FOUND_ALARM_t e;
			memcpy(&e,ev,sizeof(SACC_OPERATOR_NOT_FOUND_ALARM_t));
			performOperatorNotFoundEvent(e);
		}
 		break;


	 default:
	 }
 return true;
}

bool EventSender::checkQueue()
{
	void * ev;
	if(eventsQueue.Pop(ev,100))
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
	
	if(SaccSocket.canWrite())
	{
		bConnected=true;
		return true;
	}
	else
	{
		SaccSocket.Abort();
		
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
			sleep(100);
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
 
void EventSender::Put(const SACC_TRAFFIC_INFO_EVENT_t& ev)
{
	SACC_TRAFFIC_INFO_EVENT_t* pEv = new SACC_TRAFFIC_INFO_EVENT_t(ev);
		smsc_log_debug(logger,"EventSender::put SACC_TRAFFIC_INFO_EVENT_t addr=0x%X",pEv);
	eventsQueue.Push(pEv); 
}
void EventSender::Put(const SACC_BILLING_INFO_EVENT_t& ev)
{


	SACC_BILLING_INFO_EVENT_t* pEv = new SACC_BILLING_INFO_EVENT_t(ev);
		smsc_log_debug(logger,"EventSender::put SACC_BILLING_INFO_EVENT_t addr=0x%X",pEv);
	eventsQueue.Push(pEv);
}
void EventSender::Put(const SACC_ALARM_MESSAGE_t & ev)
{
	SACC_ALARM_MESSAGE_t* pEv = new SACC_ALARM_MESSAGE_t(ev);
		smsc_log_debug(logger,"EventSender::put SACC_ALARM_MESSAGE_t addr=0x%X",pEv);
	eventsQueue.Push(pEv);
}
void EventSender::Put(const  SACC_SESSION_EXPIRATION_TIME_ALARM_t& ev)
{
	SACC_SESSION_EXPIRATION_TIME_ALARM_t* pEv = new SACC_SESSION_EXPIRATION_TIME_ALARM_t(ev);
		smsc_log_debug(logger,"EventSender::put SACC_SESSION_EXPIRATION_TIME_ALARM_t addr=0x%X",pEv);
	eventsQueue.Push(pEv);
}
void EventSender::Put(const   SACC_OPERATOR_NOT_FOUND_ALARM_t& ev)
{
	SACC_OPERATOR_NOT_FOUND_ALARM_t* pEv = new SACC_OPERATOR_NOT_FOUND_ALARM_t(ev);
		smsc_log_debug(logger,"EventSender::put SACC_OPERATOR_NOT_FOUND_ALARM_t addr=0x%X",pEv);
	eventsQueue.Push(pEv)
}


void EventSender::performTransportEvent(const SACC_TRAFFIC_INFO_EVENT_t& e)
{
	//std::basic_ostream buff;
	//buff << e.Header.sEventType << e.Header.pAbonentNumber;
}
void EventSender::performBillingEvent(const SACC_BILLING_INFO_EVENT_t& e)
{
}
void EventSender::performAlarmEvent(const SACC_ALARM_MESSAGE_t& e)
{
}
void EventSender::performSessionExpiredEvent(const SACC_SESSION_EXPIRATION_TIME_ALARM_t& e)
{
}
void EventSender::performOperatorNotFoundEvent(const SACC_OPERATOR_NOT_FOUND_ALARM_t& e)
{
}

}//sacc
}//stat
}//scag



