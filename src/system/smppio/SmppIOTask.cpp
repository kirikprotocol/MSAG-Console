#include "system/smppio/SmppIOTask.hpp"
#include "core/network/Multiplexer.hpp"
#include "system/smppio/SmppProxy.hpp"
#include "util/debug.h"
#include <memory>

namespace smsc{
namespace system{
namespace smppio{

using core::network::Multiplexer;
using std::auto_ptr;
using namespace smsc::smpp;

void SmppIOTask::removeSocket(Socket *sock)
{
  mon.Lock();
  for(int i=0;i<sockets.Count();i++)
  {
    if(sockets[i]->getSocket()==sock)
    {
      delete sockets[i];
      sockets.Delete(i);
    }
  }
  mon.Unlock();
}


void SmppInputThread::addSocket(Socket* sock)
{
  mon.Lock();
  SmppSocket *s=new SmppSocket(ssModeRead,sock);
  s->assignTasks(this,outTask);
  sockets.Push(s);
  mon.Unlock();
}

int SmppInputThread::Execute()
{
  Multiplexer m;
  Multiplexer::SockArray ready;
  int i;
  for(;;)
  {
    mon.Lock();
    m.clear();
    if(sockets.Count()==0)
    {
      mon.wait();
      if(sockets.Count()==0)break;
    }
    for(i=0;i<sockets.Count();i++)
    {
      sockets[i]->getSocket()->setData(0,sockets[i]);
      trace2("data0:%p, socket0:%p\n",sockets[i]->getSocket()->getData(0),sockets[i]);
      m.add(sockets[i]->getSocket());
    }
    mon.Unlock();
    if(m.canRead(ready)>0)
    {
      MutexGuard g(mon);
      for(i=0;i<ready.Count();i++)
      {
        SmppSocket *ss=(SmppSocket*)(ready[i]->getData(0));
        trace2("getdata:%p\n",ss);
        int retcode=ss->receive();
        if(retcode==-1)
        {
          removeSocket(ss->getSocket());
          outTask->removeSocket(ss->getSocket());
        }else
        if(retcode==1)
        {
          SmppHeader *pdu=ss->decode();
          switch(pdu->get_commandId())
          {
            case SmppCommandSet::BIND_RECIEVER:
            case SmppCommandSet::BIND_TRANSMITTER:
            case SmppCommandSet::BIND_TRANCIEVER:
            {
              PduBindTRX *bindpdu=reinterpret_cast<smsc::smpp::PduBindTRX*>(pdu);
              SmppProxy *proxy=new SmppProxy(ss);
              PduBindTRXResp resppdu;
              resppdu.get_header().set_commandId(pdu->get_commandId()|0x80000000);
              resppdu.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
              resppdu.set_systemId("smsc");
              bool err=false;

              try{
                trace("try to register sme");
                smeManager->registerSmeProxy(bindpdu->get_systemId(),proxy);
                resppdu.get_header().set_commandStatus(SmppStatusSet::ESME_ROK);
              }catch(...)
              {
                resppdu.get_header().set_commandStatus(SmppStatusSet::ESME_RSYSERR);
                trace("registration failed");
                delete proxy;
                err=true;
              }

              char buf[64];
              int size=resppdu.size();
              __require__(size<sizeof(buf));
              SmppStream s;
              assignStreamWith(&s,buf,sizeof(buf),false);
              fillSmppPdu(&s,reinterpret_cast<SmppHeader*>(&resppdu));
              int wroff=0,wr;
              while(wroff<size)
              {
                wr=ss->getSocket()->Write(buf+wroff,size-wroff);
                if(wr<=0)break;
                wroff+=wr;
                trace2("sending:%d/%d\n",wroff,size);
              }
              if(err)
              {
                removeSocket(ss->getSocket());
                outTask->removeSocket(ss->getSocket());
                delete ss;
              }
            }break;
            default:
            {
              ss->getProxy()->putIncomingCommand(pdu);
            }break;
          }
          delete pdu;
        }
      }
    }
  }
}

void SmppOutputThread::addSocket(Socket* sock)
{
  mon.Lock();
  SmppSocket *s=new SmppSocket(ssModeWrite,sock);
  s->assignTasks(inTask,this);
  sockets.Push(s);
  mon.Unlock();
}


int SmppOutputThread::Execute()
{
  Multiplexer m;
  Multiplexer::SockArray ready;
  int i;
  for(;;)
  {
    mon.Lock();
    if(sockets.Count()==0)
    {
      mon.wait();
      if(sockets.Count()==0)break;
    }
    int cnt=0;
    m.clear();
    trace("check data for output");
    for(i=0;i<sockets.Count();i++)
    {
      SmppSocket *s=sockets[i];
      if(!s->hasData() && s->getProxy() && s->getProxy()->hasOutput())
      {
        SmppHeader *pdu=s->getProxy()->getCommand().makePdu();
        int size=calcSmppPacketLength(pdu);
        char* buf=s->getBuffer(size);
        SmppStream st;
        assignStreamWith(&st,buf,size,false);
        fillSmppPdu(&st,pdu);
        s->send(size);
      }
      if(s->hasData())
      {
        s->getSocket()->setData(1,s);
        m.add(s->getSocket());
        cnt++;
      }
    }
    trace2("found %d sockets wanting to send data",cnt);
    if(!cnt)
    {
      trace("wait for data");
      //mon.Unlock();
      mon.wait();
      mon.Unlock();
      continue;
    }
    mon.Unlock();
    if(m.canWrite(ready)>0)
    {
      MutexGuard g(mon);
      for(int i=0;i<ready.Count();i++)
      {
        SmppSocket *s=(SmppSocket *)ready[i]->getData(1);
        s->send();
      }
    }
  }
}

};//smppio
};//system
};//smsc
