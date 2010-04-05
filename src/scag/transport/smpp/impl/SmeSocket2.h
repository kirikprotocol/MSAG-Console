#ifndef __SCAG_TRANSPORT_SMPP_SMESOCKET2_H__
#define __SCAG_TRANSPORT_SMPP_SMESOCKET2_H__

#include "SmppSocket2.h"
#include "system/status.h"

namespace scag2 {
namespace transport {
namespace smpp {

struct SmeSocket:SmppSocket{
  SmeSocket(Socket* s):SmppSocket(s)
  {
      sockType=etService;
      bindFailed_ = true;
      connected = true;
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
          bindType = bt;
          setSystemId(sid);
          bindFailed_ = false;
          break;
        case rarFailed:code=St::BINDFAIL;break;
        case rarAlready: code=St::ALYBND; break;
      }
    }

    std::auto_ptr<BindRespCommand> brc;
    if (code==St::OK) {
        brc.reset(new BindRespCommand);
        brc->sysId = "msag";
        brc->interfaceVersion = std::min(bnd.get_interfaceVersion(),uint8_t(0x34));
    }
    putCommand(SmppCommand::makeCommand(resp,pdu->get_sequenceNumber(),code,brc.release()));
    return true;
  }
};

}//smpp
}//transport
}//scag


#endif
