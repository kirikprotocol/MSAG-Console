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

	/**
	 * Отправка data_sm pdu smscsme.
	 */
	void dataSm(bool sync);

protected:
	virtual Category& getLog();
	
	AckText* getExpectedResponse(DeliveryReceiptMonitor* monitor,
		const string& text, time_t recvTime);
	
	AckText* getExpectedResponse(IntermediateNotificationMonitor* monitor,
		const string& text, time_t recvTime);

	virtual void processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
		SmppHeader* header, time_t recvTime);

	virtual void processIntermediateNotification(
		IntermediateNotificationMonitor* monitor,
		SmppHeader* header, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_SMSC_SME_TEST_CASES */
