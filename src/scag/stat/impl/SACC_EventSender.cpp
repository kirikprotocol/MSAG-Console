 //#include "SACC_Defs.h"
//#include "SACC_SyncQueue.h"
#include "SACC_EventSender.h"
//#include "SACC_Events.h"
#include <logger/Logger.h>
#include <util/BufferSerialization.hpp>

using namespace smsc::util;

namespace scag2 {
namespace stat {

void SaccSerialBuffer::writeStr(std::string& s, uint16_t maxLen)
{
    uint16_t len = (s.length() > maxLen) ? maxLen : s.length() ;
    WriteNetInt16(len);
    Write(s.c_str(), len);
}
void SaccSerialBuffer::writeInt16(uint16_t i)
{
    WriteNetInt16(sizeof(uint16_t));
    WriteNetInt16(i);
}
void SaccSerialBuffer::writeInt32(uint32_t i)
{
    WriteNetInt16(sizeof(uint32_t));
    WriteNetInt32(i);
}
void SaccSerialBuffer::writeInt64(uint64_t i)
{
    WriteNetInt16(sizeof(uint64_t));
    WriteNetInt64(i);
}
void SaccSerialBuffer::writeByte(uint8_t i)
{
    WriteNetInt16(sizeof(uint8_t));
    WriteByte(i);
}
bool SaccSerialBuffer::writeToSocket(Socket& sock)
{
    uint32_t bsize = getPos();
    setPos(0);
    WriteNetInt32(bsize);
    setPos(0);
    return sock.WriteAll(getBuffer() ,bsize) > 0;
}

void SaccEventHeader::write(SaccSerialBuffer& buf)
{
    buf.writeStr(pAbonentNumber, MAX_ABONENT_NUMBER_LENGTH);
    buf.writeInt64(lDateTime);
    buf.writeInt32(iOperatorId);
    buf.writeInt32(iServiceProviderId);
    buf.writeInt32(iServiceId);

    buf.writeStr(pSessionKey, MAX_SESSION_KEY_LENGTH);    

    buf.writeByte(cProtocolId);
    buf.writeByte(cCommandId);
    buf.writeInt16(sCommandStatus);
}

void SaccTrafficInfoEvent::write(SaccSerialBuffer& buf)
{
    Header.write(buf);

    buf.writeStr(pMessageText, MAX_TEXT_MESSAGE_LENGTH * 2);
    buf.writeByte(cDirection);
    buf.writeStr(keywords, MAX_KEYWORDS_TEXT_LENGTH * 2);
}

void SaccBillingInfoEvent::write(SaccSerialBuffer& buf)
{
    Header.write(buf);

    buf.writeInt32(iMediaResourceType);
    buf.writeInt32(iPriceCatId);

    buf.WriteNetInt16(sizeof(float));
    buf.Write(&fBillingSumm, sizeof(float));

    buf.writeStr(pBillingCurrency, MAX_BILLING_CURRENCY_LENGTH);
}

void SaccAlarmMessageEvent::write(SaccSerialBuffer& buf)
{
    buf.writeStr(pAbonentsNumbers, MAX_NUMBERS_TEXT_LENGTH);
    buf.writeStr(pAddressEmail, MAX_EMAIL_ADDRESS_LENGTH);
    buf.writeStr(pMessageText, MAX_TEXT_MESSAGE_LENGTH * 2);
    buf.writeStr(pDeliveryTime, DELEVIRY_TIME_LENGTH);

    buf.writeByte(cCriticalityLevel);
 
    if(sUsr)
        buf.writeInt16(sUsr);
    else
        buf.WriteNetInt16(0); 

    if(cEsmClass)
        buf.writeByte(cEsmClass);
    else
        buf.WriteNetInt16(0); 

    if(sSrcPort)
       buf.writeInt16(sSrcPort);    
    else
       buf.WriteNetInt16(0); 
     
    if(sDestPort)
       buf.writeInt16(sDestPort);
    else
       buf.WriteNetInt16(0); 

    buf.writeStr(pPacketType, PACKET_TYPE_LENGTH);
}

void SaccAlarmEvent::write(SaccSerialBuffer& buf)
{
    Header.write(buf);

    buf.writeStr(pMessageText, MAX_TEXT_MESSAGE_LENGTH * 2);

    buf.writeByte(cDirection);
    buf.writeInt32(iAlarmEventId);
}
namespace sacc{

EventSender::EventSender()
{
    bStarted  = false;
    logger = 0;
    bConnected = false;
    Host="";
    Port=0;
    Timeout=100;
    QueueLength=10000;
    pdubuffer.resize(0xFFFF);
    queueIdx = 0;
}

EventSender::~EventSender()
{
    QueueType* queue = switchQueue();
    do {
        while ( queue->Count() > 0 )
        {
            SaccEvent* ev;                    
            if ( queue->Pop(ev) && ev)
                processEvent(ev);
        }
        queue = switchQueue();
    } while ( queue->Count() > 0 );
}

void EventSender::init(std::string& host,int port,int timeout,int queuelen,/*,bool * bf,*/smsc::logger::Logger * lg)
{
 if(!lg) throw Exception("EventSender::init logger is 0");

 QueueLength=queuelen;

 bStarted  = false;
 logger = lg;
 Host = host;
 Port= port;
 bConnected=false;
 Timeout=timeout*1000;
 lastOverflowNotify = 0;
 lastConnectTry = 0;
    smsc_log_debug(logger,"EventSender::init confuration succsess.");
}


bool EventSender::processEvent(SaccEvent *ev)
{
    pdubuffer.setPos(sizeof(uint32_t));
    pdubuffer.WriteNetInt16(ev->getEventType()); 

    ev->write(pdubuffer); 

    bConnected = pdubuffer.writeToSocket(SaccSocket);

    delete ev;

    return true;
}

int EventSender::Execute()
{
    while( bStarted )
    {
        // smsc_log_debug(logger,"sacc.evsend rolling");
        if ( !bConnected )
        {
            //          SaccSocket.Abort();
            time_t now = time(0);
            if ( lastConnectTry+30 < now ) {
                SaccSocket.Close();
                lastConnectTry = now;
                if(connect(Host,Port,Timeout))
                    bConnected = true;
            }
            if ( !bConnected ) {
                MutexGuard mg(mtx);
                mtx.wait(Timeout);
                continue;
            }
        }

        // we are connected here
        if (!bStarted) break;

        QueueType* queue = switchQueue();
        if ( ! queue->Count() ) {
            sendPing();
            MutexGuard mg(mtx);
            if ( !bStarted ) break;
            mtx.wait(Timeout);
            continue;
        }

        // we have a filled queue here
        if(!bStarted) break;      
          
        while ( queue->Count() > 0 )
        {
            if ( !bConnected ) continue;
            SaccEvent* ev;
            if ( queue->Pop(ev) && ev )
                processEvent(ev);
        }
    }

    do {
        QueueType* queue = switchQueue();
        if ( queue->Count() == 0 ) break;
        while ( queue->Count() > 0 )
        {
            SaccEvent* ev;                    
            if ( queue->Pop(ev) && ev)
            {
                if(bConnected)
                    processEvent(ev);
                else
                    delete ev;
            }
        }
    } while ( true );
    SaccSocket.Close();
    smsc_log_debug(logger,"EventSender stopped.");
    return 1;
}

bool EventSender::connect(std::string host, int port,int timeout)
{
 
 if(SaccSocket.Init(host.c_str(),port,timeout/1000)!=0)
 {
     smsc_log_warn(logger,"EventSender::connect Failed to init socket");
  return false;
 }

 if(SaccSocket.Connect()!=0)
 {
     smsc_log_warn(logger,"EventSender::connect Failed to connect");
  return false;
 }


 bConnected=true;
 smsc_log_debug(logger,"EventSender::connect succsess to %s:%d",host.c_str(),port);
 return true;
}

void EventSender::Start()
{
    bStarted=true;
    Thread::Start();
}

void EventSender::Stop()
{
    bStarted = false;
    mtx.notifyAll();
}
 
void EventSender::sendPing()
{
    pdubuffer.setPos(sizeof(uint32_t));
    pdubuffer.WriteNetInt16(0); 

    bConnected = pdubuffer.writeToSocket(SaccSocket);    
        
    smsc_log_debug(logger,"EventSender::ping sent");        
}

void EventSender::PushEvent(SaccEvent* item)
{
    MutexGuard g(mtx);
    smsc_log_debug(logger,"EventSender::put %s addr=0x%X", item->getName(), item);

    if (eventsQueue[queueIdx].Count() < QueueLength )
    {
        eventsQueue[queueIdx].Push(item);
        mtx.notifyAll();
        return;
    }
    mtx.notifyAll();  
    if(lastOverflowNotify < time(NULL) - 30)
    {
        smsc_log_warn(logger,"Error push %s to QOEUE for SACC EVENT queue is Overflow!", item->getName()); 
        lastOverflowNotify = time(NULL);
    }
    delete item;
}


EventSender::QueueType* EventSender::switchQueue()
{
    MutexGuard mg(mtx);
    unsigned freeidx = 1 - queueIdx;
    if ( eventsQueue[freeidx].Count() == 0 ) {
        queueIdx = freeidx;
        freeidx = 1 - queueIdx;
    }
    return &(eventsQueue[freeidx]);
}

}}}
