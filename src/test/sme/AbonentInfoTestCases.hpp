#ifndef TEST_SME_ABONENT_INFO_TEST_CASES
#define TEST_SME_ABONENT_INFO_TEST_CASES

#include "SmeAcknowledgementHandler.hpp"
#include "SmppTransmitterTestCases.hpp"

namespace smsc {
namespace test {
namespace sme {

struct AbonentData : public PduDataObject
{
	const string input;
	const Address addr;
	SmeType status;
	const Profile profile;
	bool validProfile;
	AbonentData(const string& _input, const Address& _addr, SmeType _status,
		const Profile& _profile, bool _validProfile)
	: input(_input), addr(_addr), status(_status), profile(_profile),
		validProfile(_validProfile) {}
};

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
	
	AbonentData* getAbonentData(const string& input);
	void sendAbonentInfoPdu(const string& input, bool sync,
		uint8_t dataCoding, bool correct);
	AckText* getExpectedResponse(SmeAckMonitor* monitor,
		const string& text, time_t recvTime);
	void processSmeAcknowledgement(SmeAckMonitor* monitor,
		SmppHeader* header, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_ABONENT_INFO_TEST_CASES */
