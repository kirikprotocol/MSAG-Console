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

namespace scag2 {
namespace transport {
namespace smpp {

using scag::transport::smpp::SmppEntityType;
using scag::transport::smpp::etUnknown;
using scag::transport::smpp::etSmsc;
using scag::transport::smpp::etService;

using scag::transport::smpp::SmppBindType;
using scag::transport::smpp::btNone;
using scag::transport::smpp::btReceiver;
using scag::transport::smpp::btTransmitter;
using scag::transport::smpp::btRecvAndTrans;
using scag::transport::smpp::btTransceiver;

}
}
}

#endif
