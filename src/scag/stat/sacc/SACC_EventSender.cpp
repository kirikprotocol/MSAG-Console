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
			delete (SACC_TRAFFIC_INFO_EVENT_t*)ev;
			performTransportEvent(e);
			
		}
 		break;
	case SaccEventsCommandIds::sec_bill:
		{
			SACC_BILLING_INFO_EVENT_t e;
			memcpy(&e,ev,sizeof(SACC_BILLING_INFO_EVENT_t));
			delete (SACC_BILLING_INFO_EVENT_t*)ev;
			performBillingEvent(e);
		}
 		break;
	case SaccEventsCommandIds::sec_alarm:
		{
			SACC_ALARM_MESSAGE_t e;
			memcpy(&e,ev,sizeof(SACC_ALARM_MESSAGE_t));
			delete (SACC_ALARM_MESSAGE_t*)ev;
			performAlarmEvent(e);
		}
		break;
	case SaccEventsCommandIds::sec_session_expired:
		{
			SACC_SESSION_EXPIRATION_TIME_ALARM_t e;
			memcpy(&e,ev,sizeof(SACC_SESSION_EXPIRATION_TIME_ALARM_t));
			delete (SACC_SESSION_EXPIRATION_TIME_ALARM_t*)ev;
			performSessionExpiredEvent(e);
		}
 		break;
	case SaccEventsCommandIds::sec_operator_not_found:
		{
			SACC_OPERATOR_NOT_FOUND_ALARM_t e;
			memcpy(&e,ev,sizeof(SACC_OPERATOR_NOT_FOUND_ALARM_t));
			delete (SACC_OPERATOR_NOT_FOUND_ALARM_t*)ev;
			performOperatorNotFoundEvent(e);
		}
 		break;

	 default:
		 break;
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
	eventsQueue.Push(pEv);
}


void EventSender::performTransportEvent(const SACC_TRAFFIC_INFO_EVENT_t& e)
{
	uint8_t * buffer = new uint8_t[sizeof(SACC_TRAFFIC_INFO_EVENT_t)];
	memcpy(buffer,&e.Header.sEventType,sizeof(uint16_t));
	buffer+=sizeof(uint16_t);
	memcpy(buffer,e.Header.pAbonentNumber,MAX_ABONENT_NUMBER_LENGTH);
	buffer+=MAX_ABONENT_NUMBER_LENGTH;
	memcpy(buffer,&e.Header.lDateTime,sizeof(uint64_t));
	buffer+=sizeof(uint64_t);

	memcpy(buffer,&e.iOperatorId,sizeof(uint32_t));
	buffer+=sizeof(uint32_t);

	memcpy(buffer,&e.Header.iServiceProviderId,sizeof(uint32_t));
	buffer+=sizeof(uint32_t);
	
	memcpy(buffer,&e.Header.iServiceId,sizeof(uint32_t));
	buffer+=sizeof(uint32_t);

	memcpy(buffer,e.pSessionKey,MAX_SESSION_KEY_LENGTH);
	buffer+=MAX_SESSION_KEY_LENGTH;

	memcpy(buffer,&e.Header.cProtocolId,sizeof(uint8_t));
	buffer+=sizeof(uint8_t);

	memcpy(buffer,&e.Header.cCommandId,sizeof(uint8_t));
	buffer+=sizeof(uint8_t);

	memcpy(buffer,&e.Header.sCommandStatus,sizeof(uint16_t));
	buffer+=sizeof(uint16_t);

	memcpy(buffer,e.pMessageText,MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t));
	buffer+=MAX_TEXT_MESSAGE_LENGTH*sizeof(uint16_t);

	memcpy(buffer,&e.cDirection,sizeof(uint8_t));
	
	SaccSocket.Write((char *)buffer,sizeof(SACC_TRAFFIC_INFO_EVENT_t));

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



