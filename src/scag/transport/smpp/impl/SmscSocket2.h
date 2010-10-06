#ifndef __SCAG_TRANSPORT_SMPP_SMSCSOCKET2_H__
#define __SCAG_TRANSPORT_SMPP_SMSCSOCKET2_H__

#include "SmppSocket2.h"
#include <string>

namespace scag2 {
namespace transport {
namespace smpp {

struct SmscSocket:SmppSocket{
  SmscSocket(const char* argHost,int argPort):host(argHost),port(argPort)
  {
    sockType=etSmsc;
    sock=new Socket();
    sock->setData(0,this);
  }
  bool connect(const std::string& bindHost)
  {
    if(sock->Init(host.c_str(),port,0)==-1)
    {
      smsc_log_warn(log, "Failed to resolve host %s",host.c_str());
      return false;
    }
    if(sock->ConnectEx(false,bindHost.c_str())==-1)
    {
      smsc_log_warn(log, "Failed to connect to %s:%d",host.c_str(),port);
      return false;
    }
    connected=true;
    return true;
  }
  void bind(const char* regSysId,const char* sysId,const char* pass,const char* addrRange,const char* sysType)
  {
    setSystemId(regSysId);
    putCommand(SmppCommand::makeBindCommand(sysId,pass,addrRange,sysType));
  }

  bool processPdu(PduGuard& pdu)
  {
    switch(pdu->get_commandId())
    {
      case smsc::smpp::SmppCommandSet::BIND_TRANCIEVER_RESP:
      {
        if(pdu->get_commandStatus()) {
            smsc_log_warn(log, "SMSC bind failed: RESP status=%u",unsigned(pdu->get_commandStatus()));
        } else if (chReg->registerSmscChannel(this)!=rarOk) {
            smsc_log_warn(log, "Registration of smsc channel failed???");
        } else {
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
