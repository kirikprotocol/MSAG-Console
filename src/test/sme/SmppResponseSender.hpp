#ifndef TEST_SME_SMPP_RESPONSE_SENDER
#define TEST_SME_SMPP_RESPONSE_SENDER

#include "smpp/smpp.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::smpp::PduDeliverySm;

struct SmppResponseSender
{
	virtual uint32_t sendDeliverySmResp(PduDeliverySm& pdu) = NULL;
};

}
}
}

#endif /* TEST_SME_SMPP_RESPONSE_SENDER */
