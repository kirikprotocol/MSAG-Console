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
  wrbuf.setSize(size);
  SmppStream stream;
  assignStreamWith(&stream,wrbuf.buffer,size,false);
  if(fillSmppPdu(&stream,reinterpret_cast<SmppHeader*>(&pdu)))
  {
    //for(int i=0;i<size;i++)printf("%02x ",(int)buf.buffer[i]);
    //printf("\n");
    if(!sendBuffer(wrbuf.buffer,size))throw Exception("Failed to send bind request");
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
  rdbuf.offset=0;
  socket.setTimeOut(to);
  __trace__("try read SMPP packet length");
  rdbuf.setSize(32);
  while(rdbuf.offset<4)
  {
    //__trace2__("read:%d,%d",rdbuf.offset,4-rdbuf.offset);
    int rd=socket.Read(rdbuf.buffer+rdbuf.offset,4-rdbuf.offset);
    if(rd<=0)throw Exception("SMPP transport connection error");
    rdbuf.offset+=rd;
    //__trace2__("read:%d",rdbuf.offset);
  }
  int sz=ntohl(*((int*)rdbuf.buffer));
  //__trace2__("SMPP packet length (%d:%d,%d,%d,%d):%d",rdbuf.offset,
  //  (int)buf.buffer[0],(int)buf.buffer[1],(int)buf.buffer[2],(int)buf.buffer[3],sz);
  rdbuf.setSize(sz);
  while(rdbuf.offset<sz)
  {
    int rd=socket.Read(rdbuf.current(),sz-rdbuf.offset);
    if(rd<=0)throw Exception("SMPP transport connection error");
    rdbuf.offset+=rd;
  }
  //for(int i=0;i<sz;i++)fprintf(stderr,"%02X ",(int)(unsigned char)rdbuf.buffer[i]);
  //fprintf(stderr,"\n");
  SmppStream s;
  assignStreamWith(&s,rdbuf.buffer,sz,true);
  return fetchSmppPdu(&s);
}

bool BaseSme::sendSms(smsc::sms::SMS* sms,int seq)
{
  PduXSm pdu;
  pdu.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
  pdu.get_header().set_sequenceNumber(seq);
  if(!fillSmppPduFromSms(&pdu,sms))
  {
    return false;
  }
  int sz=pdu.size(false);
  wrbuf.setSize(sz);
  SmppStream s;
  assignStreamWith(&s,wrbuf.buffer,sz,false);
  fillSmppPdu(&s,reinterpret_cast<SmppHeader*>(&pdu));
  if(!sendBuffer(wrbuf.buffer,sz))throw Exception("Failed to send sms");
  return true;
}

bool BaseSme::sendSmpp(SmppHeader* pdu)
{
  int sz=calcSmppPacketLength(pdu);
  wrbuf.setSize(sz);
  SmppStream s;
  assignStreamWith(&s,wrbuf.buffer,sz,false);
  if(!fillSmppPdu(&s,pdu))return false;
  if(!sendBuffer(wrbuf.buffer,sz))throw Exception("Failed to send smpp packet");
  return true;
}


bool BaseSme::sendBuffer(const char* buffer,int size)
{
  int count=0,wr;
  do{
    wr=socket.Write(buffer+count,size-count);
    if(wr<=0)return 0;
    count+=wr;
  }while(count!=size);
  return wr==size;
}



};//sme
};//smsc
