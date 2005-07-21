#ifndef __SCAG_TRANSPORT_SMPP_SMPPTYPES_H__
#define __SCAG_TRANSPORT_SMPP_SMPPTYPES_H__

namespace scag{
namespace transport{
namespace smpp{

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
