//#include "SACC_Defs.h"
//#include "SACC_SyncQueue.h"
#include "SACC_EventSender.h"
//#include "SACC_Events.h"
#include "stat/Statistics.h"
#include <logger/Logger.h>
#include <util/BufferSerialization.hpp>


using namespace scag::stat;
using namespace scag::stat::Counters;
using namespace smsc::util;

namespace scag{
namespace stat{
namespace sacc{
 


EventSender::EventSender()
{

 bStarted  = false;
 logger = 0;
 bConnected=false;
 Host="";
 Port=0;
 Timeout=100;
 QueueLength=10000;
 pdubuffer.resize(0xFFFF);
}



EventSender::~EventSender()
{

	//smsc_log_debug(logger,"EventSender:: ~EventSender %d",*bStarted);
	//evReconnect.notifyAll();
    //mtx.notifyAll();	
}



void EventSender::init(std::string& host,int port,int timeout,int queuelen,/*,bool * bf,*/smsc::logger::Logger * lg)
{

 if(!lg)
  throw Exception("EventSender::init logger is 0");

// if(!bf)
 // throw Exception("EventSender::init start-stop flag is 0");

 QueueLength=queuelen;

 bStarted  = false;//bf;
 logger = lg;
 Host = host;
 Port= port;
 bConnected=false;
 Timeout=timeout*1000;
 smsc_log_debug(logger,"EventSender::init confuration succsess.");

}



bool EventSender::processEvent(void *ev)
{

 uint16_t evType=0;
 memcpy(&evType,ev,sizeof(uint16_t));  
 
 switch(evType) 
 {
	 case sec_transport:
	  {
		   SACC_TRAFFIC_INFO_EVENT_t e;
		   memcpy(&e,ev,sizeof(SACC_TRAFFIC_INFO_EVENT_t));
		   delete (SACC_TRAFFIC_INFO_EVENT_t*)ev;
		   performTransportEvent(e);
		   smsc_log_debug(logger,"EventSender::Execute Sacc stat TRAFFIC event  processed from queue addr=0x%X",evType,ev);
	  }
	   break;

	 case sec_bill:
	  {
		   SACC_BILLING_INFO_EVENT_t e;
		   memcpy(&e,ev,sizeof(SACC_BILLING_INFO_EVENT_t));
		   delete (SACC_BILLING_INFO_EVENT_t*)ev;
		   performBillingEvent(e);
		   smsc_log_debug(logger,"EventSender::Execute Sacc stat BILLING event  processed from queue addr=0x%X",evType,ev);
	  }
	   break;

	 case sec_alarm_message:
	  {
		   SACC_ALARM_MESSAGE_t e;
		   memcpy(&e,ev,sizeof(SACC_ALARM_MESSAGE_t));
		   delete (SACC_ALARM_MESSAGE_t*)ev;
		   performAlarmMessageEvent(e);
		   smsc_log_debug(logger,"EventSender::Execute Sacc stat ALARM_MESSAGE  processed from queue addr=0x%X",evType,ev);
	  }
	  break;

	 case sec_alarm:
	  {
		   SACC_ALARM_t e;
		   memcpy(&e,ev,sizeof(SACC_ALARM_t));
		   delete (SACC_ALARM_t*)ev;
		   performAlarmEvent(e);
		   smsc_log_debug(logger,"EventSender::Execute Sacc stat ALARM event  processed from queue addr=0x%X",evType,ev);
	  }

	  default:
		  smsc_log_error(logger,"EventSender Unknown event!!");
	   break;

	  }
 return true;
}

bool EventSender::checkQueue()
{
 
  MutexGuard g(mtx);
  
  mtx.wait(Timeout);

  void * ev;
  if(bConnected)
  {
	if(eventsQueue.Pop(ev))
	{
	   if(ev)
	   { 
		  processEvent(ev);
	   }
	   
	  return true;
	}
  }
  return false;
}

int EventSender::Execute()
{
 if(connect(Host,Port,Timeout))
 {
	   bConnected=true;
 }
 
 while( bStarted)
 {

	  if(!bConnected )
	  {
		SaccSocket.Abort();

			if(connect(Host,Port,Timeout))
			{
			   bConnected=true;
			}
		//evReconnect.wait(Timeout);
	  }
	  checkQueue();
 }

 smsc_log_debug(logger,"EventSender stopped.");
 return 1;
}



bool EventSender::connect(std::string host, int port,int timeout)
{
 
 if(SaccSocket.Init(host.c_str(),port,timeout/1000)!=0)
 {
     smsc_log_error(logger,"EventSender::connect Failed to init socket");
  return false;
 }

 if(SaccSocket.Connect()!=0)
 {
     smsc_log_error(logger,"EventSender::connect Failed to connect");
  return false;
 }


 bConnected=true;
     smsc_log_debug(logger,"EventSender::connect succsess to %s:%d",host.c_str(),port);
 return true;
}


bool EventSender::isActive()
{
   return bConnected;
}





void EventSender::Start()
{

 
 bStarted=true;
 Thread::Start();

}

 

void EventSender::Stop()
{

 //evReconnect.notifyAll();
 
 bStarted =false;
 mtx.notifyAll();

}

void EventSender::Put (const SACC_ALARM_t& ev)
{

 SACC_ALARM_t* pEv = new SACC_ALARM_t(ev);
 smsc_log_debug(logger,"EventSender::put SACC_TRAFFIC_INFO_EVENT_t addr=0x%X",pEv);
 
 if(!PushEvent(pEv))
 {
    smsc_log_error(logger,"Error push alarm_event to QOEUE for SACC EVENT queue is Overflow!"); 
  delete pEv;
 }
}



void EventSender::Put(const SACC_TRAFFIC_INFO_EVENT_t& ev)
{
 SACC_TRAFFIC_INFO_EVENT_t* pEv = new SACC_TRAFFIC_INFO_EVENT_t(ev);
 smsc_log_debug(logger,"EventSender::put SACC_TRAFFIC_INFO_EVENT_t addr=0x%X",pEv);

 if(!PushEvent(pEv))
 {
     smsc_log_error(logger,"Error push traffic_event to QOEUE for SACC EVENT queue is Overflow!"); 
  delete pEv;
 }
}

void EventSender::Put(const SACC_BILLING_INFO_EVENT_t& ev)
{
 SACC_BILLING_INFO_EVENT_t* pEv = new SACC_BILLING_INFO_EVENT_t(ev);
 smsc_log_debug(logger,"EventSender::put SACC_BILLING_INFO_EVENT_t addr=0x%X",pEv);

 if(!PushEvent(pEv))
 {
  smsc_log_error(logger,"Error push billing_event to QOEUE for SACC EVENT? queue is Overflow!"); 
  delete pEv;
 }
}

void EventSender::Put(const SACC_ALARM_MESSAGE_t & ev)
{
 SACC_ALARM_MESSAGE_t* pEv = new SACC_ALARM_MESSAGE_t(ev);
 smsc_log_debug(logger,"EventSender::put SACC_ALARM_MESSAGE_t addr=0x%X",pEv);

 if(!PushEvent(pEv))
 {
    smsc_log_error(logger,"Error push Alarm_Message to QOEUE for SACC EVENT queue is Overflow!"); 
  delete pEv;
 }
}
 
void EventSender::performTransportEvent(const SACC_TRAFFIC_INFO_EVENT_t& e)
{

 pdubuffer.setPos(sizeof(uint32_t));///header plase 
 pdubuffer.WriteNetInt16(e.getEventType());         
 
 std::string stran;
 stran.assign((char*)e.Header.pAbonentNumber);
 
 pdubuffer.WriteNetInt16((uint16_t)stran.length());
 pdubuffer.Write(e.Header.pAbonentNumber,stran.length());
 pdubuffer.WriteNetInt16(sizeof(uint64_t));
 pdubuffer.WriteNetInt64(e.Header.lDateTime);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.iOperatorId);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.Header.iServiceProviderId);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.Header.iServiceId);
 std::string strsk;
 strsk.assign((char*)e.pSessionKey);
 
