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
	Timeout=100;
	QueueLength=10000;
}

EventSender::~EventSender()
{

}


void EventSender::init(std::string& host,int port,int timeout,int queuelen,bool * bf,smsc::logger::Logger * lg)
{

	if(!lg)
		throw Exception("EventSender::init logger is 0");

	if(!bf)
		throw Exception("EventSender::init start-stop flag is 0");

	QueueLength=queuelen;
	bStarted  = bf;
	logger = lg;
	Host = host;
	Port= port;
	bConnected=false;
	Timeout=timeout;
	smsc_log_debug(logger,"EventSender::init confuration succsess.");
}

bool EventSender::processEvent(void *ev)
{
	
 uint16_t evType=0;

 memcpy(&evType,ev,sizeof(uint16_t)); 		

 smsc_log_debug(logger,"EventSender::Execute Sacc stat event (%d) processed from queue addr=0x%X",evType,ev);

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
	case SaccEventsCommandIds::sec_alarm_message:
		{
			SACC_ALARM_MESSAGE_t e;
			memcpy(&e,ev,sizeof(SACC_ALARM_MESSAGE_t));
			delete (SACC_ALARM_MESSAGE_t*)ev;
			performAlarmMessageEvent(e);
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
	case SaccEventsCommandIds::sec_alarm:
		{
			SACC_ALARM_t e;
			memcpy(&e,ev,sizeof(SACC_ALARM_t));
			delete (SACC_ALARM_t*)ev;
			performAlarmEvent(e);
		}
			
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
	EventMonitor e;
	while( *bStarted)
	{
		if(!bConnected)
		{
			SaccSocket.Abort();
			
			e.wait(Timeout);
			if(connect(Host,Port,100))
			{
				bConnected=true;
			}
		}
		
		checkQueue();

		
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
 
void EventSender::Put (const SACC_ALARM_t& ev)
{
	SACC_ALARM_t* pEv = new SACC_ALARM_t(ev);
	//	smsc_log_debug(logger,"EventSender::put SACC_TRAFFIC_INFO_EVENT_t addr=0x%X",pEv);
	if(eventsQueue.Push(pEv,QueueLength)==-1)
	{
		smsc_log_error(logger,"Error push to QOEUE for SACC EVENT? queue is Overflow!"); 
		delete pEv;
	}
}

void EventSender::Put(const SACC_TRAFFIC_INFO_EVENT_t& ev)
{
	SACC_TRAFFIC_INFO_EVENT_t* pEv = new SACC_TRAFFIC_INFO_EVENT_t(ev);
	//	smsc_log_debug(logger,"EventSender::put SACC_TRAFFIC_INFO_EVENT_t addr=0x%X",pEv);
	if(eventsQueue.Push(pEv,QueueLength)==-1)
	{
		smsc_log_error(logger,"Error push to QOEUE for SACC EVENT? queue is Overflow!"); 
		delete pEv;
	}
}
void EventSender::Put(const SACC_BILLING_INFO_EVENT_t& ev)
{

	SACC_BILLING_INFO_EVENT_t* pEv = new SACC_BILLING_INFO_EVENT_t(ev);
	//	smsc_log_debug(logger,"EventSender::put SACC_BILLING_INFO_EVENT_t addr=0x%X",pEv);
	if(eventsQueue.Push(pEv,QueueLength)==-1)
	{
		smsc_log_error(logger,"Error push to QOEUE for SACC EVENT? queue is Overflow!"); 
		delete pEv;
	}
}
void EventSender::Put(const SACC_ALARM_MESSAGE_t & ev)
{
	SACC_ALARM_MESSAGE_t* pEv = new SACC_ALARM_MESSAGE_t(ev);
	//	smsc_log_debug(logger,"EventSender::put SACC_ALARM_MESSAGE_t addr=0x%X",pEv);
	if(eventsQueue.Push(pEv,QueueLength)==-1)
	{
		smsc_log_error(logger,"Error push to QOEUE for SACC EVENT? queue is Overflow!"); 
		delete pEv;
	}
}
void EventSender::Put(const  SACC_SESSION_EXPIRATION_TIME_ALARM_t& ev)
{
	SACC_SESSION_EXPIRATION_TIME_ALARM_t* pEv = new SACC_SESSION_EXPIRATION_TIME_ALARM_t(ev);
	//	smsc_log_debug(logger,"EventSender::put SACC_SESSION_EXPIRATION_TIME_ALARM_t addr=0x%X",pEv);
	if(eventsQueue.Push(pEv,QueueLength)==-1)
	{
		smsc_log_error(logger,"Error push to QOEUE for SACC EVENT? queue is Overflow!"); 
		delete pEv;
	}
}
void EventSender::Put(const   SACC_OPERATOR_NOT_FOUND_ALARM_t& ev)
{
	SACC_OPERATOR_NOT_FOUND_ALARM_t* pEv = new SACC_OPERATOR_NOT_FOUND_ALARM_t(ev);
	//	smsc_log_debug(logger,"EventSender::put SACC_OPERATOR_NOT_FOUND_ALARM_t addr=0x%X",pEv);
	if(eventsQueue.Push(pEv,QueueLength)==-1)
	{
		smsc_log_error(logger,"Error push to QOEUE for SACC EVENT? queue is Overflow!"); 
		delete pEv;
	}
}

 
void EventSender::performTransportEvent(const SACC_TRAFFIC_INFO_EVENT_t& e)
{

	SaccPDU pdu;
	pdu.insertPDUString8((uint8_t*)e.Header.pAbonentNumber,MAX_ABONENT_NUMBER_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.lDateTime,sizeof(uint64_t));
	pdu.insertSegment((uint8_t*)&e.iOperatorId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceProviderId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceId,sizeof(uint32_t));
	pdu.insertPDUString8((uint8_t*)e.pSessionKey,MAX_SESSION_KEY_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.cProtocolId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.cCommandId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.sCommandStatus,sizeof(uint16_t));

	uint32_t sz =0;
	while(e.pMessageText[sz]!=0)
	{
		sz++;
	}

	pdu.insertSegment((uint8_t*)e.pMessageText,sz*sizeof(uint16_t));
	pdu.insertSegment((uint8_t*)&e.cDirection,sizeof(uint8_t));
	pdu.insertPDUHeader(e.Header.sEventType);

	uint8_t * buffer = new uint8_t[pdu.getallsize()];
	pdu.getAll(buffer) ;
	
	int b =SaccSocket.WriteAll((char*)buffer,pdu.getallsize());
		
	if(b<=0)
		bConnected=false;
	pdu.free();
	delete buffer;


}
void EventSender::performBillingEvent(const SACC_BILLING_INFO_EVENT_t& e)
{
	
	SaccPDU pdu;
	pdu.insertPDUString8((uint8_t*)e.Header.pAbonentNumber,MAX_ABONENT_NUMBER_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.lDateTime,sizeof(uint64_t));
	pdu.insertSegment((uint8_t*)&e.iOperatorId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceProviderId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceId,sizeof(uint32_t));
	pdu.insertPDUString8((uint8_t*)e.pSessionKey,MAX_SESSION_KEY_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.cProtocolId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.cCommandId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.sCommandStatus,sizeof(uint16_t));

	pdu.insertSegment((uint8_t*)&e.iMediaResourceType,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.iPriceCatId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.fBillingSumm,sizeof(float));
	pdu.insertPDUString8((uint8_t*)e.pBillingCurrency ,MAX_BILLING_CURRENCY_LENGTH);

	pdu.insertPDUHeader(e.Header.sEventType);

	uint8_t * buffer = new uint8_t[pdu.getallsize()];
	pdu.getAll(buffer) ;
	
	int b =SaccSocket.WriteAll((char*)buffer,pdu.getallsize());
		
	if(b<=0)
		bConnected=false;
	pdu.free();
	delete buffer;	

}
void EventSender::performAlarmMessageEvent(const SACC_ALARM_MESSAGE_t& e)
{
	SaccPDU pdu;
	pdu.insertPDUString8((uint8_t*)e.pAbonentsNumbers,MAX_NUMBERS_TEXT_LENGTH);
	pdu.insertPDUString8((uint8_t*)e.pAddressEmail ,MAX_EMAIL_ADDRESS_LENGTH);
	
	uint32_t sz =0;
	while(e.pMessageText[sz]!=0)
	{
		sz++;
	}
	pdu.insertSegment((uint8_t*)e.pMessageText,sz*sizeof(uint16_t));
	pdu.insertPDUHeader(e.sEventType);

	uint8_t * buffer = new uint8_t[pdu.getallsize()];
	pdu.getAll(buffer) ;
	
	int b =SaccSocket.WriteAll((char*)buffer,pdu.getallsize());
		
	if(b<=0)
		bConnected=false;
	pdu.free();
	delete buffer;	
}

void makeAlarmEvent(uint32_t evtype,void * ev)
{

}

void EventSender::performAlarmEvent(const SACC_ALARM_t& e)
{
	SaccPDU pdu;

	pdu.insertPDUString8((uint8_t*)e.Header.pAbonentNumber,MAX_ABONENT_NUMBER_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.lDateTime,sizeof(uint64_t));
	pdu.insertSegment((uint8_t*)&e.iOperatorId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceProviderId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceId,sizeof(uint32_t));
	pdu.insertPDUString8((uint8_t*)e.pSessionKey,MAX_SESSION_KEY_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.cProtocolId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.cCommandId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.sCommandStatus,sizeof(uint16_t));

	uint32_t sz =0;
	while(e.pMessageText[sz]!=0)
	{
		sz++;
	}

	pdu.insertSegment((uint8_t*)e.pMessageText,sz*sizeof(uint16_t));
	pdu.insertSegment((uint8_t*)&e.cDirection,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.iAlarmEventId,sizeof(uint32_t));

	pdu.insertPDUHeader(e.Header.sEventType);

	uint8_t * buffer = new uint8_t[pdu.getallsize()];
	pdu.getAll(buffer) ;
	
	int b =SaccSocket.WriteAll((char*)buffer,pdu.getallsize());
		
	if(b<=0)
		bConnected=false;
	pdu.free();
	delete buffer;
}
void EventSender::performSessionExpiredEvent(const SACC_SESSION_EXPIRATION_TIME_ALARM_t& e)
{
	uint32_t et=3;
	SaccPDU pdu;

	pdu.insertPDUString8((uint8_t*)e.Header.pAbonentNumber,MAX_ABONENT_NUMBER_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.lDateTime,sizeof(uint64_t));
	pdu.insertSegment((uint8_t*)&e.iOperatorId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceProviderId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceId,sizeof(uint32_t));
	pdu.insertPDUString8((uint8_t*)e.pSessionKey,MAX_SESSION_KEY_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.cProtocolId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.cCommandId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.sCommandStatus,sizeof(uint16_t));

	uint32_t sz =0;
	while(e.pMessageText[sz]!=0)
	{
		sz++;
	}

	pdu.insertSegment((uint8_t*)e.pMessageText,sz*sizeof(uint16_t));
	pdu.insertSegment((uint8_t*)&e.cDirection,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&et,sizeof(uint32_t));

	pdu.insertPDUHeader(0x0003);

	uint8_t * buffer = new uint8_t[pdu.getallsize()];
	pdu.getAll(buffer) ;
	
	int b =SaccSocket.WriteAll((char*)buffer,pdu.getallsize());
		
	if(b<=0)
		bConnected=false;
	pdu.free();
	delete buffer;
}
void EventSender::performOperatorNotFoundEvent(const SACC_OPERATOR_NOT_FOUND_ALARM_t& e)
{
	uint32_t et=1;
	SaccPDU pdu;

	pdu.insertPDUString8((uint8_t*)e.Header.pAbonentNumber,MAX_ABONENT_NUMBER_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.lDateTime,sizeof(uint64_t));
	pdu.insertSegment((uint8_t*)&e.iOperatorId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceProviderId,sizeof(uint32_t));
	pdu.insertSegment((uint8_t*)&e.Header.iServiceId,sizeof(uint32_t));
	pdu.insertPDUString8((uint8_t*)e.pSessionKey,MAX_SESSION_KEY_LENGTH);
	pdu.insertSegment((uint8_t*)&e.Header.cProtocolId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.cCommandId,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&e.Header.sCommandStatus,sizeof(uint16_t));

	uint32_t sz =0;
	while(e.pMessageText[sz]!=0)
	{
		sz++;
	}

	pdu.insertSegment((uint8_t*)e.pMessageText,sz*sizeof(uint16_t));
	pdu.insertSegment((uint8_t*)&e.cDirection,sizeof(uint8_t));
	pdu.insertSegment((uint8_t*)&et,sizeof(uint32_t));

	pdu.insertPDUHeader(0x0003);

	uint8_t * buffer = new uint8_t[pdu.getallsize()];
	pdu.getAll(buffer) ;
	
	int b =SaccSocket.WriteAll((char*)buffer,pdu.getallsize());
		
	if(b<=0)
		bConnected=false;
	pdu.free();
	delete buffer;	
}

}//sacc
}//stat
}//scag



