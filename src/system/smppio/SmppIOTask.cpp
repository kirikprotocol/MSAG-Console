#ifndef DISABLE_TRACING
//#define DISABLE_TRACING
#endif

#include "system/smppio/SmppIOTask.hpp"
#include "system/smppio/SmppProxy.hpp"
#include "util/debug.h"
#include "system/smppio/SmppSocketsManager.hpp"
#include <exception>
#ifdef SNMP
#include "system/snmp/SnmpCounter.hpp"
#include "snmp/SnmpAgent.hpp"
#endif
//#include <memory>

namespace smsc{
namespace system{
namespace smppio{

//using std::auto_ptr;
using namespace smsc::smpp;
using namespace std;

/*
const int SOCKET_SLOT_REFCOUNT=0;
const int SOCKET_SLOT_SOCKETSMANAGER=1;
const int SOCKET_SLOT_INPUTSMPPSOCKET=2;
const int SOCKET_SLOT_OUTPUTSMPPSOCKET=3;
const int SOCKET_SLOT_INPUTMULTI=4;
const int SOCKET_SLOT_OUTPUTMULTI=5;
const int SOCKET_SLOT_KILL=6;
*/

SmppSocketData& getSocketData(Socket* s)
{
  return *((SmppSocketData*)s->getData(0));
}

static Mutex getKillMutex;

static SmppProxy* getRegisteredProxy(SmeManager* smeManager,const char* sysId,const char* pass,int ct)
{
  MutexGuard mg(getKillMutex);
  SmppProxy* proxy=dynamic_cast<SmppProxy*>(smeManager->checkSmeProxy(sysId,pass));
  if(!proxy)return 0;
  proxy->AddRef(ct);
  return proxy;
}

static void KillProxy(int ct,SmppProxy* proxy,SmeManager* smeManager)
{
  static smsc::logger::Logger* snmpLog=smsc::logger::Logger::getInstance("smpp.reg");
  MutexGuard mg(getKillMutex);
  int cnt=proxy?proxy->Unref(ct):0;
  std::string sysId=proxy?proxy->getSystemId():"unknown";
  if(proxy && cnt==0)
  {
#ifdef SNMP
    smsc::system::SnmpCounter::SmeTrapSeverities smeTrpSvrt=SnmpCounter::getInstance().getSmeSeverities(sysId.c_str());
#endif
    try{
#ifdef SNMP
      if(ct==ctTransceiver)
      {
        if(smeTrpSvrt.onUnregister!=-1)
        {
          char buf[1024];
          sprintf(buf,"ACTIVE ESME %s unregistered successfully (AlaramID=%s; severity=%d)",sysId.c_str(),sysId.c_str(),smeTrpSvrt.onUnregister);
          smsc::snmp::SnmpAgent::trap(proxy->getSystemId(),"ESME",(smsc::snmp::SnmpAgent::alertSeverity)smeTrpSvrt.onUnregister,buf);
        }
      }
#endif
      __trace2__("unregistering(%p) smeId=%s",proxy,sysId.c_str());
      smeManager->unregisterSmeProxy(proxy);
      smsc_log_debug(snmpLog,"unregister sme: %s successful",sysId.c_str());
    }catch(...)
    {
#ifdef SNMP
      if(ct==ctTransceiver)
      {
        if(smeTrpSvrt.onUnregister!=-1)
        {
          char buf[1024];
          sprintf(buf,"ACTIVE ESME %s unregistration failed (AlaramID=%s; severity=%d)",sysId.c_str(),sysId.c_str(),smeTrpSvrt.onUnregisterFailed);
          smsc::snmp::SnmpAgent::trap(proxy->getSystemId(),"ESME",(smsc::snmp::SnmpAgent::alertSeverity)smeTrpSvrt.onUnregisterFailed,buf);
        }
      }
#endif
      smsc_log_debug(snmpLog,"unregister sme: %s failed",proxy->getSystemId());
      __trace__("failed to unregister");
    }
    //__trace2__("KILLPROXY: %p(%s)",proxy,proxy->getSystemId());
    //delete proxy;
    proxy=0;
  }

#ifdef SNMP
  __trace2__("unreg: sid=%s, ct=%d, cnt=%d",sysId.c_str(),ct,cnt);
  if(proxy && ct!=ctTransceiver && (cnt%2))
  {
    smsc::system::SnmpCounter::SmeTrapSeverities smeTrpSvrt=SnmpCounter::getInstance().getSmeSeverities(sysId.c_str());
    if(smeTrpSvrt.onUnregister!=-1)
    {
      __trace2__("sending unreg trap for %s",proxy->getSystemId());
      char buf[1024];
      char trueSid[32];
      sprintf(trueSid,"%s_%s",sysId.c_str(),ct==ctReceiver?"rx":"tx");
      sprintf(buf,"ACTIVE ESME %s unregistered successfully (AlaramID=%s; severity=%d)",trueSid,trueSid,smeTrpSvrt.onUnregister);
      smsc::snmp::SnmpAgent::trap(trueSid,"ESME",(smsc::snmp::SnmpAgent::alertSeverity)smeTrpSvrt.onUnregister,buf);
    }
  }
#endif

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
  sockets.Push(s);
}


void SmppInputThread::removeSocket(Socket *sock)
{
  mon.Lock();
  getSocketData(sock).killSocket=true;
  for(int i=0;i<sockets.Count();i++)
  {
    if(sockets[i]->getSocket()==sock)
    {
      if(!getSocketData(sock).inMulti)
      {
        killSocket(i);
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
  getSocketData(s).killSocket=1;
  mul.remove(s);
  sockets.Delete(idx);
  SmppSocketsManager *m=getSocketData(s).socketManager;
  trace2("Removing socket %p from smppsocket %p by input thread",s,ss);
  mon.Unlock();
  KillProxy(ss->getChannelType(),ss->getProxy(),smeManager);
  mon.Lock();
  int rcnt=m->removeSocket(s);
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
  assignStreamWith(&s,buf,(unsigned)sizeof(buf),false);
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
  smsc::logger::Logger* snmpLog=smsc::logger::Logger::getInstance("snmp.reg");
  int i;
  while(!isStopping)
  {
    try
    {
      MutexGuard g(mon);
      while(sockets.Count()==0)
      {
        __trace__("in:wait for sockets");
        //mon.wait();
        mon.wait();
        if(isStopping)break;
        __trace__("in:got new socket");
      }
      if(isStopping)break;
      time_t now=time(NULL);
      for(i=0;i<sockets.Count();i++)
      {
        SmppSocket *ss=sockets[i];
        Socket *s=ss->getSocket();
        if(!getSocketData(s).inSocket)
        {
          getSocketData(s).inSocket=ss;
          mul.add(s);
        }
        bool to=false;
        SmppProxy *prx=ss->getProxy();
        if(
            getSocketData(s).killSocket ||
            ss->isConnectionTimedOut() ||
            (prx && now-ss->getLastUpdate()-inactivityTime>inactivityTimeOut) ||
            (prx && prx->isDisconnecting() && now-prx->getDisconnectTime()>10)||
            (!prx && now-ss->getConnectTime()>bindTimeout && ss->getChannelType()!=ctUnbound)
          )
        {
          bool evt[5]=
          {
            getSocketData(s).killSocket,
            ss->isConnectionTimedOut(),
            (prx && now-ss->getLastUpdate()-inactivityTime>inactivityTimeOut),
            (prx && now-prx->isDisconnecting() && now-prx->getDisconnectTime()>10),
            (!prx && now-ss->getConnectTime()>bindTimeout && ss->getChannelType()!=ctUnbound)
          };
          char smb[6]={evt[0]?'Y':'N',evt[1]?'Y':'N',evt[2]?'Y':'N',evt[3]?'Y':'N',evt[4]?'Y':'N',0};
          info2(log,"SmppInputThread:: killing socket %p by request, timeout or invactivity timeout:%s",ss,smb);
          s->Close();
          killSocket(i);
          i--;
          continue;
        }
        if(now-ss->getLastUpdate()>inactivityTime && now-ss->getLastEL()>5)
        {
          __trace2__("eqlink for %p/%p/%p",ss,ss->getSocket(),ss->getProxy());
          ss->updateLastEL();
          if(ss->getProxy())
          {
            ss->getProxy()->putCommand
            (
              SmscCommand::makeEnquireLink
              (
                ss->getProxy()->getNextSequenceNumber(),
                0,
                ss->getChannelType()
              )
            );
          }
        }
        getSocketData(s).inMulti=true;
      }
    }catch(std::exception& e)
    {
      __warning2__("exception in smppintask:%s",e.what());
      mon.Unlock();
    }catch(...)
    {
      __warning2__("exception in smppintask:%s","unknown");
      mon.Unlock();
    }
    //trace("in: canRead\n");
    try{
      if(mul.canRead(ready,error,100)>0)
      {
        MutexGuard g(mon);
        for(i=0;i<sockets.Count();i++)
        {
          Socket *s=sockets[i]->getSocket();
          if(!getSocketData(s).inSocket)
          {
            getSocketData(s).inSocket=sockets[i];
            mul.add(s);
          }
          getSocketData(s).inMulti=false;
          if(getSocketData(s).killSocket)
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
              killSocket(j);
              break;
            }
          }
        }
        for(i=0;i<ready.Count();i++)
        {
          SmppSocket *ss=getSocketData(ready[i]).inSocket;
          //trace2("getdata:%p\n",ss);
          int retcode=ss->receive();
          if(retcode==-1)
          {
            for(int j=0;j<sockets.Count();j++)
            {
              if(sockets[j]==ss)
              {
                killSocket(j);
                break;
              }
            }
          }else
          if(retcode==1)
          {
            SmppHeader *pdu=ss->decode();
            if(!pdu)
            {
              for(int j=0;j<sockets.Count();j++)
              {
                if(sockets[j]==ss)
                {
                  killSocket(j);
                  break;
                }
              }
              continue;
            }
            __trace2__("SmppInputThread: received commandId=%x, socket=%p",pdu->get_commandId(),ss->getSocket());
            int errcode=Status::INVCMDID;
            switch(pdu->get_commandId())
            {
              case SmppCommandSet::BIND_RECIEVER:
              case SmppCommandSet::BIND_TRANSMITTER:
              case SmppCommandSet::BIND_TRANCIEVER:
              {
                if(ss->getProxy())
                {
                  __trace2__("bind cmd: prx: %p, ct=%d, opnd=%d, dscnt=%d, unbnd=%d", ss->getProxy(), ss->getChannelType(), ss->getProxy()->isOpened(), ss->getProxy()->isDisconnecting(), ss->getProxy()->isUnbinding());
                }
                if(ss->getProxy() &&
                   (
                     ss->getChannelType()!=ctUnbound ||
                     !ss->getProxy()->isOpened() ||
                     ss->getProxy()->isDisconnecting() ||
                     ss->getProxy()->isUnbinding()
                   )
                  )
                {
                  __trace__("Decline bind of sme that is already bound, disconnecting or unbinding");
                  SmppHeader *resp=(SmppHeader*)new PduBindTRXResp();
                  resp->set_commandId(pdu->get_commandId()|0x80000000);
                  resp->set_sequenceNumber(pdu->get_sequenceNumber());
                  resp->set_commandStatus
                  (
                    ss->getProxy()->isOpened()?
                      SmppStatusSet::ESME_RALYBND:
                      SmppStatusSet::ESME_RBINDFAIL
                  );
#ifdef SNMP
                  if(resp->get_commandStatus()==SmppStatusSet::ESME_RBINDFAIL)
                  {
                    SnmpCounter::getInstance().incCounter(SnmpCounter::cntErr0xd,ss->getProxy()->getSystemId());
                  }
#endif
                  ((PduBindTRXResp*)resp)->set_scInterfaceVersion(0x34);
                  ((PduBindTRXResp*)resp)->set_systemId("smsc");
                  SmscCommand cmd=SmscCommand::makeSmppPduCommand
                  (
                    resp,
                    ss->getChannelType()==ctReceiver?1:2
                  );
                  try{
                    ss->getProxy()->putCommand(cmd);
                  }catch(...)
                  {
                    __trace__("SmppInputThread: failed to put outgoing command");
                  }
                  break;
                }
                PduBindTRX *bindpdu=
                  reinterpret_cast<smsc::smpp::PduBindTRX*>(pdu);

                SmeSystemId sid=bindpdu->get_systemId()?bindpdu->get_systemId():"";

                SmppProxy *proxy=NULL;
                bool err=false;
                bool rebindproxy=false;

                PduBindTRXResp resppdu;


                try{
                  proxy=getRegisteredProxy(
                    smeManager,
                    bindpdu->get_systemId()?bindpdu->get_systemId():"",
                    bindpdu->get_password()?bindpdu->get_password():"",
                    pdu->get_commandId()==SmppCommandSet::BIND_RECIEVER?
                    proxyReceiver:
                    pdu->get_commandId()==SmppCommandSet::BIND_TRANSMITTER?
                    proxyTransmitter:-1
                  );
                  if(proxy)
                  {
                    rebindproxy=true;
                    info2(log,"SmppProxy: rebind(%s):%p!",proxy->getSystemId(),proxy);
                  }
                }catch(SmeRegisterException& e)
                {
                  smsc_log_debug(snmpLog,"register sme: %s failed",sid.c_str());
#ifdef SNMP
                  SnmpCounter::SmeTrapSeverities trpSvrt=SnmpCounter::getInstance().getSmeSeverities(sid.c_str());
                  if(trpSvrt.onRegisterFailed!=-1)
                  {
                    char snmpMsg[1024];
                    sprintf(snmpMsg,"ACTIVE ESME %s registration failed (AlaramID=%s; severity=%d)",sid.c_str(),sid.c_str(),trpSvrt.onRegisterFailed);
                    smsc::snmp::SnmpAgent::trap(sid.c_str(),"ESME",(smsc::snmp::SnmpAgent::alertSeverity)trpSvrt.onRegisterFailed,snmpMsg);
                  }
#endif

                  int respcode=SmppStatusSet::ESME_RBINDFAIL;
                  switch(e.getReason())
                  {
                    case SmeRegisterFailReasons::rfUnknownSystemId:respcode=SmppStatusSet::ESME_RBINDFAIL;break;
                    case SmeRegisterFailReasons::rfAlreadyRegistered:respcode=SmppStatusSet::ESME_RALYBND;break;
                    case SmeRegisterFailReasons::rfInvalidPassword:respcode=SmppStatusSet::ESME_RBINDFAIL;break;
                    case SmeRegisterFailReasons::rfDisabled:respcode=SmppStatusSet::ESME_RBINDFAIL;break;
                    //case SmeRegisterFailReasons::rfInternalError:;
                  }
                  resppdu.get_header().set_commandStatus(respcode);
                  warn2(log,"registration failed:%s",e.what());
                  //delete proxy;
                  err=true;
                }
                catch(std::exception& e)
                {
                  warn2(log,"exception:%s",e.what());
                  err=true;
                  resppdu.get_header().set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                }
                catch(...)
                {
                  warn1(log,"Unknown exception");
                  err=true;
                  resppdu.get_header().set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                }

                SmeIndex proxyIndex;

                try{
                  proxyIndex=smeManager->lookup(sid);
                }catch(...)
                {
                  warn2(log,"smeMan->lookup failed for %s",sid.c_str());
                  resppdu.get_header().
                    set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                  err=true;
                }

                SmeInfo si;
                if(!err)
                {
                  si=smeManager->getSmeInfo(proxyIndex);
                  if(!proxy)
                  {
                    proxy=new SmppProxy(ss,totalLimit,si.proclimit,si.timeout);
                    proxy->updateSmeInfo(si);
                    info2(log,"SmppProxy: new(%s) %p!",si.systemId.c_str(),proxy);
                  }
                  switch(pdu->get_commandId())
                  {
                    case SmppCommandSet::BIND_RECIEVER:
                      if(!(si.bindMode==smeRX || si.bindMode==smeTRX))
                      {
                        info2(log,"SmppProxy: '%s' wrong bind mode %d",si.systemId.c_str(),si.bindMode);
                        resppdu.get_header().
                          set_commandStatus(SmppStatusSet::ESME_RALYBND);
                        err=true;
                      }
                      if(!err)
                      {
                        if(rebindproxy)
                        {
                          if(proxy->getBindMode()!=proxyTransmitter)
                          {
                            info2(log,"SmppProxy: '%s' rebind->already bound",si.systemId.c_str());
                            resppdu.get_header().
                              set_commandStatus(SmppStatusSet::ESME_RALYBND);
                            err=true;
                          }else
                          {
                            info2(log,"SmppProxy: upgrade(%s) %p to transceiver (rss=%p)",si.systemId.c_str(),proxy,ss);
                            proxy->setProxyType(proxyTransceiver);
                            proxy->setReceiverSocket(ss);
                          }
                        }else
                        {
                          proxy->setProxyType(proxyReceiver);
                          proxy->setTransmitterSocket(0);
                        }

                        ss->setChannelType(ctReceiver);
                        getSocketData(ss->getSocket()).outSocket->setChannelType(ctReceiver);
                      }
                      break;
                    case SmppCommandSet::BIND_TRANSMITTER:
                      if(!(si.bindMode==smeTX || si.bindMode==smeTRX))
                      {
                        info2(log,"SmppProxy: '%s' wrong bind mode %d",si.systemId.c_str(),si.bindMode);
                        resppdu.get_header().
                          set_commandStatus(SmppStatusSet::ESME_RALYBND);
                        err=true;
                      }
                      if(!err)
                      {
                        if(rebindproxy)
                        {
                          if(proxy->getBindMode()!=proxyReceiver)
                          {
                            info2(log,"SmppProxy: '%s' rebind->already bound",si.systemId.c_str());
                            resppdu.get_header().
                              set_commandStatus(SmppStatusSet::ESME_RALYBND);
                            err=true;
                          }else
                          {
                            __warning2__("SmppProxy: upgrade %p to transceiver (trs=%p)",proxy,ss);
                            proxy->setProxyType(proxyTransceiver);
                            proxy->setTransmitterSocket(ss);
                          }
                        }else
                        {
                          proxy->setProxyType(proxyTransmitter);
                          proxy->setReceiverSocket(0);
                        }

                        ss->setChannelType(ctTransmitter);
                        getSocketData(ss->getSocket()).outSocket->setChannelType(ctTransmitter);
                      }
                      break;
                    case SmppCommandSet::BIND_TRANCIEVER:
                      if(si.bindMode!=smeTRX)
                      {
                        info2(log,"SmppProxy: '%s' wrong bind mode %d",si.systemId.c_str(),si.bindMode);
                        resppdu.get_header().
                          set_commandStatus(SmppStatusSet::ESME_RALYBND);
                        err=true;
                      }
                      if(rebindproxy && proxy->isDualChannel())
                      {
                        info2(log,"SmppProxy: '%s' transceiver rebind->dual channel",si.systemId.c_str());
                        err=true;
                        resppdu.get_header().
                          set_commandStatus(SmppStatusSet::ESME_RALYBND);
                      }
                      if(!err)
                      {
                        if(rebindproxy)
                        {
                          proxy=new SmppProxy(ss,totalLimit,si.proclimit,si.timeout);
                          proxy->updateSmeInfo(si);
                          info2(log,"SmppProxy: new backup proxy(%s) %p!",si.systemId.c_str(),proxy);
                          rebindproxy=false;
                        }
                        proxy->setProxyType(proxyTransceiver);
                        getSocketData(ss->getSocket()).outSocket->setChannelType(ctTransceiver);
                        ss->setChannelType(ctTransceiver);
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
                  warn1(log,"bind failed: password too long");
                  err=true;
                }

                if(bindpdu->get_systemId() &&
                   strlen(bindpdu->get_systemId())>15)
                {
                  resppdu.get_header().
                    set_commandStatus(SmppStatusSet::ESME_RINVSYSID);
                  warn1(log,"bind failed: systemId too long");
                  err=true;
                }

                if(bindpdu->get_systemType() &&
                   strlen(bindpdu->get_systemType())>12)
                {
                  resppdu.get_header().
                    set_commandStatus(SmppStatusSet::ESME_RINVSYSTYP);
                  warn1(log,"bind failed: systemType too long");
                  err=true;
                }

                if(bindpdu->get_addressRange().get_value() &&
                   strlen(bindpdu->get_addressRange().get_value())>41)
                {
                  resppdu.get_header().
                    set_commandStatus(SmppStatusSet::ESME_RINVPARLEN);
                  warn1(log,"bind failed: systemType too long");
                  err=true;
                }

                if(!err)
                {
                  try{
                    if(!rebindproxy)
                    {
                      debug2(log,"try to register sme:%s",sid.c_str());
                      //smsc_log_debug(snmpLog,"register sme:%s",sid.c_str());
                      smeManager->registerSmeProxy(
                        bindpdu->get_systemId()?bindpdu->get_systemId():"",
                        bindpdu->get_password()?bindpdu->get_password():"",
                        proxy);

                      smsc_log_debug(snmpLog,"register sme: %s successful",sid.c_str());
#ifdef SNMP
                      SnmpCounter::SmeTrapSeverities trpSvrt=SnmpCounter::getInstance().getSmeSeverities(sid.c_str());
                      if(trpSvrt.onRegister!=-1)
                      {
                        char snmpMsg[1024];
                        char realSid[32];
                        if(pdu->get_commandId()!=SmppCommandSet::BIND_TRANCIEVER)
                        {
                          sprintf(realSid,"%s_%s",sid.c_str(),pdu->get_commandId()==SmppCommandSet::BIND_RECIEVER?"rx":"tx");
                        }else
                        {
                          sprintf(realSid,"%s",sid.c_str());
                        }
                        sprintf(snmpMsg,"CLEARED ESME %s registration successful (AlaramID=%s; severity=%d)",realSid,realSid,trpSvrt.onRegister);
                        smsc::snmp::SnmpAgent::trap(realSid,"ESME",(smsc::snmp::SnmpAgent::alertSeverity)trpSvrt.onRegister,snmpMsg);
                      }
#endif
                      proxy->setId(sid,proxyIndex);
                      info2(log,"NEWPROXY: p=%p, smid=%s, forceDC=%s",proxy,sid.c_str(),si.hasFlag(sfForceGsmDatacoding)?"true":"false");
                    }else
                    {
#ifdef SNMP
                      SnmpCounter::SmeTrapSeverities trpSvrt=SnmpCounter::getInstance().getSmeSeverities(sid.c_str());
                      if(trpSvrt.onRegister!=-1)
                      {
                        char snmpMsg[1024];
                        char realSid[32];
                        sprintf(realSid,"%s_%s",sid.c_str(),pdu->get_commandId()==SmppCommandSet::BIND_RECIEVER?"rx":"tx");
                        sprintf(snmpMsg,"CLEARED ESME %s registration successful (AlaramID=%s; severity=%d)",realSid,realSid,trpSvrt.onRegister);
                        smsc::snmp::SnmpAgent::trap(realSid,"ESME",(smsc::snmp::SnmpAgent::alertSeverity)trpSvrt.onRegister,snmpMsg);
                      }
#endif
                      smsc_log_debug(snmpLog,"register second channel of sme:%s",sid.c_str());
                      //proxy->AddRef();
                    }
                    resppdu.get_header().
                      set_commandStatus(SmppStatusSet::ESME_ROK);
                  }catch(SmeRegisterException& e)
                  {
                    smsc_log_debug(snmpLog,"register sme: %s failed",sid.c_str());
                    int respcode=SmppStatusSet::ESME_RBINDFAIL;
                    switch(e.getReason())
                    {
                      case SmeRegisterFailReasons::rfUnknownSystemId:respcode=SmppStatusSet::ESME_RBINDFAIL;break;
                      case SmeRegisterFailReasons::rfAlreadyRegistered:respcode=SmppStatusSet::ESME_RALYBND;break;
                      case SmeRegisterFailReasons::rfInvalidPassword:respcode=SmppStatusSet::ESME_RBINDFAIL;break;
                      case SmeRegisterFailReasons::rfDisabled:respcode=SmppStatusSet::ESME_RBINDFAIL;break;
                      //case SmeRegisterFailReasons::rfInternalError:;
                    }
                    resppdu.get_header().
                      set_commandStatus(respcode);
                    warn2(log,"registration failed:%s",e.what());
                    //delete proxy;
                    err=true;
                  }
                  catch(std::exception &ex)
                  {
                    resppdu.get_header().
                      set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                    warn2(log,"registration failed: %s", ex.what());
                    //delete proxy;
                    err=true;
                  }
                  catch(...)
                  {
                    resppdu.get_header().
                      set_commandStatus(SmppStatusSet::ESME_RBINDFAIL);
                    warn1(log,"registration failed: unknown reason");
                    //delete proxy;
                    err=true;
                  }
                  if(!err)
                  {
                    try{
                      proxy->putIncomingCommand(SmscCommand::makeSMEAlert(proxyIndex),0);
                    }catch(...)
                    {
                    }
                  }
                }
#ifdef SNMP
                if(err)
                {
                  SnmpCounter::SmeTrapSeverities trpSvrt=SnmpCounter::getInstance().getSmeSeverities(sid.c_str());
                  if(trpSvrt.onRegisterFailed!=-1)
                  {
                    char snmpMsg[1024];
                      char realSid[32];
                      if(pdu->get_commandId()!=SmppCommandSet::BIND_TRANCIEVER)
                      {
                        sprintf(realSid,"%s_%s",sid.c_str(),pdu->get_commandId()==SmppCommandSet::BIND_RECIEVER?"rx":"tx");
                      }else
                      {
                        sprintf(realSid,"%s",sid.c_str());
                      }
                      sprintf(realSid,"%s_%s",sid.c_str(),pdu->get_commandId()==SmppCommandSet::BIND_RECIEVER?"rx":"tx");
                      sprintf(snmpMsg,"ACTIVE ESME %s registration failed (AlaramID=%s; severity=%d)",realSid,realSid,trpSvrt.onRegisterFailed);
                      smsc::snmp::SnmpAgent::trap(realSid,"ESME",(smsc::snmp::SnmpAgent::alertSeverity)trpSvrt.onRegisterFailed,snmpMsg);
                  }
                }
                if(resppdu.get_header().get_commandStatus()==SmppStatusSet::ESME_RBINDFAIL)
                {
                  SnmpCounter::getInstance().incCounter(SnmpCounter::cntErr0xd,sid.c_str());
                }
#endif
                resppdu.set_scInterfaceVersion(0x34);
                char buf[64];
                int size=resppdu.size();
                __require__(size<(int)sizeof(buf));
                SmppStream s;
                assignStreamWith(&s,buf,(unsigned)sizeof(buf),false);
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
                  for(int j=0;j<sockets.Count();j++)
                  {
                    if(sockets[j]==ss)
                    {
                      killSocket(j);
                      break;
                    }
                  }
                  if(rebindproxy)
                  {
                    if(pdu->get_commandId()==SmppCommandSet::BIND_RECIEVER)
                    {
                      warn1(log,"rebind failed, removing receiver channel references");
                      KillProxy(proxyReceiver,proxy,smeManager);
                      KillProxy(proxyReceiver,proxy,smeManager);
                    }else
                    {
                      warn1(log,"rebind failed, removing transmitter channel references");
                      KillProxy(proxyTransmitter,proxy,smeManager);
                      KillProxy(proxyTransmitter,proxy,smeManager);
                    }
                  }else
                  {
                    if(proxy)delete proxy;
                  }
                }else
                {
                  proxy->setSmeFlags(si.flags);
                  ss->assignProxy(proxy);
                  ss->setSystemId(si.systemId.c_str());
                  __trace2__("assign proxy: %p/%p",ss,proxy);
                  __trace2__("assign proxy: %p/%p",getSocketData(ss->getSocket()).outSocket,proxy);
                  getSocketData(ss->getSocket()).outSocket->assignProxy(proxy);
                  getSocketData(ss->getSocket()).outSocket->setSystemId(si.systemId.c_str());
                }
              }break;
              case SmppCommandSet::UNBIND_RESP:
              {
                SmppProxy* proxy=ss->getProxy();
                if(proxy && proxy->isDisconnecting())
                {
                  __trace__("UNBIND_RESP received, disconnecting");
                  ss->getSocket()->Close();
                  proxy->close();
                }
              }break;
              case SmppCommandSet::UNBIND:
              {
                try{
                  if(ss->getProxy() && ss->getProxy()->isOpened())
                  {
                    info2(log,"Received UNBIND(%s):%p",ss->getProxy()->getSystemId(),ss->getProxy());
                    __trace2__("UNBINDRESP sent for %p",ss->getProxy());
                    ss->getProxy()->putCommand
                    (
                      SmscCommand::makeUnbindResp
                      (
                        pdu->get_sequenceNumber(),
                        Status::OK,
                        ss->getChannelType()
                      )
                    );
                    //ss->getProxy()->close();
                    ss->getProxy()->Unbind();
                    mon.Unlock();
                    KillProxy(ss->getChannelType(),ss->getProxy(),smeManager);
                    mon.Lock();
                    ss->assignProxy(0);
                    ss->setChannelType(ctUnbound);
                  }else
                  {
                    SendGNack(ss,pdu->get_sequenceNumber(),SmppStatusSet::ESME_RINVBNDSTS);
                  }
                }catch(...)
                {
                  warn1(log,"Exception during attempt to send bind response");
                }
                //ss->getSocket()->setData(SOCKET_SLOT_KILL,1);
              }break;
              case SmppCommandSet::GENERIC_NACK:
              {
                char buf[32];
                ss->getSocket()->GetPeer(buf);
                warn2(log,"SmppInputThread: received gnack from %s",buf);
              }break;
              case SmppCommandSet::ENQUIRE_LINK_RESP:
              {
                char buf[32];
                ss->getSocket()->GetPeer(buf);
                debug2(log,"ENQUIRE_LINK_RESP:%s",buf);
              }break;
              case SmppCommandSet::ENQUIRE_LINK:
              {
                //SmscCommand cmd(pdu);
                try{
                  if(ss->getProxy() && ss->getProxy()->isOpened())
                  {
                    ss->getProxy()->putCommand
                    (
                      SmscCommand::makeEnquireLinkResp
                      (
                        pdu->get_sequenceNumber(),
                        Status::OK,
                        ss->getChannelType()
                      )
                    );
                  }else
                  {
                    PduEnquireLink pduresp;
                    pduresp.get_header().set_commandId(SmppCommandSet::ENQUIRE_LINK_RESP);
                    pduresp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
                    char buf[64];
                    SmppStream s;
                    assignStreamWith(&s,buf,(unsigned)sizeof(buf),false);
                    fillSmppPdu(&s,reinterpret_cast<SmppHeader*>(&pduresp));
                    ss->getSocket()->WriteAll(buf,pduresp.size());
                  }
                }catch(exception& e)
                {
                  __warning2__("SmppInput: exception in putCommand[%s]:%s",ss->getProxy()?ss->getProxy()->getSystemId():"unknown",e.what());
                }
                catch(...)
                {
                  __warning2__("SmppInput: exception in putCommand[%s]:unknown",ss->getProxy()?ss->getProxy()->getSystemId():"unknown");
                }
              }break;
              case SmppCommandSet::SUBMIT_SM:
              {
                PduSubmitSm* sbm=(PduSubmitSm*)pdu;
                if(sbm->get_message().get_smLength()==255)
                {
                  SmscCommand answer=SmscCommand::makeSubmitSmResp
                         (
                           "",
                           pdu->get_sequenceNumber(),
                           Status::INVPARLEN,
                           false
                         );
                  if(ss->getProxy())
                  {
                    ss->getProxy()->putCommand(answer);
                  }
                  break;
                }
              }//no break on purpose!!!
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
                      ss->getProxy()->getSmeFlags()
                    );
                    try{
                      if(ss->getProxy()->isOpened())
                      {
                        ss->getProxy()->putIncomingCommand(cmd,ss->getChannelType());
                      }else
                      {
                        SendGNack(ss,pdu->get_sequenceNumber(),SmppStatusSet::ESME_RINVBNDSTS);
                      }
                    }catch(ProxyLimitException& e)
                    {
                      __warning2__("SmppInput: exception in putCommand[%s]:%s",ss->getProxy()?ss->getProxy()->getSystemId():"unknown",e.what());
                      SmscCommand answer;
                      bool haveAnswer=true;
                      switch(cmd->get_commandId())
                      {
                        case SUBMIT:
                        {
                          answer=SmscCommand::makeSubmitSmResp
                                 (
                                   "",
                                   cmd->get_dialogId(),
                                   Status::THROTTLED,
                                   cmd->get_sms()->getIntProperty(Tag::SMPP_DATA_SM)
                                 );
                        }break;
                        case QUERY:
                        {
                          answer=SmscCommand::makeQuerySmResp
                                 (
                                   cmd->get_dialogId(),
                                   Status::THROTTLED,
                                   0,0,0,0
                                 );
                        }break;
                        case REPLACE:
                        {
                          answer=SmscCommand::makeReplaceSmResp
                                 (
                                   cmd->get_dialogId(),
                                   Status::THROTTLED
                                 );
                        }break;
                        case CANCEL:
                        {
                          answer=SmscCommand::makeCancelSmResp
                                 (
                                   cmd->get_dialogId(),
                                   Status::THROTTLED
                                 );
                        }break;
                        case SUBMIT_MULTI_SM:
                        {
                          answer=SmscCommand::makeSubmitMultiResp
                                 (
                                   "",
                                   cmd->get_dialogId(),
                                   Status::THROTTLED
                                 );
                        }break;
                        default:haveAnswer=false;break;
                      }
                      if(haveAnswer)
                      {
                        try{
                          ss->getProxy()->putCommand(answer);
                        }catch(...)
                        {
                          __warning__("SmppInput: failed to put error answer into proxy");
                        }
                      }
                    }catch(...)
                    {
                      __warning__("SmppInput: exception in putIncomingCommand, proxy died?");
                    }
                    break;
                  }
                  catch(std::exception& e)
                  {
                    __warning2__("Failed to build command from pdu, sending gnack:%s",e.what());
                    errcode=Status::INVOPTPARAMVAL;
                  }
                  catch(...)
                  {
                    __warning__("Failed to build command from pdu, sending gnack");
                    errcode=Status::INVOPTPARAMVAL;
                  }
                }
                //
                // ��� ��� � ��������, ����� �� ������ ���� break!
                //
              }// no break on purpose!
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
    }catch(std::exception& e)
    {
      __warning2__("exception2 in smppintask:%s",e.what());
      mon.Unlock();
    }catch(...)
    {
      __warning2__("exception2 in smppintask:%s","unknown");
      mon.Unlock();
    }
  }
  __trace__("exiting smpp input thread loop");
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
  getSocketData(sock).outSocket=s;
  sockets.Push(s);
  mon.Unlock();
}


void SmppOutputThread::removeSocket(Socket *sock)
{
  mon.Lock();
  getSocketData(sock).killSocket=true;
  for(int i=0;i<sockets.Count();i++)
  {
    if(sockets[i]->getSocket()==sock)
    {
      if(!getSocketData(sock).outMulti)
      {
        killSocket(i);
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
  getSocketData(s).killSocket=true;
  sockets.Delete(idx);
  trace2("removing socket %p by output thread",s);
  SmppSocketsManager *m=getSocketData(s).socketManager;
  mon.Unlock(); // this method is always called only with locked mon.
  KillProxy(ss->getChannelType(),ss->getProxy(),smeManager);
  mon.Lock();
  int rcnt=m->removeSocket(s);
  delete ss;
}

int SmppOutputThread::Execute()
{
  Multiplexer::SockArray ready;
  Multiplexer::SockArray error;
  Multiplexer::SockArray tokill;

  smsc::logger::Logger *olog=smsc::logger::Logger::getInstance("smpp.out");

  int i;
  while(!isStopping)
  {
    try{
      mon.Lock();
      while(sockets.Count()==0)
      {
        __trace__("out:wait for sockets");
        mon.wait();
        if(isStopping)break;
        __trace__("out:got new socket");
      }
      if(isStopping)
      {
        mon.Unlock();
        break;
      }
      int cnt=0;
      mul.clear();
      //trace("check data for output");

      time_t now=time(NULL);
      for(i=0;i<sockets.Count();i++)
      {
        SmppSocket *ss=sockets[i];
        Socket *s=ss->getSocket();
        if(now-getSocketData(s).lastUpdateTime-inactivityTime>inactivityTimeOut*2)
        {
          info2(log,"Closing socket %p: double inactivity timeout reached",s);
          s->Close();
        }

        if(getSocketData(s).killSocket)
        {
          killSocket(i);
          i--;
          continue;
        }
        SmscCommand cmd;
        bool haveCmd=false;
        if(!ss->hasData() && ss->getProxy())
        {
          mon.Unlock();
          haveCmd=ss->getOutgoingCommand(cmd);
          mon.Lock();
        }
        if(haveCmd)
        {

          SmppHeader *pdu=0;
          try{
            pdu=cmd.makePdu(ss->getProxy()->getSmeFlags());
          }catch(...)
          {
            warn2(olog,"Failed to build pdu from command:%d,dlgid=%d",cmd->get_commandId(),cmd->get_dialogId());
            continue;
          }
          if(pdu==0)continue;
          int cmdid=pdu->get_commandId();
          trace2("SmppOutThread: commandId=%x, seq number:%d,socket=%p",
            pdu->get_commandId(),pdu->get_sequenceNumber(),ss->getSocket());
          int size=calcSmppPacketLength(pdu);
          char* buf=ss->getBuffer(size);
          SmppStream st;
          assignStreamWith(&st,buf,size,false);
          fillSmppPdu(&st,pdu);
          disposePdu(pdu);
          ss->send(size);
          getSocketData(s).outMulti=true;
          if(cmdid==SmppCommandSet::UNBIND_RESP)
          {
            trace2("SmppOutputThread: UNBIND_RESP, killing proxy:%p/%p/%p",ss,ss->getSocket(),ss->getProxy());
            /*try{
              smeManager->unregisterSmeProxy(ss->getProxy()->getSystemId());
            }catch(...)
            {
              __trace__("SmppOutputThread: failed to unregister");
            }*/
            mon.Unlock();
            KillProxy(ss->getChannelType(),ss->getProxy(),smeManager);
            mon.Lock();
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
    }catch(std::exception& e)
    {
      __warning2__("exception in smppouttask:%s",e.what());
      mon.Unlock();
    }catch(...)
    {
      __warning2__("exception in smppouttask:%s","unknown");
      mon.Unlock();
    }
    try{
      if(mul.canWrite(ready,error,1)>0)
      {
        mon.Lock();
        for(i=0;i<sockets.Count();i++)
        {
          SmppSocket *ss=sockets[i];
          Socket *s=ss->getSocket();
          getSocketData(s).outMulti=false;
          if(getSocketData(s).killSocket)
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
              killSocket(j);
              break;
            }
          }
        }
        for(i=0;i<ready.Count();i++)
        {
          SmppSocket *ss=getSocketData(ready[i]).outSocket;
          if(ss->send()==-1)
          {
            for(int j=0;j<sockets.Count();j++)
            {
              if(sockets[j]==ss)
              {
                killSocket(j);
                break;
              }
            }
          }
        }
        mon.Unlock();
      }
    }catch(std::exception& e)
    {
      __warning2__("exception2 in smppouttask:%s",e.what());
      mon.Unlock();
    }catch(...)
    {
      __warning2__("exception2 in smppouttask:%s","unknown");
      mon.Unlock();
    }
  } // main loop return
  mon.Lock();
  for(i=sockets.Count()-1;i>=0;i--)
  {
    killSocket(i);
  }
  mon.Unlock();
  sockets.Clean();
  return 0;
} // Execute

}//smppio
}//system
}//smsc