 pdubuffer.WriteNetInt16((uint16_t)strsk.length());
 pdubuffer.Write(e.pSessionKey,strsk.length());
 pdubuffer.WriteNetInt16(1);
 pdubuffer.WriteByte(e.Header.cProtocolId);
 pdubuffer.WriteNetInt16(1);
 pdubuffer.WriteByte(e.Header.cCommandId);
 pdubuffer.WriteNetInt16(2);
 pdubuffer.WriteNetInt16(e.Header.sCommandStatus);

 uint32_t sz =0;
 while(e.pMessageText[sz]!=0 && sz < MAX_TEXT_MESSAGE_LENGTH )
 {
   sz++;
 }
 pdubuffer.WriteNetInt16(sz*sizeof(uint16_t));
 pdubuffer.Write(e.pMessageText,sz*sizeof(uint16_t));
 pdubuffer.WriteNetInt16(1);
 pdubuffer.WriteByte(e.cDirection);
 uint32_t bsize= pdubuffer.getPos();
 pdubuffer.setPos(0);
 pdubuffer.WriteNetInt32(bsize);
 pdubuffer.setPos(0);
 int b =SaccSocket.WriteAll(pdubuffer.getBuffer() ,bsize);
 if(b<=0)
  bConnected=false;

}

void EventSender::performBillingEvent(const SACC_BILLING_INFO_EVENT_t& e)
{
	
 pdubuffer.setPos(sizeof(uint32_t));///header plase 
 pdubuffer.WriteNetInt16(e.getEventType()); 
 
 std::string stran;
 stran.assign((char*)e.Header.pAbonentNumber);
 pdubuffer.WriteNetInt16((uint16_t)stran.length());
 pdubuffer.Write(e.Header.pAbonentNumber,stran.length());
 pdubuffer.WriteNetInt16(sizeof(uint64_t));
 pdubuffer.WriteNetInt64(e.Header.lDateTime);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.iOperatorId);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.Header.iServiceProviderId);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.Header.iServiceId);

 std::string strsk;
 strsk.assign((char*)e.pSessionKey);
 pdubuffer.WriteNetInt16((uint16_t)strsk.length());
 pdubuffer.Write(e.pSessionKey,strsk.length());
 pdubuffer.WriteNetInt16(1);
 pdubuffer.WriteByte(e.Header.cProtocolId);
 pdubuffer.WriteNetInt16(1);
 pdubuffer.WriteByte(e.Header.cCommandId);
 pdubuffer.WriteNetInt16(2);
 pdubuffer.WriteNetInt16(e.Header.sCommandStatus);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.iMediaResourceType);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.iPriceCatId);
 pdubuffer.WriteNetInt16(sizeof(float));
 pdubuffer.Write(&e.fBillingSumm,sizeof(float));

 std::string strbcur;
 strbcur.assign((char*)e.pBillingCurrency);
 pdubuffer.WriteNetInt16((uint16_t)strbcur.length());
 pdubuffer.Write(e.pBillingCurrency,strbcur.length());

 uint32_t bsize= pdubuffer.getPos();
 pdubuffer.setPos(0);
 pdubuffer.WriteNetInt32(bsize);
 pdubuffer.setPos(0);
 int b =SaccSocket.WriteAll(pdubuffer.getBuffer() ,bsize);

 if(b<=0)
  bConnected=false;
 
}

