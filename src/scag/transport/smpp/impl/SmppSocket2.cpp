#include "SmppSocket2.h"
#include "smpp/smpp.h"
#include "util/int.h"
#include "system/status.h"
#include "SmppManager2.h"

namespace {

smsc::core::synchronization::Mutex digitmtx;

const unsigned digitlen = 3;
const std::string& digitstring()
{
    static bool done = false;
    static std::string ds;
    if ( ! done ) {
        MutexGuard mg(digitmtx);
        if ( ! done ) {
            ds.reserve(256*digitlen+1);
            char buf[10];
            for ( unsigned i = 0; i < 256; ++i ) {
                sprintf( buf, "%02x ", i );
                ds.append(buf);
            }
            done = true;
        }
    }
    return ds;
}

void bufdump( std::string& out, const unsigned char* inbuf, unsigned insize )
{
    out.reserve( out.size() + insize*digitlen + 10 );
    const char* digits = digitstring().c_str();
    for ( ; insize-- > 0; ++inbuf ) {
        out.append( digits + ((*inbuf)*digitlen), digitlen );
    }
}
}

namespace scag2 {
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
        disconnect();
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
        smsc_log_warn(log,"command received from %s too large:%d",getCachedPeer(),rdToRead);
        MutexGuard mg(mtx);
        disconnect();
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
      smsc_log_warn(log, "SmppSocket error from recv");
      disconnect();
      return;
  }
  lastActivity=time(NULL);
  rdBufUsed+=res;
  if(rdBufUsed<rdToRead)return;
  if(dump->isDebugEnabled())
  {
      std::string out;
      ::bufdump( out, reinterpret_cast<const unsigned char*>(rdBuffer), rdToRead );
      dump->log(smsc::logger::Logger::LEVEL_DEBUG, "in from %s(%s): %s",
                getCachedPeer(), systemId.c_str(), out.c_str());
  }
  SmppStream s;
  assignStreamWith(&s,rdBuffer,rdBufUsed,true);
  rdBufUsed=0;
  rdToRead=0;
  PduGuard pdu(smsc::smpp::fetchSmppPdu(&s));
  if(pdu.isNull())
  {
      smsc_log_warn(log, "Failed to parse pdu from %s, closing connection",getCachedPeer());
      disconnect();
      return;
  }
  if(processPdu(pdu))return;

  switch(pdu->get_commandId())
  {
    case SmppCommandSet::GENERIC_NACK:
    {
      smsc_log_warn(log, "Generic nack:%x/%x",pdu->get_sequenceNumber(),pdu->get_commandId());
    }break;

    case SmppCommandSet::BIND_RECIEVER:
    case SmppCommandSet::BIND_TRANSMITTER:
    case SmppCommandSet::BIND_TRANCIEVER:
    {
      smsc_log_debug(log, "uninmplemented %x",pdu->get_commandId());
    }break;

    case SmppCommandSet::BIND_RECIEVER_RESP:
    case SmppCommandSet::BIND_TRANSMITTER_RESP:
    case SmppCommandSet::BIND_TRANCIEVER_RESP:
    {
      smsc_log_debug(log, "uninmplemented %x",pdu->get_commandId());
    }break;

    case SmppCommandSet::UNBIND:
    {
      smsc_log_info(log, "unbind %s",systemId.c_str());
      putCommand(SmppCommand::makeUnbindResp(pdu->get_sequenceNumber(),0,0));
      if(bindType!=btNone)
      {
        smsc_log_debug(log, "unregisterChannel (bt=%d)", bindType);
        chReg->unregisterChannel(this);
      }
      bindType=btNone;
    }break;
    case SmppCommandSet::UNBIND_RESP:
    {
      smsc_log_debug(log, "uninmplemented %x",pdu->get_commandId());
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
      smsc_log_debug(log, "uninmplemented %x",pdu->get_commandId());
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
          std::auto_ptr<SmppCommand> cmd(new SmppCommand(pdu,false));
          cmdQueue->putCommand(this,cmd);
          break;
      }catch(std::exception& e)
      {
        smsc_log_warn(log, "putCommand exception:%s",e.what());
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
    smsc_log_debug(log, "sendData: %d/%d(%s)",wrBufSent,wrBufUsed,getCachedPeer());
  if(wrBufUsed && wrBufSent<wrBufUsed)
  {
    int res;
    {
      MutexGuard mg(mtx);
      res=sock->Write(wrBuffer,wrBufUsed-wrBufSent);
    }
    if(res<=0)
    {
        smsc_log_warn(log, "sendData: write failed:%d", res);
        disconnect();
        return;
    }
    //lastActivity=time(NULL);
    wrBufSent+=res;
    if(wrBufSent==wrBufUsed)
    {
      wrBufSent=0;
      wrBufUsed=0;
    }
    smsc_log_debug(log, "sendData: sent %d bytes",res);
    return;
  }
  using namespace smsc::smpp;
  PduGuard pdu;
  std::auto_ptr<SmppCommand> cmd;
  {
    MutexGuard mg(outMtx);
    if(outQueue.Count()==0)return;
    SmppCommand* c;
    outQueue.Pop(c);
    cmd.reset(c);
  }
    // smsc_log_debug(log, "received cmd=%p", cmd.get() );
    if (cmd->flagSet(SmppCommandFlags::EXPIRED_COMMAND)) return;
  pdu = cmd->makePdu();
  int sz = calcSmppPacketLength(pdu);
  if(sz>wrBufSize)
  {
    delete [] wrBuffer;
    wrBuffer=new char[sz];
    wrBufSize=sz;
  }
  smsc_log_debug(log,"Preparing to send %x/%d",
     pdu->get_commandId(), pdu->get_sequenceNumber());
  SmppStream st;
  assignStreamWith(&st,wrBuffer,wrBufSize,false);
  if(!fillSmppPdu(&st,pdu))
  {
    smsc_log_warn(log, "Failed to create buffer from pdu, skipping");
    wrBufUsed=0; wrBufSent=0;
    return;
  }
  if(dump->isDebugEnabled())
  {
      std::string out;
      ::bufdump( out, reinterpret_cast<const unsigned char*>(wrBuffer), sz );
      dump->log(smsc::logger::Logger::LEVEL_DEBUG, "out to %s(%s),%d: %s",
                getCachedPeer(), systemId.c_str(), outQueue.Count(), out.c_str());
  }
  wrBufSent=0;
  wrBufUsed=sz;
  smsc_log_debug(log,"Prepared buffer size %d",wrBufUsed);
}


void SmppSocket::genEnquireLink(int to)
{
    MutexGuard mg(outMtx);
    time_t now=time(NULL);
    if(now-lastEnquireLink<to)
    {
        return;
    }
    lastEnquireLink=now;
    SmppEntity* se = SmppManager::Instance().getSmppEntity(systemId.c_str());
    outQueue.Push(SmppCommand::makeCommand(ENQUIRELINK,se?se->getNextSeq():1,0,0).release());
}

}//smpp
}//transport
}//scag
