#include "sme/sme.hpp"
#include "util/Exception.hpp"
#include <string.h>



namespace smsc{
namespace sme{

using namespace smsc::smpp;
using smsc::util::Exception;

bool BaseSme::init()
{
  if(socket.Init(smscHost.c_str(),smscPort,0)==-1)return false;
  if(socket.Connect()==-1)return false;
  return true;
}

void BaseSme::bindsme()
{
  PduBindTRX pdu;
  pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANCIEVER);
  pdu.get_header().set_sequenceNumber(getNextSeq());
  pdu.set_systemId(smeSystemId.c_str());
  int size=pdu.size();
  buf.setSize(size);
  SmppStream stream;
  assignStreamWith(&stream,buf.buffer,size,false);
  if(fillSmppPdu(&stream,reinterpret_cast<SmppHeader*>(&pdu)))
  {
    //for(int i=0;i<size;i++)printf("%02x ",(int)buf.buffer[i]);
    //printf("\n");
    if(!sendBuffer(buf.buffer,size))throw Exception("Failed to send bind request");
  }
  else
  {
    throw Exception("Failed to fill bind pdu");
  }
  trace("bind pdu sent\n");
  PduBindTRXResp *resp=reinterpret_cast<PduBindTRXResp*>(receiveSmpp(0));
  if(resp->get_header().get_sequenceNumber()!=pdu.get_header().get_sequenceNumber())
  {
    throw Exception("Unexpected response");
  }
  int status=resp->get_header().get_commandStatus();
  delete resp;
  if(status!=0) throw Exception("Failed to bind SME");
}

SmppHeader* BaseSme::receiveSmpp(int to)
{
  buf.offset=0;
  socket.setTimeOut(to);
  while(buf.offset<4)
  {
    int rd=socket.Read(buf.buffer+buf.offset,buf.size-buf.offset);
    if(rd<=0)throw Exception("SMPP transport connection error");
    buf.offset+=rd;
  }
  int sz=ntohl(*((int*)buf.buffer));
  buf.setSize(sz);
  while(buf.offset<sz)
  {
    int rd=socket.Read(buf.current(),buf.freeSpace());
    if(rd<=0)throw Exception("SMPP transport connection error");
    buf.offset+=rd;
  }
  SmppStream s;
  assignStreamWith(&s,buf.buffer,sz,true);
  return fetchSmppPdu(&s);
}

bool BaseSme::sendSms(smsc::sms::SMS* sms)
{
  PduXSm pdu;
  int seq=getNextSeq();
  pdu.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
  pdu.get_header().set_sequenceNumber(seq);
  if(!fillSmppPduFromSms(&pdu,sms))
  {
    return false;
  }
  int sz=pdu.size(false);
  buf.setSize(sz);
  SmppStream s;
  assignStreamWith(&s,buf.buffer,sz,false);
  fillSmppPdu(&s,reinterpret_cast<SmppHeader*>(&pdu));
  if(!sendBuffer(buf.buffer,sz))throw Exception("Failed to send sms");
  return true;
}


bool BaseSme::sendBuffer(const char* buffer,int size)
{
  int wr=socket.Write(buffer,size);
  return wr==size;
}



};//sme
};//smsc
