#ifndef _SCAG_TRANSPORT_SMPP_SMPPTYPES2_H
#define _SCAG_TRANSPORT_SMPP_SMPPTYPES2_H

namespace scag2 {
namespace transport {
namespace smpp {

enum SmppEntityType{
  etUnknown,etSmsc,etService
};

enum SmppBindType{
  btNone,btReceiver,btTransmitter,btRecvAndTrans,btTransceiver
};

}//smpp
}//transport
}//scag


#endif
