#include "system/smppio/SmppIOTask.hpp"
#include "core/network/Multiplexer.hpp"
#include "system/smppio/SmppProxy.hpp"
#include "util/debug.h"

namespace smsc{
namespace system{
namespace smppio{

using core::network::Multiplexer;

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
      sockets[i]->getSocket()->setData(0,&sockets[i]);
      m.add(sockets[i]->getSocket());
    }
    mon.Unlock();
    if(m.canRead(ready)>0)
    {
      for(i=0;i<ready.Count();i++)
      {
        SmppSocket *ss=(SmppSocket*)(ready[i]->getData(0));
        int retcode=ss->receive();
        if(retcode==-1)
        {
          removeSocket(ss->getSocket());
          outTask->removeSocket(ss->getSocket());
        }else
        if(retcode==1)
        {
          smsc::smpp::SmppHeader *pdu=ss->decode();
          switch(pdu->commandId)
          {
            case smsc::smpp::SmppCommandSet::BIND_RECIEVER:
            case smsc::smpp::SmppCommandSet::BIND_TRANSMITTER:
            case smsc::smpp::SmppCommandSet::BIND_TRANCIEVER:
            {
              smsc::smpp::PduBindTRX* bindpdu=reinterpret_cast<smsc::smpp::PduBindTRX*>(pdu);
              SmppProxy *proxy=new SmppProxy(ss);
              try{
                trace("try to register sme");
                smeManager->registerSmeProxy(bindpdu->get_systemId(),proxy);
              }catch(...)
              {

                trace("registration failed");
                delete proxy;
              }
            }break;
            default:
            {
              ss->getProxy()->putIncomingCommand(pdu);
              delete pdu;
            }break;
          }
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
    for(i=0;i<sockets.Count();i++)
    {

    }
  }
}

};//smppio
};//system
};//smsc
