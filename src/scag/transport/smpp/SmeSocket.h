#ifndef __SCAG_TRANSPORT_SMPP_SMESOCKET_H__
#define __SCAG_TRANSPORT_SMPP_SMESOCKET_H__

#include "SmppSocket.h"
#include "system/status.h"

namespace scag{
namespace transport{
namespace smpp{

struct SmeSocket:SmppSocket{
  SmeSocket(net::Socket* s):SmppSocket(s)
  {
    sockType=etService;
  }

  bool processPdu(PduGuard& pdu)
  {
    namespace St=smsc::system::Status;
    SmppBindType bt=btNone;
    CommandId resp;
    int code=St::OK;
    switch(pdu->get_commandId())
    {
      case SmppCommandSet::BIND_RECIEVER:
        bt=btReceiver;
        resp=BIND_RECIEVER_RESP;
        break;
      case SmppCommandSet::BIND_TRANSMITTER:
        bt=btTransmitter;
        resp=BIND_TRANSMITTER_RESP;
        break;
      case SmppCommandSet::BIND_TRANCIEVER:
        bt=btTransceiver;
        resp=BIND_TRANCIEVER_RESP;
        break;
      default:return false;
    }
    PduBindTRX& bnd=pdu.as<PduBindTRX>();
    const char* sid=bnd.get_systemId();
    const char* pwd=bnd.get_password();
    pwd=pwd?pwd:"";
    if(!sid || !*sid || strlen(sid)>15)code=St::INVSYSID;
    if(strlen(pwd)>8)code=St::INVPASWD;
    if(code==St::OK)
    {
      int res=chReg->registerSmeChannel(sid,pwd,bt,this);
      switch(res)
      {
        case rarOk:
          bindType=bt;
          systemId=sid;
          break;
        case rarFailed:code=St::BINDFAIL;break;
        case rarAlready:code=St::ALYBND;break;
      }
    }
    putCommand(SmppCommand::makeCommand(resp,pdu->get_sequenceNumber(),code,0));
    return true;
  }
};

}//smpp
}//transport
}//scag


#endif
