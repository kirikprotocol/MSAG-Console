#ifndef TEST_SME_SME_ACK_HANDLER
#define TEST_SME_SME_ACK_HANDLER

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace sme {

using std::vector;
using log4cpp::Category;
using smsc::smpp::PduSubmitSm;
using smsc::smpp::PduDeliverySm;
using smsc::smeman::SmeInfo;
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
	SmeAcknowledgementHandler(SmppFixture* fixture, const string& smeServiceType,
		uint8_t smeProtocolId);
	
	virtual ~SmeAcknowledgementHandler();

	void addSmeAddr(const Address& addr);
	void addSmeAlias(const Address& alias);

	virtual void processPdu(PduDeliverySm& pdu, time_t recvTime);

	/**
	 * ќбработка sme acknowledgement.
	 */
	virtual void processSmeAcknowledgement(SmeAckMonitor* monitor,
		PduDeliverySm& pdu, time_t recvTime) = NULL;

protected:
	SmppFixture* fixture;
	CheckList* chkList;
	vector<const Address*> smeAddr;
	vector<const Address*> smeAlias;
	const string smeServiceType;
	uint8_t smeProtocolId;

	virtual Category& getLog() = NULL;
	vector<int> checkRoute(PduSubmitSm& pdu1, PduDeliverySm& pdu2) const;
	void updateDeliveryReceiptMonitor(SmeAckMonitor* monitor,
		PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_SME_ACK_HANDLER */
