#ifndef TEST_SME_SMPP_PROFILER_TEST_CASES
#define TEST_SME_SMPP_PROFILER_TEST_CASES

#include "SmppProtocolTestCases.hpp"
#include <string>

namespace smsc {
namespace test {
namespace sme {

using std::string;
using std::pair;
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
	 * Обновление настроек уведомления о доствке.
	 */
	void updateReportOptionsCorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * Обновление настроек кодировки.
	 */
	void updateCodePageCorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * Обновление профиля некорректными данными.
	 */
	void updateProfileIncorrect(bool sync, uint8_t dataCoding);

	/**
	 * Ответные сообщение от profiler доставляются правильно.
	 */
	virtual void processSmeAcknowledgement(SmeAckMonitor* monitor,
		PduDeliverySm& pdu, time_t recvTime);

protected:
	virtual Category& getLog();
	void sendUpdateProfilePdu(const string& text, PduData::IntProps* intProps,
		PduData::StrProps* strProps, PduData::ObjProps* objProps, bool sync,
		uint8_t dataCoding);
	bool checkPdu(PduDeliverySm &pdu);
	AckText* getExpectedResponse(SmeAckMonitor* monitor,
		PduDeliverySm &pdu, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_SMPP_PROFILER_TEST_CASES */
