#include "SmppSocket.h"
#include "smpp/smpp.h"
#include "util/int.h"
#include "system/status.h"

namespace scag{
namespace transport{
namespace smpp{

using namespace smsc::smpp;

void SmppSocket::processInput()
{
  if(rdBufUsed<4)
  {
    int res;
    {
      MutexGuard mg(mtx);
      res=sock->Read(rdBuffer,4);
    }
    if(res<=0)
    {
      connected=false;
      return;
    }
    lastActivity=time(NULL);
    rdBufUsed+=res;
    return;
  }
  if(rdToRead==0)
  {
    //rdToRead=ntohl(*(uint32_t*)rdBuffer);
    memcpy(&rdToRead,rdBuffer,4);
    rdToRead=ntohl(rdToRead);
    if(rdToRead>70000)
    {
      char peer[32];
      sock->GetPeer(peer);
      smsc_log_warn(log,"command received from %s too large:%d",peer,rdToRead);
      MutexGuard mg(mtx);
      sock->Close();
      connected=false;
      return;
    }
    if(rdToRead>rdBufSize)
    {
      delete [] rdBuffer;
      rdBuffer=new char[rdToRead];
      rdBufSize=rdToRead;
      *(uint32_t*)rdBuffer=htonl(rdToRead);
    }
  }
  int res;
  {
    MutexGuard mg(mtx);
    res=sock->Read(rdBuffer+rdBufUsed,rdToRead-rdBufUsed);
  }
  if(res<=0)
  {
    connected=false;
    return;
  }
  lastActivity=time(NULL);
  rdBufUsed+=res;
  if(rdBufUsed<rdToRead)return;
  if(dump->isDebugEnabled())
  {
    std::string out;
    char buf[32];
    for(int i=0;i<rdToRead;i++)
    {
      sprintf(buf,"%02x",(unsigned char)rdBuffer[i]);
      out+=buf;
      out+=" ";
    }
    sock->GetPeer(buf);
    dump->log(smsc::logger::Logger::LEVEL_DEBUG,"in from %s(%s): %s",buf,systemId.c_str(),out.c_str());
  }
  SmppStream s;
  assignStreamWith(&s,rdBuffer,rdBufUsed,true);
  rdBufUsed=0;
  rdToRead=0;
  PduGuard pdu(smsc::smpp::fetchSmppPdu(&s));
  if(pdu.isNull())
  {
    char peer[32];
    sock->GetPeer(peer);
    smsc_log_warn(log,"Failed to parse pdu from %s, closing connection",peer);
    connected=false;
    return;
  }
  if(processPdu(pdu))return;

  switch(pdu->get_commandId())
  {
    case SmppCommandSet::GENERIC_NACK:
    {
      info2(log,"Generic nack:%x/%x",pdu->get_sequenceNumber(),pdu->get_commandId());
    }break;

    case SmppCommandSet::BIND_RECIEVER:
    case SmppCommandSet::BIND_TRANSMITTER:
    case SmppCommandSet::BIND_TRANCIEVER:
    {
      info2(log,"uninmplemented %x",pdu->get_commandId());
    }break;

    case SmppCommandSet::BIND_RECIEVER_RESP:
    case SmppCommandSet::BIND_TRANSMITTER_RESP:
    case SmppCommandSet::BIND_TRANCIEVER_RESP:
    {
      info2(log,"uninmplemented %x",pdu->get_commandId());
    }break;

    case SmppCommandSet::UNBIND:
    {
      info2(log,"unbind %s",systemId.c_str());
      putCommand(SmppCommand::makeUnbindResp(pdu->get_sequenceNumber(),0,0));
      if(bindType!=btNone)
      {
        info2(log,"unregisterChannel(bt=%d)",bindType);
        chReg->unregisterChannel(this);
      }
      bindType=btNone;
    }break;
    case SmppCommandSet::UNBIND_RESP:
    {
      info2(log,"uninmplemented %x",pdu->get_commandId());
    }break;

    case SmppCommandSet::ENQUIRE_LINK:
    {
      //info2(log,"uninmplemented %x",pdu->get_commandId());
      putCommand
      (
        SmppCommand::makeCommand
        (
          ENQUIRELINK_RESP,
          pdu->get_sequenceNumber(),
          smsc::system::Status::OK,0
        )
      );
    }break;
    case SmppCommandSet::ENQUIRE_LINK_RESP:
    {
      info2(log,"uninmplemented %x",pdu->get_commandId());
    }break;

    case SmppCommandSet::QUERY_SM:
    case SmppCommandSet::QUERY_SM_RESP:
    case SmppCommandSet::SUBMIT_SM:
    case SmppCommandSet::SUBMIT_SM_RESP:
    case SmppCommandSet::DELIVERY_SM:
    case SmppCommandSet::DELIVERY_SM_RESP:
    case SmppCommandSet::REPLACE_SM:
    case SmppCommandSet::REPLACE_SM_RESP:
    case SmppCommandSet::CANCEL_SM:
    case SmppCommandSet::CANCEL_SM_RESP:
    //case OUTBIND:
    case SmppCommandSet::SUBMIT_MULTI:
    case SmppCommandSet::SUBMIT_MULTI_RESP:
    case SmppCommandSet::ALERT_NOTIFICATION:
    case SmppCommandSet::DATA_SM:
    case SmppCommandSet::DATA_SM_RESP:
    {
      try{
        SmppCommand cmd(pdu,false);
        cmdQueue->putCommand(this,cmd);
        break;
      }catch(std::exception& e)
      {
        warn2(log,"exception:%s",e.what());
      }
    }//break; fallthru
    default:
    {
      putCommand(SmppCommand::makeGenericNack(pdu->get_commandStatus(),smsc::system::Status::INVCMDID));
    };
  }
}

void SmppSocket::sendData()
{
  char buf[32];
  sock->GetPeer(buf);
  debug2(log,"sendData: %d/%d(%s)",wrBufSent,wrBufUsed,buf);
  if(wrBufUsed && wrBufSent<wrBufUsed)
  {
    int res;
    {
      MutexGuard mg(mtx);
      res=sock->Write(wrBuffer,wrBufUsed-wrBufSent);
    }
    if(res<=0)
    {
      info2(log,"sendData: write failed:%d",res);
      connected=false;
      return;
    }
    lastActivity=time(NULL);
    wrBufSent+=res;
    if(wrBufSent==wrBufUsed)
    {
      wrBufSent=0;
      wrBufUsed=0;
    }
    debug2(log,"sendData: sent %d bytes",res);
    return;
  }
  using namespace smsc::smpp;
  PduGuard pdu;
  SmppCommand cmd;
  {
    MutexGuard mg(outMtx);
    if(outQueue.Count()==0)return;
    outQueue.Pop(cmd);
  }
  pdu=cmd.makePdu();
  int sz=calcSmppPacketLength(pdu);
  if(sz>wrBufSize)
  {
    delete [] wrBuffer;
    wrBuffer=new char[sz];
    wrBufSize=sz;
  }
  debug2(log,"Preparing to send %x/%d",pdu->get_commandId(),pdu->get_sequenceNumber());
  SmppStream st;
  assignStreamWith(&st,wrBuffer,wrBufSize,false);
  if(!fillSmppPdu(&st,pdu))
  {
    warn1(log,"Failed to create buffer from pdu, skipping");
    wrBufUsed=0;
    wrBufSent=0;
    return;
  }
  if(dump->isDebugEnabled())
  {
    std::string out;
    for(int i=0;i<sz;i++)
    {
      sprintf(buf,"%02x",(unsigned char)wrBuffer[i]);
      out+=buf;
      out+=" ";
    }
    sock->GetPeer(buf);
    dump->log(smsc::logger::Logger::LEVEL_DEBUG,"out to %s(%s): %s",buf,systemId.c_str(),out.c_str());
  }
  wrBufSent=0;
  wrBufUsed=sz;
  debug2(log,"Prepared buffer size %d",wrBufUsed);
}

}//smpp
}//transport
}//scag
