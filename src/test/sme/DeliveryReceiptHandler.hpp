#ifndef TEST_SME_DELIVERY_RECEIPT_HANDLER
#define TEST_SME_DELIVERY_RECEIPT_HANDLER

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::smpp::PduSubmitSm;
using smsc::smpp::PduDeliverySm;
using smsc::smeman::SmeInfo;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::test::core::DeliveryReceiptMonitor;

/**
 * ѕодтверждени€ доставки (delivery receipts) работают правильно.
 * @author bryz
 */
class DeliveryReceiptHandler : public BaseTestCases, public PduHandler
{
public:
	DeliveryReceiptHandler(SmppFixture* fixture);
	
	virtual ~DeliveryReceiptHandler() {}
	
	virtual void processPdu(PduDeliverySm& pdu, time_t recvTime);

	virtual void processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
		PduDeliverySm &pdu, time_t recvTime) = NULL;

protected:
	SmppFixture* fixture;
	CheckList* chkList;
	const SmeInfo* sme;

	virtual Category& getLog() = NULL;
	vector<int> checkRoute(PduSubmitSm& pdu1, PduDeliverySm& pdu2) const;
};

}
}
}

#endif /* TEST_SME_DELIVERY_RECEIPT_HANDLER */