void EventSender::performAlarmMessageEvent(const SACC_ALARM_MESSAGE_t& e)
{
 
 pdubuffer.setPos(sizeof(uint32_t));///header plase 
 pdubuffer.WriteNetInt16(e.getEventType()); 
 
 std::string stran;
 stran.assign((char*)e.pAbonentsNumbers);
 pdubuffer.WriteNetInt16((uint16_t)stran.length());
 pdubuffer.Write(e.pAbonentsNumbers,stran.length());

 std::string stremail;
 stremail.assign((char*)e.pAddressEmail);
 pdubuffer.WriteNetInt16((uint16_t)stremail.length());
 pdubuffer.Write(e.pAddressEmail,stremail.length());
 
 uint32_t sz =0;
 while(e.pMessageText[sz]!=0 && sz < MAX_TEXT_MESSAGE_LENGTH )
 {
   sz++;
 }
 pdubuffer.WriteNetInt16(sz*sizeof(uint16_t));
 pdubuffer.Write(e.pMessageText,sz*sizeof(uint16_t));
 uint32_t bsize= pdubuffer.getPos();
 pdubuffer.setPos(0);
 pdubuffer.WriteNetInt32(bsize);
 pdubuffer.setPos(0);
 int b =SaccSocket.WriteAll(pdubuffer.getBuffer() ,bsize);

 if(b<=0)
  bConnected=false;
}


void EventSender::performAlarmEvent(const SACC_ALARM_t& e)
{
 pdubuffer.setPos(sizeof(uint32_t));///header plase 
 pdubuffer.WriteNetInt16(e.getEventType()); 
 std::string stran;
 stran.assign((char*)e.Header.pAbonentNumber);
 pdubuffer.WriteNetInt16((uint16_t)stran.length());//an
 pdubuffer.Write(e.Header.pAbonentNumber,stran.length());//an
 pdubuffer.WriteNetInt16(sizeof(uint64_t));
 pdubuffer.WriteNetInt64(e.Header.lDateTime);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.iOperatorId);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.Header.iServiceProviderId);
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.Header.iServiceId);
 std::string strsk;
 strsk.assign((char*)e.pSessionKey);
 pdubuffer.WriteNetInt16((uint16_t)strsk.length());
 pdubuffer.Write(e.pSessionKey,strsk.length());

 pdubuffer.WriteNetInt16(1);
 pdubuffer.WriteByte(e.Header.cProtocolId);
 pdubuffer.WriteNetInt16(1);
 pdubuffer.WriteByte(e.Header.cCommandId);
 pdubuffer.WriteNetInt16(2);
 pdubuffer.WriteNetInt16(e.Header.sCommandStatus);
 
  uint32_t sz =0;
 while(e.pMessageText[sz]!=0 && sz < MAX_TEXT_MESSAGE_LENGTH )
 {
   sz++;
 }
 pdubuffer.WriteNetInt16(sz*sizeof(uint16_t));
 pdubuffer.Write(e.pMessageText,sz*sizeof(uint16_t));
 pdubuffer.WriteNetInt16(1);//!!!!!
 pdubuffer.WriteByte(e.cDirection);
 
 pdubuffer.WriteNetInt16(sizeof(uint32_t));
 pdubuffer.WriteNetInt32(e.iAlarmEventId);

 uint32_t bsize= pdubuffer.getPos();
 pdubuffer.setPos(0);
 pdubuffer.WriteNetInt32(bsize);
 pdubuffer.setPos(0);
 int b =SaccSocket.WriteAll(pdubuffer.getBuffer() ,bsize);

 if(b<=0)
  bConnected=false;

}

bool EventSender::PushEvent(void* item)
{

  MutexGuard g(mtx);

  if(eventsQueue.Count()<QueueLength)
  {
	  eventsQueue.Push(item);
	  mtx.notifyAll();
	  return true;
  }
  mtx.notifyAll();  
  return false;
}




}//sacc

}//stat

}//scag

















































