#include "SACC_Defs.h"
#include "SACC_SyncQueue.h"
#include "SACC_EventSender.h"

namespace scag{
namespace stat{
namespace sacc{

using smsc::util::Exception;
using smsc::logger::Logger;

EventSender::EventSender()
{

}

EventSender::~EventSender()
{

}

EventSender::EventSender(SyncQeuue * q,bool * bf)
{
	pQueue = q;
	bStop  = bf;
}

bool EventSender::processEvent(void *ev)
{

	uint16_t EventType;
	memcpy(&EventType,ev,2);

	switch(EventType)
	{
		case SACC_SEND_TRANSPORT_EVENT:
			{
				delete (SACC_TRAFFIC_INFO_EVENT_t*)ev;
				 
			}
			break;

		case SACC_SEND_BILL_EVENT:
			{
				delete (SACC_BILLING_INFO_EVENT_t*) ev;
				
			}
			break;

		case SACC_SEND_ALARM_EVENT:
			break;

		case SACC_SEND_OPERATOR_NOT_FOUND_ALARM:
			{
				delete (SACC_OPERATOR_NOT_FOUND_ALARM_t*) ev;
				
			}
			break;

		case SACC_SEND_SESSION_EXPIRATION_TIME_ALARM:
			{
				delete (SACC_SESSION_EXPIRATION_TIME_ALARM_t*) ev;
				
			}
			break;

		case SACC_SEND_ALARM_MESSAGE:
			{
				delete (SACC_ALARM_MESSAGE_t*) ev;
				
			}
			break;
			
		default:
			break;
	}

	
}

bool EventSender::checkQueue()
{
	void * ev;
	if(pQueue.Pop(ev,1000))
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
	while(!(*bStop))
	{
		checkQueue();
	}
}

bool EventSender::connect(std::string host, int port)
{

	
	return true;
}


}//sacc
}//stat
}//scag

