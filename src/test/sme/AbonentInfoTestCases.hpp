#ifndef TEST_SME_ABONENT_INFO_TEST_CASES
#define TEST_SME_ABONENT_INFO_TEST_CASES

#include "SmeAcknowledgementHandler.hpp"
#include "SmppTransmitterTestCases.hpp"

namespace smsc {
namespace test {
namespace sme {

/**
 * Этот класс содержит все test cases необходимые для тестирования abonent info sme.
 * @author bryz
 */
class AbonentInfoTestCases : public SmeAcknowledgementHandler
{
public:
	AbonentInfoTestCases(SmppFixture* fixture);
	virtual ~AbonentInfoTestCases() {}

	/**
	 * Отправка корректного запроса на abonent info.
	 */
	void queryAbonentInfoCorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка некорректного запроса на abonent info.
	 */
	void queryAbonentInfoIncorrect(bool sync, uint8_t dataCoding, int num);

protected:
	virtual Category& getLog();
	
	AckText* getExpectedResponse(const string& input,
		const Address& smeAddr, time_t submitTime);
	void sendAbonentInfoPdu(const string& input, bool sync, uint8_t dataCoding);
	AckText* getExpectedResponse(SmeAckMonitor* monitor,
		const string& text, time_t recvTime);
	void processSmeAcknowledgement(SmeAckMonitor* monitor,
		PduDeliverySm& pdu, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_ABONENT_INFO_TEST_CASES */
