#ifndef TEST_SME_SMSC_SME_TEST_CASES
#define TEST_SME_SMSC_SME_TEST_CASES

#include "DeliveryReportHandler.hpp"
#include "SmppTransmitterTestCases.hpp"

namespace smsc {
namespace test {
namespace sme {

/**
 * Этот класс содержит все test cases необходимые для тестирования smsc sme.
 * @author bryz
 */
class SmscSmeTestCases : public DeliveryReportHandler
{
public:
	SmscSmeTestCases(SmppFixture* fixture);
	
	virtual ~SmscSmeTestCases() {}

	/**
	 * Отправка submit_sm pdu smscsme.
	 */
	void submitSm(bool sync);

protected:
	virtual Category& getLog();
	
	AckText* getExpectedResponse(DeliveryReceiptMonitor* monitor,
		PduSubmitSm* origPdu, const string& text, time_t recvTime);
	
	AckText* getExpectedResponse(IntermediateNotificationMonitor* monitor,
		PduSubmitSm* origPdu, const string& text, time_t recvTime);

	virtual void processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
		PduDeliverySm& pdu, time_t recvTime);

	virtual void processIntermediateNotification(
		IntermediateNotificationMonitor* monitor,
		PduDeliverySm& pdu, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_SMSC_SME_TEST_CASES */
