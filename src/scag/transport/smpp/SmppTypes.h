#ifndef __SCAG_TRANSPORT_SMPP_SMPPTYPES_H__
#define __SCAG_TRANSPORT_SMPP_SMPPTYPES_H__

#include <string>

namespace scag{
namespace transport{
namespace smpp{

enum SmppEntityType{
  etUnknown,etSmsc,etService
};
inline std::string SmppEntityTypeToString(SmppEntityType smppEntityType)
{
	std::string strSmppEntityType;
	switch(smppEntityType)
	{
	case etSmsc:		strSmppEntityType="etSmsc";		break;
	case etService:		strSmppEntityType="etService";	break;
	default:			strSmppEntityType="etUnknown";
	}
	return strSmppEntityType;
}

enum SmppBindType{
  btNone,btReceiver,btTransmitter,btRecvAndTrans,btTransceiver
};

inline std::string SmppBindTypeToString(SmppBindType smppBindType)
{
	std::string strSmppBindType;
	switch(smppBindType)
	{
	case btNone:		strSmppBindType="";			break;
	case btReceiver:	strSmppBindType="RX";		break;
	case btTransmitter:	strSmppBindType="TX";	break;
	case btRecvAndTrans:strSmppBindType="RTX";	break;
	case btTransceiver:	strSmppBindType="TRX";	break;
	default:			strSmppBindType="Unknown";
	}
	return strSmppBindType;
}

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
using scag::transport::smpp::SmppEntityTypeToString;
using scag::transport::smpp::SmppBindTypeToString;

}
}
}

#endif
