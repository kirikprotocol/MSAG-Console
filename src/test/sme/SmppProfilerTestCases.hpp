#ifndef TEST_SME_SMPP_PROFILER_TEST_CASES
#define TEST_SME_SMPP_PROFILER_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"
#include "SmppTransmitterTestCases.hpp"
#include <string>

namespace smsc {
namespace test {
namespace sme {

using std::string;
using std::pair;
using log4cpp::Category;
using smsc::test::core::PduData;
using smsc::test::core::SmeAckMonitor;
using smsc::test::util::BaseTestCases;
using namespace smsc::smpp; //pdu

/**
 * Тест кейсы для profiler через smpp протокол.
 */
class SmppProfilerTestCases : public BaseTestCases,
	public SmeAcknowledgementHandler
{
public:
	SmppProfilerTestCases(SmppFixture* _fixture)
	: fixture(_fixture), chkList(fixture->chkList) {}

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
	SmppFixture* fixture;
	CheckList* chkList;

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
