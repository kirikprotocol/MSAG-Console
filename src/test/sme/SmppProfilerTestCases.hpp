#ifndef TEST_SME_SMPP_PROFILER_TEST_CASES
#define TEST_SME_SMPP_PROFILER_TEST_CASES

#include "SmppProtocolTestCases.hpp"
#include <string>

namespace smsc {
namespace test {
namespace sme {

using std::string;
using log4cpp::Category;
using smsc::sme::SmeConfig;
using smsc::test::core::PduData;
using namespace smsc::smpp; //pdu

/**
 * Тест кейсы для profiler через smpp протокол.
 */
class SmppProfilerTestCases : public SmppProtocolTestCases,
	public SmeAcknowledgementHandler
{
public:
	SmppProfilerTestCases(const SmeConfig& config, SmppFixture* fixture)
		: SmppProtocolTestCases(config, fixture)
	{
		fixture->ackHandler = this;
	}

	virtual ~SmppProfilerTestCases() {}

	/**
	 * Обновление профиля корректными данными.
	 */
	void updateProfileCorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * Обновление профиля некорректными данными.
	 */
	void updateProfileIncorrect(bool sync, uint8_t dataCoding);

	/**
	 * Ответные сообщение от profiler доставляются правильно.
	 */
	virtual void processSmeAcknowledgement(SmeAckMonitor* monitor,
		PduDeliverySm &pdu);

protected:
	virtual Category& getLog();
	void sendUpdateProfilePdu(PduSubmitSm* pdu, const string& text,
		bool sync, uint8_t dataCoding, PduData::IntProps& intProps);
	bool checkPdu(PduDeliverySm &pdu);
};

}
}
}

#endif /* TEST_SME_SMPP_PROFILER_TEST_CASES */
