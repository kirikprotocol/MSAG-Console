#ifndef TEST_SME_SMSC_SME_TEST_CASES
#define TEST_SME_SMSC_SME_TEST_CASES

#include "DeliveryReceiptHandler.hpp"

namespace smsc {
namespace test {
namespace sme {

/**
 * Этот класс содержит все test cases необходимые для тестирования smsc sme.
 * @author bryz
 */
class SmscSmeTestCases : public DeliveryReceiptHandler
{
public:
	SmscSmeTestCases(SmppFixture* fixture)
	: DeliveryReceiptHandler(fixture) {}
	
	virtual ~SmscSmeTestCases() {}

protected:
	virtual Category& getLog();
	
	AckText* getExpectedResponse(DeliveryReceiptMonitor* monitor,
		PduSubmitSm* origPdu, const string& text, time_t recvTime);

	void processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
		PduDeliverySm& pdu, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_SMSC_SME_TEST_CASES */
