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
    if(res==-1)
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
    rdToRead=ntohl(*(uint32_t*)rdBuffer);
    if(rdToRead>70000)
    {
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
  if(res==-1)
  {
    connected=false;
    return;
  }
  lastActivity=time(NULL);
  rdBufUsed+=res;
  if(rdBufUsed<rdToRead)return;
  SmppStream s;
  assignStreamWith(&s,rdBuffer,rdBufUsed,true);
  rdBufUsed=0;
  rdToRead=0;
  PduGuard pdu(smsc::smpp::fetchSmppPdu(&s));
  if(pdu.isNull())
  {
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
      info2(log,"uninmplemented %x",pdu->get_commandId());
    }break;
    case SmppCommandSet::UNBIND_RESP:
    {
      info2(log,"uninmplemented %x",pdu->get_commandId());
    }break;

    case SmppCommandSet::ENQUIRE_LINK:
    {
      info2(log,"uninmplemented %x",pdu->get_commandId());
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
        cmdQueue->putCommand(bindType,cmd);
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
  if(wrBufUsed && wrBufSent<wrBufUsed)
  {
    int res;
    {
      MutexGuard mg(mtx);
      res=sock->Write(wrBuffer,wrBufUsed-wrBufSent);
    }
    if(res==-1)
    {
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

  SmppStream st;
  assignStreamWith(&st,wrBuffer,wrBufSize,false);
  fillSmppPdu(&st,pdu);
  wrBufSent=0;
  wrBufUsed=sz;
}

}//smpp
}//transport
}//scag
