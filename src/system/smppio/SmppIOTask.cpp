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


static void KillProxy(int ct,SmppProxy* proxy,SmeManager* smeManager)
{
  if(proxy && !proxy->Unref(ct))
  {
    try{
      __trace2__("unregistering smeId=%s",proxy->getSystemId());
      smeManager->unregisterSmeProxy(proxy->getSystemId());
    }catch(...)
    {
      __trace2__("failed to unregister");
    }
    __trace2__("KILLPROXY: %p(%s)",proxy,proxy->getSystemId());
    delete proxy;
  }
}

SmppInputThread::~SmppInputThread()
{
  while(sockets.Count())
  {
    removeSocket(sockets[0]->getSocket());
  }
}

void SmppInputThread::addSocket(Socket* sock,int to)
{
  MutexGuard g(mon);
  SmppSocket *s=new SmppSocket(ssModeRead,sock,to);
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
  sockets.Delete(idx);
  SmppSocketsManager *m=
    (SmppSocketsManager*)s->getData(SOCKET_SLOT_SOCKETSMANAGER);
  trace2("removing socket %p by input thread",s);
  int rcnt=m->removeSocket(s);
  KillProxy(ss->getChannelType(),ss->getProxy(),smeManager);
  delete ss;
}


static void SendGNack(SmppSocket* ss,int seq,int status)
{
  __trace2__("Generating generic nack:seq=%d,status=%d",seq,status);
  PduGenericNack pdu;
  pdu.get_header().set_commandId(SmppCommandSet::GENERIC_NACK);
  pdu.get_header().set_sequenceNumber(seq);
  pdu.get_header().set_commandStatus(status);
  char buf[64];
  int size=pdu.size();
  __require__(size<(int)sizeof(buf));
  SmppStream s;
  assignStreamWith(&s,buf,sizeof(buf),false);
  fillSmppPdu(&s,reinterpret_cast<SmppHeader*>(&pdu));
  int wroff=0,wr;
  while(wroff<size)
  {
    wr=ss->getSocket()->Write(buf+wroff,size-wroff);
    if(wr<=0)break;
    wroff+=wr;
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
        bool to=false;
        if(
          s->getData(SOCKET_SLOT_KILL) ||
          ss->isConnectionTimedOut() ||
          (ss->getProxy() && time(NULL)-ss->getLastUpdate()-inactivityTime>inactivityTimeOut)
          )
        {
          __trace__("SmppInputThread:: killing socket by request, timeout or invactivity timeout");
          s->Close();
          if(!s->getData(SOCKET_SLOT_KILL))outTask->removeSocket(s);
          killSocket(i);
          i--;
          continue;
        }
        if(time(NULL)-ss->getLastUpdate()>inactivityTime && time(NULL)-ss->getLastEL()>5)
        {
          ss->updateLastEL();
          if(ss->getProxy())
          {
            ss->getProxy()->putCommand
            (
              SmscCommand::makeCommand
              (
                ENQUIRELINK,
                ss->getProxy()->getNextSequenceNumber(),
                0
              )
            );
          }
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
          __trace2__("SmppInputThread: received commandId=%x",pdu->get_commandId());
          int errcode=Status::INVCMDID;
          switch(pdu->get_commandId())
          {
            case SmppCommandSet::BIND_RECIEVER:
            case SmppCommandSet::BIND_TRANSMITTER:
            case SmppCommandSet::BIND_TRANCIEVER:
            {
              if(ss->getProxy() && ss->getProxy()->isOpened())
              {
                SmppHeader *resp=(SmppHeader*)new PduBindTRXResp();
                resp->set_commandId(pdu->get_commandId()|0x80000000);
                resp->set_sequenceNumber(pdu->get_sequenceNumber());
                resp->set_commandStatus(SmppStatusSet::ESME_RALYBND);
                ((PduBindTRXResp*)resp)->set_scInterfaceVersion(0x34);
                SmscCommand cmd=SmscCommand::makeSmppPduCommand(resp);
                try{
                  ss->getProxy()->putCommand(cmd);
                }catch(...)
                {
                  __trace2__("SmppInputThread: failed to put outgoing command");
                }
                break;
              }
              PduBindTRX *bindpdu=
                reinterpret_cast<smsc::smpp::PduBindTRX*>(pdu);

              std::string sid=bindpdu->get_systemId()?bindpdu->get_systemId():"";

              SmppProxy *proxy=NULL;
              bool err=false;
              bool rebindproxy=false;

              try{
                proxy=(SmppProxy*)smeManager->checkSmeProxy(
                  bindpdu->get_systemId()?bindpdu->get_systemId():"",
                  bindpdu->get_password()?bindpdu->get_password():""
                );
                if(proxy)
                {
                  rebindproxy=true;
                  __trace2__("SmppProxy: rebind!");
                }
              }catch(...)
              {
                err=true;
              }

              if(!proxy)proxy=new SmppProxy(ss);

              SmeIndex proxyIndex;
              PduBindTRXResp resppdu;

              try{
                proxyIndex=smeManager->lookup(sid);
              }catch(...)
              {
                resppdu.get_header().
                  set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                err=true;
              }
              SmeInfo si;
              if(!err)
              {
                si=smeManager->getSmeInfo(proxyIndex);
                switch(pdu->get_commandId())
                {
                  case SmppCommandSet::BIND_RECIEVER:
                    if(!(si.bindMode==smeRX || si.bindMode==smeTRX))
                    {
                      resppdu.get_header().
                        set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                      err=true;
                    }
                    if(!err)
                    {
                      if(rebindproxy)
                      {
                        if(proxy->getBindMode()!=proxyTransmitter)
                        {
                          resppdu.get_header().
                            set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                          err=true;
                        }else
                        {
                          __trace2__("SmppProxy: upgrade to transceiver");
                          proxy->setProxyType(proxyTransceiver);
                          proxy->setReceiverSocket(ss);
                        }
                      }else
                        proxy->setProxyType(proxyReceiver);

                      ss->setChannelType(ctReceiver);
                      ((SmppSocket*)(ss->getSocket()->
                        getData(SOCKET_SLOT_OUTPUTSMPPSOCKET)))->
                        setChannelType(ctReceiver);
                    }
                    break;
                  case SmppCommandSet::BIND_TRANSMITTER:
                    if(!(si.bindMode==smeTX || si.bindMode==smeTRX))
                    {
                      resppdu.get_header().
                        set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                      err=true;
                    }
                    if(!err)
                    {
                      if(rebindproxy)
                      {
                        if(proxy->getBindMode()!=proxyReceiver)
                        {
                          resppdu.get_header().
                            set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                          err=true;
                        }else
                        {
                          __trace2__("SmppProxy: upgrade to transceiver");
                          proxy->setProxyType(proxyTransceiver);
                          proxy->setTransmitterSocket(ss);
                        }
                      }else
                        proxy->setProxyType(proxyTransmitter);

                      ss->setChannelType(ctTransmitter);
                      ((SmppSocket*)(ss->getSocket()->
                        getData(SOCKET_SLOT_OUTPUTSMPPSOCKET)))->
                        setChannelType(ctTransmitter);
                    }
                    break;
                  case SmppCommandSet::BIND_TRANCIEVER:
                    if(si.bindMode!=smeTRX)
                    {
                      resppdu.get_header().
                        set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                      err=true;
                    }
                    if(rebindproxy)err=true;
                    if(!err)
                    {
                      proxy->setProxyType(proxyTransceiver);
                      ss->setChannelType(ctTransceiver);
                      ((SmppSocket*)(ss->getSocket()->
                        getData(SOCKET_SLOT_OUTPUTSMPPSOCKET)))->
                        setChannelType(ctTransceiver);
                    }
                    break;
                }
              }
              resppdu.get_header().
                set_commandId(pdu->get_commandId()|0x80000000);
              resppdu.get_header().
                set_sequenceNumber(pdu->get_sequenceNumber());
              resppdu.set_systemId("smsc");

              if(bindpdu->get_password() && strlen(bindpdu->get_password())>8)
              {
                resppdu.get_header().
                  set_commandStatus(SmppStatusSet::ESME_RINVPASWD);
                __warning__("bind failed: password too long");
                err=true;
              }

              if(bindpdu->get_systemId() &&
                 strlen(bindpdu->get_systemId())>15)
              {
                resppdu.get_header().
                  set_commandStatus(SmppStatusSet::ESME_RINVSYSID);
                __warning__("bind failed: systemId too long");
                err=true;
              }

              if(bindpdu->get_systemType() &&
                 strlen(bindpdu->get_systemType())>12)
              {
                resppdu.get_header().
                  set_commandStatus(SmppStatusSet::ESME_RINVSYSTYP);
                __warning__("bind failed: systemType too long");
                err=true;
              }

              if(bindpdu->get_addressRange().get_value() &&
                 strlen(bindpdu->get_addressRange().get_value())>41)
              {
                resppdu.get_header().
                  set_commandStatus(SmppStatusSet::ESME_RINVPARLEN);
                __warning__("bind failed: systemType too long");
                err=true;
              }

              if(!err)
              {
                try{
                  if(!rebindproxy)
                  {
                    __trace2__("try to register sme:%s",sid.c_str());
                    smeManager->registerSmeProxy(
                      bindpdu->get_systemId()?bindpdu->get_systemId():"",
                      bindpdu->get_password()?bindpdu->get_password():"",
                      proxy);
                    proxy->setId(sid,proxyIndex);
                    __trace2__("NEWPROXY: p=%p, smid=%s, forceDC=%s",proxy,sid.c_str(),si.forceDC?"true":"false");
                  }else
                  {
                    proxy->AddRef();
                  }
                  resppdu.get_header().
                    set_commandStatus(SmppStatusSet::ESME_ROK);
                }catch(SmeRegisterException& e)
                {
                  int errcode=SmppStatusSet::ESME_RBINDFAIL;
                  switch(e.getReason())
                  {
                    case SmeRegisterFailReasons::rfUnknownSystemId:errcode=SmppStatusSet::ESME_RINVSYSID;
                    case SmeRegisterFailReasons::rfAlreadyRegistered:errcode=SmppStatusSet::ESME_RALYBND;
                    case SmeRegisterFailReasons::rfInvalidPassword:errcode=SmppStatusSet::ESME_RINVPASWD;
                    case SmeRegisterFailReasons::rfDisabled:errcode=SmppStatusSet::ESME_RBINDFAIL;
                    //case SmeRegisterFailReasons::rfInternalError:;
                  }
                  resppdu.get_header().
                    set_commandStatus(errcode);
                  trace2("registration failed:%s",e.what());
                  //delete proxy;
                  err=true;
                }
                catch(...)
                {
                  resppdu.get_header().
                    set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                  trace2("registration failed: unknown reason");
                  //delete proxy;
                  err=true;
                }
              }
              resppdu.set_scInterfaceVersion(0x34);
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
                trace2("sending:%d/%d",wroff,size);
              }
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
                if(!rebindproxy)delete proxy;
              }else
              {
                ss->assignProxy(proxy);
                ((SmppSocket*)(ss->getSocket()->
                  getData(SOCKET_SLOT_OUTPUTSMPPSOCKET)))->
                  assignProxy(proxy);
              }
            }break;
            case SmppCommandSet::UNBIND:
            {
              __trace__("Received UNBIND");
              try{
                if(ss->getProxy() && ss->getProxy()->isOpened())
                {
                  __trace2__("UNBINDRESP sent for %p",ss->getProxy());
                  ss->getProxy()->putCommand
                  (
                    SmscCommand::makeUnbindResp
                    (
                      pdu->get_sequenceNumber(),
                      Status::OK
                    )
                  );
                  ss->getProxy()->close();
                  KillProxy(ss->getChannelType(),ss->getProxy(),smeManager);
                  ss->assignProxy(0);
                }else
                {
                  SendGNack(ss,pdu->get_sequenceNumber(),SmppStatusSet::ESME_RINVBNDSTS);
                }
              }catch(...)
              {
              }
              //ss->getSocket()->setData(SOCKET_SLOT_KILL,1);
            }break;
            case SmppCommandSet::GENERIC_NACK:
            {
              __trace__("SmppInputThread: received gnack");
            }break;
            case SmppCommandSet::ENQUIRE_LINK_RESP:
            {
            }break;
            case SmppCommandSet::ENQUIRE_LINK:
            {
              //SmscCommand cmd(pdu);
              try{
                if(ss->getProxy() && ss->getProxy()->isOpened())
                {
                  ss->getProxy()->putCommand
                  (
                    SmscCommand::makeCommand
                    (
                      ENQUIRELINK_RESP,
                      pdu->get_sequenceNumber(),
                      0
                    )
                  );
                }else
                {
                  PduEnquireLink pduresp;
                  pduresp.get_header().set_commandId(SmppCommandSet::ENQUIRE_LINK_RESP);
                  pduresp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
                  char buf[64];
                  SmppStream s;
                  assignStreamWith(&s,buf,sizeof(buf),false);
                  fillSmppPdu(&s,reinterpret_cast<SmppHeader*>(&pduresp));
                  ss->getSocket()->WriteAll(buf,pduresp.size());
                }
              }catch(...)
              {
                __trace__("SmppInput: exception in putIncomingCommand, proxy limit or proxy died");
              }
            }break;
            case SmppCommandSet::SUBMIT_SM:
            //case SmppCommandSet::SUBMIT_SM_RESP:
            //case SmppCommandSet::DELIVERY_SM:
            case SmppCommandSet::DELIVERY_SM_RESP:
            case SmppCommandSet::REPLACE_SM:
            case SmppCommandSet::QUERY_SM:
            case SmppCommandSet::DATA_SM:
            case SmppCommandSet::DATA_SM_RESP:
            case SmppCommandSet::CANCEL_SM:
            case SmppCommandSet::SUBMIT_MULTI:
            {
              if(ss->getProxy())
              {
                try{
                  SmscCommand cmd
                  (
                    pdu,
                    smeManager->getSmeInfo(ss->getProxy()->getSmeIndex()).forceDC
                  );
                  try{
                    if(ss->getProxy()->isOpened())
                    {
                      ss->getProxy()->putIncomingCommand(cmd);
                    }else
                    {
                      SendGNack(ss,pdu->get_sequenceNumber(),SmppStatusSet::ESME_RINVBNDSTS);
                    }
                  }catch(...)
                  {
                    __trace__("SmppInput: exception in putIncomingCommand, proxy limit or proxy died");
                  }
                  break;
                }catch(...)
                {
                  __trace__("Failed to build command from pdu, sending gnack");
                  errcode=Status::INVOPTPARAMVAL;
                }
              }
              //
              // Это так и задумано, здесь не должно быть break!
              //
            }
            default:
            {
              if(!ss->getProxy() || !ss->getProxy()->isOpened())
              {
                SendGNack(ss,pdu->get_sequenceNumber(),SmppStatusSet::ESME_RINVBNDSTS);
              }else
              {
                ss->getProxy()->putCommand
                (
                  SmscCommand::makeGenericNack
                  (
                    pdu->get_sequenceNumber(),
                    errcode
                  )
                );
                //SendGNack(ss,pdu->get_sequenceNumber(),SmppStatusSet::ESME_RINVCMDID);
              }
              /*SmscCommand cmd=
                SmscCommand::makeGenericNack
                (
                  pdu->get_sequenceNumber(),
                  SmppStatusSet::ESME_RINVCMDID
                );
              try{
                if(ss->getProxy())
                {
                  ss->getProxy()->putCommand(cmd);
                }
              }catch(...)
              {
              }
              */
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
  sockets.Clean();
  return 0;
}

SmppOutputThread::~SmppOutputThread()
{
  while(sockets.Count())
  {
    removeSocket(sockets[0]->getSocket());
  }
}


void SmppOutputThread::addSocket(Socket* sock,int to)
{
  mon.Lock();
  SmppSocket *s=new SmppSocket(ssModeWrite,sock,to);
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
  int rcnt=m->removeSocket(s);
  KillProxy(ss->getChannelType(),ss->getProxy(),smeManager);
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
        trace("remove ok. Killing socket");
        killSocket(i);
        i--;
        continue;
      }
      if(!ss->hasData() && ss->getProxy() && ss->hasOutput())
      {
        SmscCommand cmd=ss->getProxy()->getOutgoingCommand();

        SmppHeader *pdu=cmd.makePdu(smeManager->getSmeInfo(ss->getProxy()->getSmeIndex()).forceDC);
        int cmdid=pdu->get_commandId();
        trace2("SmppOutThread: commandId=%x, seq number:%d",
          pdu->get_commandId(),pdu->get_sequenceNumber());
        int size=calcSmppPacketLength(pdu);
        char* buf=ss->getBuffer(size);
        SmppStream st;
        assignStreamWith(&st,buf,size,false);
        fillSmppPdu(&st,pdu);
        disposePdu(pdu);
        ss->send(size);
        s->setData(SOCKET_SLOT_OUTPUTMULTI,(void*)1);
        if(cmdid==SmppCommandSet::UNBIND_RESP)
        {
          trace2("SmppOutputThread: UNBIND_RESP, killing proxy:%p",ss->getProxy());
          try{
            smeManager->unregisterSmeProxy(ss->getProxy()->getSystemId());
          }catch(...)
          {
            __trace__("SmppOutputThread: failed to unregister");
          }
          KillProxy(ss->getChannelType(),ss->getProxy(),smeManager);
          ss->assignProxy(0);
        }
      }
      if(ss->hasData())
      {
        //s->getSocket()->setData(1,s);
        mul.add(s);
        cnt++;
      }
    }
    //trace2("found %d sockets wanting to send data",cnt);
    if(!cnt)
    {
      //trace("out: wait for data");
      //mon.Unlock();
      mon.wait(2000);
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
  sockets.Clean();
  return 0;
} // Execute

};//smppio
};//system
};//smsc
