#include "SACC_EventSender.h"
#include <logger/Logger.h>
#include "util/BufferSerialization.hpp"

using namespace smsc::util;

namespace scag2 {
namespace stat {

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
    //pdubuffer.resize(0xFFFF);
    queueIdx = 0;
    enabled_ = false;
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

void EventSender::init(std::string& host,int port,int timeout,int queuelen,/*,bool * bf,*/smsc::logger::Logger * lg, bool enabled)
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
  enabled_ = enabled;
  smsc_log_debug(logger,"EventSender::init confuration succsess.");
  if (!enabled_) {
    smsc_log_info(logger,"EventSender is disabled.");
  }
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
                if (!bStarted) break;
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
  if (!enabled_) {
    return;
  }
    bStarted=true;
    Thread::Start();
}

void EventSender::Stop()
{
  if (!enabled_) {
    return;
  }
    MutexGuard mg(mtx);
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
  if (!enabled_) {
    if (item) {
      smsc_log_warn(logger,"EventSender is disabled, delete event: %s addr=0x%X", item->getName(), item);
      delete item;
    }
    return;
  }
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
