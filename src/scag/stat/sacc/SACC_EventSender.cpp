#include "SACC_Defs.h"
#include "SACC_SyncQueue.h"
#include "SACC_EventSender.h"
#include "SACC_Events.h"

#include <logger/Logger.h>

namespace scag{
namespace stat{
namespace sacc{


EventSender::EventSender()
{

}

EventSender::~EventSender()
{

}

EventSender::EventSender(SyncQueue<void*> * q,bool * bf,smsc::logger::Logger * lg)
{
	pQueue = q;
	bStarted  = bf;
	logger = lg;
}

bool EventSender::processEvent(void *ev)
{

	uint16_t EventType;
	memcpy(&EventType,ev,2);

	switch(EventType)
	{
		case SACC_SEND_TRANSPORT_EVENT:
			{
				SACC_TRAFFIC_INFO_EVENT_t eTraffic;
				memcpy(&eTraffic,ev,sizeof(SACC_TRAFFIC_INFO_EVENT_t));
				//performTrafficEvent(&eTraffic);
				smsc_log_debug(logger,"EventSender:SACC_SEND_TRANSPORT_EVENT detected");

				delete (SACC_TRAFFIC_INFO_EVENT_t*)ev;
			}
			break;

		case SACC_SEND_BILL_EVENT:
			{
				SACC_BILLING_INFO_EVENT_t eBilling;
				memcpy(&eBilling,ev,sizeof(SACC_BILLING_INFO_EVENT_t));
				smsc_log_debug(logger,"EventSender:SACC_SEND_BILL_EVENT detected");

				delete (SACC_BILLING_INFO_EVENT_t*) ev;
			}
			break;

		case SACC_SEND_ALARM_EVENT:
				smsc_log_debug(logger,"Warning: EventSender: wrong sacc send alarm command detected!");
			break;

		case SACC_SEND_OPERATOR_NOT_FOUND_ALARM:
			{
				SACC_OPERATOR_NOT_FOUND_ALARM_t eOprNotFound;
				memcpy(&eOprNotFound,ev,sizeof(SACC_OPERATOR_NOT_FOUND_ALARM_t));
				smsc_log_debug(logger,"EventSender:SACC_SEND_OPERATOR_NOT_FOUND_ALARM detected");

				delete (SACC_OPERATOR_NOT_FOUND_ALARM_t*) ev;
			}
			break;

		case SACC_SEND_SESSION_EXPIRATION_TIME_ALARM:
			{
				SACC_SESSION_EXPIRATION_TIME_ALARM_t eSessionExpiration;
				memcpy(&eSessionExpiration,ev,sizeof(SACC_SESSION_EXPIRATION_TIME_ALARM_t));
				smsc_log_debug(logger,"EventSender:SACC_SEND_SESSION_EXPIRATION_TIME_ALARM detected");

				delete (SACC_SESSION_EXPIRATION_TIME_ALARM_t*) ev;
			}
			break;

		case SACC_SEND_ALARM_MESSAGE:
			{
				SACC_ALARM_MESSAGE_t eAlarmMessage;
				memcpy(&eAlarmMessage,ev,sizeof(SACC_ALARM_MESSAGE_t));
				smsc_log_debug(logger,"EventSender:SACC_SEND_ALARM_MESSAGE detected");
				
				delete (SACC_ALARM_MESSAGE_t*) ev;
			}
			break;
			
		default:
			throw Exception("Error: EventSender: unknown sacc send command detected!");
			break;
	}

	
}

bool EventSender::checkQueue()
{
	void * ev;
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

bool EventSender::connect(std::string host, int port)
{
	if(SaccSocket.Init(host,port,timeout)==-1)
	{
		smsc_log_error(logger,"EventSender::connect Failed to init socket\n");
		return false;
	}
		  
	if(SaccSocket.Connect()==-1)
	{
		smsc_log_error(logger,"EventSender::connect Failed to connect\n");
		return false;
	}

	
	return true;
}

bool EventSender::performTrafficEvent(SACC_TRAFFIC_INFO_EVENT_t *pTraffic)
{
	if(!pTraffic)
		return false;

	return true;
}

void EventSender::Stop()
{
	bStarted=false;
}

void EventSender::Start()
{
	Thread::Start();
}

}//sacc
}//stat
}//scag


