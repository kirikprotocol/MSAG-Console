#ifndef DISABLE_TRACING
//#define DISABLE_TRACING
#endif

#include "system/smppio/SmppIOTask.hpp"
#include "system/smppio/SmppProxy.hpp"
#include "util/debug.h"
#include "system/smppio/SmppSocketsManager.hpp"
#include <exception>
//#include <memory>

namespace smsc{
namespace system{
namespace smppio{

//using std::auto_ptr;
using namespace smsc::smpp;
using namespace std;

const int SOCKET_SLOT_REFCOUNT=0;
const int SOCKET_SLOT_SOCKETSMANAGER=1;
const int SOCKET_SLOT_INPUTSMPPSOCKET=2;
const int SOCKET_SLOT_OUTPUTSMPPSOCKET=3;
const int SOCKET_SLOT_INPUTMULTI=4;
const int SOCKET_SLOT_OUTPUTMULTI=5;
const int SOCKET_SLOT_KILL=6;

SmppInputThread::~SmppInputThread()
{
  while(sockets.Count())
  {
    removeSocket(sockets[0]->getSocket());
  }
}

void SmppInputThread::addSocket(Socket* sock)
{
  MutexGuard g(mon);
  SmppSocket *s=new SmppSocket(ssModeRead,sock);
  s->assignTasks(this,outTask);
  sock->setData(SOCKET_SLOT_INPUTSMPPSOCKET,0);
  sock->setData(SOCKET_SLOT_INPUTMULTI,0);
  sock->setData(SOCKET_SLOT_KILL,0);
  sockets.Push(s);
}


void SmppInputThread::removeSocket(Socket *sock)
{
  mon.Lock();
  for(int i=0;i<sockets.Count();i++)
  {
    if(sockets[i]->getSocket()==sock)
    {
      if(sock->getData(SOCKET_SLOT_INPUTMULTI))
      {
        sock->setData(SOCKET_SLOT_KILL,(void*)1);
      }
      else
      {
        mon.Unlock();
        killSocket(i);
        mon.Lock();
      }
      break;
    }
  }
  mon.Unlock();
}

void SmppInputThread::killSocket(int idx)
{
  SmppSocket *ss=sockets[idx];
  Socket *s=ss->getSocket();
  mul.remove(s);
  delete ss;
  sockets.Delete(idx);
  SmppSocketsManager *m=
    (SmppSocketsManager*)s->getData(SOCKET_SLOT_SOCKETSMANAGER);
  trace2("removing socket %p by input thread",s);
  m->removeSocket(s);
  if(ss->getProxy())
  {
    try{
      smeManager->unregisterSmeProxy(ss->getProxy()->getId());
    }catch(...)
    {
    }
    delete ss->getProxy();
  }
}


int SmppInputThread::Execute()
{
  Multiplexer::SockArray ready;
  Multiplexer::SockArray error;
  int i;
  while(!isStopping)
  {
    {
      MutexGuard g(mon);
      while(sockets.Count()==0)
      {
        trace("in:wait for sockets");
        //mon.wait();
        mon.wait();
        if(isStopping)break;
        trace("in:got new socket");
      }
      if(isStopping)break;
      for(i=0;i<sockets.Count();i++)
      {
        SmppSocket *ss=sockets[i];
        Socket *s=ss->getSocket();
        if(!s->getData(SOCKET_SLOT_INPUTSMPPSOCKET))
        {
          s->setData(SOCKET_SLOT_INPUTSMPPSOCKET,(void*)ss);
          mul.add(s);
        }
        if(s->getData(SOCKET_SLOT_KILL))
        {
          killSocket(i);
          i--;
          continue;
        }
        s->setData(SOCKET_SLOT_INPUTMULTI,(void*)1);
      }
    }
    //trace("in: canRead\n");
    if(mul.canRead(ready,error,1000)>0)
    {
      MutexGuard g(mon);
      for(i=0;i<sockets.Count();i++)
      {
        Socket *s=sockets[i]->getSocket();
        if(!s->getData(SOCKET_SLOT_INPUTSMPPSOCKET))
        {
          s->setData(SOCKET_SLOT_INPUTSMPPSOCKET,sockets[i]);
          mul.add(s);
        }
        s->setData(SOCKET_SLOT_INPUTMULTI,0);
        if(s->getData(SOCKET_SLOT_KILL))
        {
          int j;
          for(j=0;i<error.Count();j++)
          {
            if(error[j]==s)
            {
              error.Delete(j);
            }
          }
          for(j=0;j<ready.Count();j++)
          {
            if(ready[j]==s)
            {
              ready.Delete(j);
              break;
            }
          }
          killSocket(i);
          i--;
        }
      }
      for(i=0;i<error.Count();i++)
      {
        for(int j=0;j<sockets.Count();j++)
        {
          if(sockets[j]->getSocket()==error[i])
          {
            outTask->removeSocket(error[i]);
            killSocket(j);
            break;
          }
        }
      }
      for(i=0;i<ready.Count();i++)
      {
        SmppSocket *ss=
          (SmppSocket*)(ready[i]->getData(SOCKET_SLOT_INPUTSMPPSOCKET));
        //trace2("getdata:%p\n",ss);
        int retcode=ss->receive();
        if(retcode==-1)
        {
          for(int i=0;i<sockets.Count();i++)
          {
            if(sockets[i]==ss)
            {
              outTask->removeSocket(ss->getSocket());
              killSocket(i);
              break;
            }
          }
        }else
        if(retcode==1)
        {
          SmppHeader *pdu=ss->decode();
          if(!pdu)
          {
            for(int i=0;i<sockets.Count();i++)
            {
              if(sockets[i]==ss)
              {
                outTask->removeSocket(ss->getSocket());
                killSocket(i);
                break;
              }
            }
            continue;
          }
          switch(pdu->get_commandId())
          {
            case SmppCommandSet::BIND_RECIEVER:
            case SmppCommandSet::BIND_TRANSMITTER:
            case SmppCommandSet::BIND_TRANCIEVER:
            {
              PduBindTRX *bindpdu=
                reinterpret_cast<smsc::smpp::PduBindTRX*>(pdu);
              SmppProxy *proxy=new SmppProxy(ss);
              switch(pdu->get_commandId())
              {
                case SmppCommandSet::BIND_RECIEVER:
                  proxy->setProxyType(proxyReceiver);
                  break;
                case SmppCommandSet::BIND_TRANSMITTER:
                  proxy->setProxyType(proxyTranmitter);
                  break;
                case SmppCommandSet::BIND_TRANCIEVER:
                  proxy->setProxyType(proxyTransceiver);
                  break;
              }
              PduBindTRXResp resppdu;
              resppdu.get_header().
                set_commandId(pdu->get_commandId()|0x80000000);
              resppdu.get_header().
                set_sequenceNumber(pdu->get_sequenceNumber());
              resppdu.set_systemId("smsc");
              bool err=false;

              try{
                trace("try to register sme");
                smeManager->registerSmeProxy(
                  bindpdu->get_systemId(),
                  bindpdu->get_password()?bindpdu->get_password():"",
                  proxy);
                proxy->setId(bindpdu->get_systemId());
                resppdu.get_header().
                  set_commandStatus(SmppStatusSet::ESME_ROK);
              }catch(exception& e)
              {
                resppdu.get_header().
                  set_commandStatus(SmppStatusSet::ESME_RINVSYSID);
                trace2("registration failed:%s",e.what());
                //delete proxy;
                err=true;
              }

              char buf[64];
              int size=resppdu.size();
              __require__(size<(int)sizeof(buf));
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
              ss->assignProxy(proxy);
              if(err)
              {
                for(int i=0;i<sockets.Count();i++)
                {
                  if(sockets[i]==ss)
                  {
                    outTask->removeSocket(ss->getSocket());
                    killSocket(i);
                    break;
                  }
                }
              }else
              {
                ((SmppSocket*)(ss->getSocket()->
                  getData(SOCKET_SLOT_OUTPUTSMPPSOCKET)))->
                  assignProxy(proxy);
              }
            }break;
            case SmppCommandSet::UNBIND:
            {
              try{
                ss->getProxy()->putIncomingCommand
                (
                  SmscCommand::makeUnbindResp
                  (
                    pdu->get_sequenceNumber(),
                    SmppStatusSet::ESME_ROK
                  )
                );
              }catch(...)
              {
              }
              //ss->getSocket()->setData(SOCKET_SLOT_KILL,1);
            }break;
            case SmppCommandSet::SUBMIT_SM:
            case SmppCommandSet::SUBMIT_SM_RESP:
            case SmppCommandSet::DELIVERY_SM:
            case SmppCommandSet::DELIVERY_SM_RESP:
            {
              try{
                SmscCommand cmd(pdu);
                try{
                  ss->getProxy()->putIncomingCommand(cmd);
                }catch(...)
                {
                }
                break;
              }catch(...)
              {
              }
              //
              // Это так и задумано, здесь не должно быть break!
              //
            }
            default:
            {
              __trace__("Generating generic nack!");
              SmscCommand cmd=
                SmscCommand::makeGenericNack
                (
                  pdu->get_sequenceNumber(),
                  SmppStatusSet::ESME_RINVCMDID
                );
              try{
                ss->getProxy()->putCommand(cmd);
              }catch(...)
              {
              }
            }
          }
          disposePdu(pdu);
        }
      }
    }
  }
  trace("exiting smpp input thread loop");
  for(i=sockets.Count()-1;i>=0;i--)
  {
    killSocket(i);
  }
  return 0;
}

SmppOutputThread::~SmppOutputThread()
{
  while(sockets.Count())
  {
    removeSocket(sockets[0]->getSocket());
  }
}


void SmppOutputThread::addSocket(Socket* sock)
{
  mon.Lock();
  SmppSocket *s=new SmppSocket(ssModeWrite,sock);
  s->assignTasks(inTask,this);
  sock->setData(SOCKET_SLOT_OUTPUTSMPPSOCKET,s);
  sock->setData(SOCKET_SLOT_OUTPUTMULTI,0);
  sock->setData(SOCKET_SLOT_KILL,0);
  sockets.Push(s);
  mon.Unlock();
}


void SmppOutputThread::removeSocket(Socket *sock)
{
  mon.Lock();
  for(int i=0;i<sockets.Count();i++)
  {
    if(sockets[i]->getSocket()==sock)
    {
      if(sock->getData(SOCKET_SLOT_OUTPUTMULTI))
      {
        sock->setData(SOCKET_SLOT_KILL,(void*)1);
      }
      else
      {
        mon.Unlock();
        killSocket(i);
        mon.Lock();
      }
      break;
    }
  }
  mon.Unlock();
}

void SmppOutputThread::killSocket(int idx)
{
  SmppSocket *ss=sockets[idx];
  Socket *s=ss->getSocket();
  sockets.Delete(idx);
  trace2("removing socket %p by output thread",s);
  SmppSocketsManager *m=
    (SmppSocketsManager*)s->getData(SOCKET_SLOT_SOCKETSMANAGER);
  m->removeSocket(s);
  delete ss;
}

int SmppOutputThread::Execute()
{
  Multiplexer::SockArray ready;
  Multiplexer::SockArray error;
  Multiplexer::SockArray tokill;

  int i;
  while(!isStopping)
  {
    mon.Lock();
    while(sockets.Count()==0)
    {
      trace("out:wait for sockets");
      mon.wait();
      if(isStopping)break;
      trace("out:got new socket");
    }
    if(isStopping)break;
    int cnt=0;
    mul.clear();
    trace("check data for output");
    for(i=0;i<sockets.Count();i++)
    {
      SmppSocket *ss=sockets[i];
      Socket *s=ss->getSocket();
      if(s->getData(SOCKET_SLOT_KILL))
      {
        trace("try to remove socket from inTask");
        inTask->removeSocket(s);
        trace("remove ok. Killing socke");
        killSocket(i);
        i--;
        continue;
      }
      if(!ss->hasData() && ss->getProxy() && ss->getProxy()->hasOutput())
      {
        SmppHeader *pdu=ss->getProxy()->getOutgoingCommand().makePdu();
        trace2("SEND: seq number:%d",pdu->get_sequenceNumber());
        int size=calcSmppPacketLength(pdu);
        char* buf=ss->getBuffer(size);
        SmppStream st;
        assignStreamWith(&st,buf,size,false);
        fillSmppPdu(&st,pdu);
        disposePdu(pdu);
        ss->send(size);
        s->setData(SOCKET_SLOT_OUTPUTMULTI,(void*)1);
      }
      if(ss->hasData())
      {
        //s->getSocket()->setData(1,s);
        mul.add(s);
        cnt++;
      }
    }
    trace2("found %d sockets wanting to send data",cnt);
    if(!cnt)
    {
      trace("out: wait for data");
      //mon.Unlock();
      mon.wait();
      mon.Unlock();
      continue;
    }
    mon.Unlock();
    if(mul.canWrite(ready,error,1000)>0)
    {
      mon.Lock();
      for(i=0;i<sockets.Count();i++)
      {
        SmppSocket *ss=sockets[i];
        Socket *s=ss->getSocket();
        s->setData(SOCKET_SLOT_OUTPUTMULTI,0);
        if(s->getData(SOCKET_SLOT_KILL))
        {
          int j;
          for(j=0;j<error.Count();j++)
          {
            if(error[j]==s)
            {
              error.Delete(j);
            }
          }
          for(j=0;j<ready.Count();j++)
          {
            if(ready[j]==s)
            {
              ready.Delete(j);
              break;
            }
          }
          //inTask->removeSocket(s);
          tokill.Push(s);
          killSocket(i);
          i--;
        }
      }
      for(i=0;i<error.Count();i++)
      {
        for(int j=0;j<sockets.Count();j++)
        {
          if(sockets[j]->getSocket()==error[i])
          {
            //inTask->removeSocket(error[i]);
            tokill.Push(error[i]);
            killSocket(j);
            break;
          }
        }
      }
      for(i=0;i<ready.Count();i++)
      {
        SmppSocket *ss=
          (SmppSocket *)ready[i]->getData(SOCKET_SLOT_OUTPUTSMPPSOCKET);
        if(ss->send()==-1)
        {
          for(int j=0;j<sockets.Count();j++)
          {
            if(sockets[j]==ss)
            {
              //inTask->removeSocket(ss->getSocket());
              tokill.Push(ss->getSocket());
              killSocket(j);
              break;
            }
          }
        }
      }
      mon.Unlock();
      for(int j=0;j<tokill.Count();j++)
      {
        inTask->removeSocket(tokill[j]);
      }
      tokill.Clean();
    } // if ready
  } // main loop return
  for(i=sockets.Count()-1;i>=0;i--)
  {
    killSocket(i);
  }
  return 0;
} // Execute

};//smppio
};//system
};//smsc
