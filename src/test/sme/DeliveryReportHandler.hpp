#ifndef TEST_SME_DELIVERY_REPORT_HANDLER
#define TEST_SME_DELIVERY_REPORT_HANDLER

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"

namespace smsc {
namespace test {
namespace sme {

using std::vector;
using log4cpp::Category;
using smsc::sms::Address;
using smsc::smpp::PduSubmitSm;
using smsc::smpp::PduDeliverySm;
using smsc::smeman::SmeSystemId;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::test::core::DeliveryReceiptMonitor;
using smsc::test::core::IntermediateNotificationMonitor;

/**
 * ѕодтверждени€ доставки (delivery receipts) работают правильно.
 * @author bryz
 */
class DeliveryReportHandler : public BaseTestCases, public PduHandler
{
public:
	DeliveryReportHandler(SmppFixture* fixture, const Address& smeAddr,
		const Address& smeAlias, const SmeSystemId& smeServiceType,
		uint8_t smeProtocolId);
	
	virtual ~DeliveryReportHandler() {}
	
	virtual void processPdu(SmppHeader* header, time_t recvTime);

	virtual void processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
		SmppHeader* header, time_t recvTime) = NULL;

	virtual void processIntermediateNotification(
		IntermediateNotificationMonitor* monitor,
		SmppHeader* header, time_t recvTime) = NULL;

protected:
	SmppFixture* fixture;
	CheckList* chkList;
	const Address smeAddr;
	const Address smeAlias;
	const SmeSystemId smeServiceType;
	const uint8_t smeProtocolId;

	virtual Category& getLog() = NULL;
	vector<int> checkRoute(SmppHeader* header1, SmppHeader* header2) const;
};

}
}
}

#endif /* TEST_SME_DELIVERY_REPORT_HANDLER */

