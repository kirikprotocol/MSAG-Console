#ifndef __SCAG_TRANSPORT_SMPP_SMSCSOCKET_H__
#define __SCAG_TRANSPORT_SMPP_SMSCSOCKET_H__

#include "SmppSocket.h"
#include <string>

namespace scag{
namespace transport{
namespace smpp{

struct SmscSocket:SmppSocket{
  SmscSocket(const char* argHost,int argPort):host(argHost),port(argPort)
  {
    sockType=etSmsc;
    sock=new net::Socket();
  }
  bool connect()
  {
    if(sock->Init(host.c_str(),port,0)==-1)
    {
      info2(log,"Failed to resolve host %s",host.c_str());
      return false;
    }
    if(sock->Connect()==-1)
    {
      info2(log,"Failed to connect to %s:%d",host.c_str(),port);
      return false;
    }
    return true;
  }
  void bind(const char* sysId,const char* pass)
  {
    systemId=sysId;
    putCommand(SmppCommand::makeBindCommand(sysId,pass));
  }

  bool processPdu(PduGuard& pdu)
  {
    switch(pdu->get_commandId())
    {
      case smsc::smpp::SmppCommandSet::BIND_TRANCIEVER_RESP:
      {
        if(chReg->registerSmscChannel(this)!=rarOk)
        {
          warn1(log,"Registration of smsc channel failed???");
        }else
        {
          bindType=btTransceiver;
        }
      }break;
      default: return false;
    }
    return true;
  }

protected:
  std::string host;
  int port;
};

}//smpp
}//transport
}//scag


#endif
