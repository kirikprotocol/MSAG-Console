#ifndef TEST_SME_SME_ACK_HANDLER
#define TEST_SME_SME_ACK_HANDLER

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::smpp::PduDeliverySm;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::test::core::PduRegistry;
using smsc::test::core::SmeAckMonitor;

/**
 * ќтветные сообщение от sme доставл€ютс€ правильно.
 * @author bryz
 */
class SmeAcknowledgementHandler : public BaseTestCases, public PduHandler
{
public:
	SmeAcknowledgementHandler(SmppFixture* _fixture)
	: fixture(_fixture), chkList(_fixture->chkList) {}
	
	virtual ~SmeAcknowledgementHandler() {}

	virtual void processPdu(PduDeliverySm& pdu, time_t recvTime);

	/**
	 * ќбработка sme acknowledgement.
	 */
	virtual void processSmeAcknowledgement(SmeAckMonitor* monitor,
		PduDeliverySm& pdu, time_t recvTime) = NULL;

protected:
	SmppFixture* fixture;
	CheckList* chkList;
	
	virtual Category& getLog() = NULL;
	void updateDeliveryReceiptMonitor(SmeAckMonitor* monitor,
		PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_SME_ACK_HANDLER */
