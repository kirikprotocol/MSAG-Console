#include <stdio.h>
#include "smpp/smpp.h"
#include "core/network/Socket.hpp"
#include "core/threads/Thread.hpp"
#include <poll.h>
#include "core/buffers/TmpBuf.hpp"
#include <memory>
#include "util/timeslotcounter.hpp"

using namespace smsc::core::network;
using namespace smsc::smpp;
using namespace smsc::core::buffers;

struct FlooderCfg{
  std::string host;
  int port;
  std::string sysId;
  std::string pwd;
  std::string msg;
  int speed;

  std::string source;
  std::string destination;
};

bool fillPduAddress(const char* str,PduAddress& addr)
{
  char addr_value[32]={0,};
  int iplan,itype;
  int scaned = sscanf(str,".%d.%d.%20s",&itype,&iplan,addr_value);
  if ( scaned != 3 )
  {
    scaned = sscanf(str,"+%20[0123456789?]s",addr_value);
    if ( scaned )
    {
      itype=1;
      iplan=1;
    }
    else
    {
      scaned = sscanf(str,"%20[0123456789?]s",addr_value);
      if ( !scaned )
        return false;
      else
      {
        iplan=1;
        itype=0;
      }
    }
  }
  addr.set_numberingPlan(iplan);
  addr.set_typeOfNumber(itype);
  addr.set_value(addr_value);
  return true;
}

class Flooder:public smsc::core::threads::Thread{
public:
  Flooder(const FlooderCfg& argCfg):cfg(argCfg),stopped(false),counter(1,5){}
  int Execute()
  {
    Socket sck;
    if(sck.Init(cfg.host.c_str(),cfg.port,0)==-1)
    {
      printf("Failed to init socket %s:%d\n",cfg.host.c_str(),cfg.port);
      return -1;
    }
    if(sck.Connect()==-1)
    {
      printf("Failed to connect to %s:%d\n",cfg.host.c_str(),cfg.port);
      return -1;
    }
    pollfd pfd;
    pfd.events=POLLIN|POLLOUT;
    pfd.fd=sck.getSocket();

    if(!bindSme(sck))
    {
      return -1;
    }
    SmppStream ssWr;
    SmppStream ssRd;
    TmpBuf<char,1024> bufWr(0);
    TmpBuf<char,1024> bufRd(0);
    uint32_t sz;
    PduSubmitSm sbm;
    sbm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
    sbm.get_header().set_commandStatus(0);
    fillPduAddress(cfg.source.c_str(),sbm.get_message().get_source());
    fillPduAddress(cfg.destination.c_str(),sbm.get_message().get_dest());
    sbm.get_message().set_esmClass(0);
    sbm.get_message().set_dataCoding(3);
    sbm.get_message().set_shortMessage(cfg.msg.c_str(),(int)cfg.msg.length());
    int seq=1;
    while(!stopped)
    {

    }
    return 0;
  }

  void Stop()
  {
    stopped=true;
  }

protected:
  FlooderCfg cfg;
  bool stopped;
  smsc::util::TimeSlotCounter<> counter;

  bool bindSme(Socket& sck)
  {
    PduBindTRX bnd;
    bnd.get_header().set_commandId(SmppCommandSet::BIND_TRANCIEVER);
    bnd.get_header().set_sequenceNumber(0);
    bnd.get_header().set_commandStatus(0);
    bnd.set_systemId(cfg.sysId.c_str());
    bnd.set_password(cfg.sysId.c_str());
    uint32_t sz=calcSmppPacketLength(&bnd.get_header());
    TmpBuf<char,1024> buf(sz);
    SmppStream ss;
    assignStreamWith(&ss,buf.get(),sz,false);
    fillSmppPdu(&ss,&bnd.get_header());
    if(sck.WriteAll(buf.get(),sz)==-1)
    {
      printf("sme %s failed to send bind request\n",cfg.sysId.c_str());
      return false;
    }
    if(sck.ReadAll(buf.get(),4)==-1)
    {
      printf("failed to read bind_resp\n");
      return false;
    }
    memcpy(buf.get(),&sz,4);
    sz=ntohl(sz);
    if(sz<16 || sz>70000)
    {
      printf("invalid packet size %d\n",sz);
      return false;
    }
    buf.setSize(sz);
    if(sck.ReadAll(buf.get()+4,sz-4)==-1)
    {
      printf("faield to read bind_resp\n");
      return false;
    }
    assignStreamWith(&ss,buf.get(),sz,true);
    std::auto_ptr<SmppHeader> resp(fetchSmppPdu(&ss));
    if(resp->get_commandId()!=SmppCommandSet::BIND_TRANCIEVER_RESP)
    {
      printf("unexpected response commandId:%x\n",resp->get_commandId());
      return false;
    }
    if(resp->get_commandStatus()!=0)
    {
      printf("response status=%d\n",resp->get_commandStatus());
      return false;
    }
    return true;
  }

};

int main(int argc,char* argv[])
{
  return 0;
}
