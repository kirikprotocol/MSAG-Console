#ifndef TEST_SME_SMPP_PROFILER_TEST_CASES
#define TEST_SME_SMPP_PROFILER_TEST_CASES

#include "SmeAcknowledgementHandler.hpp"
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
 * ���� ����� ��� profiler ����� smpp ��������.
 */
class SmppProfilerTestCases : public SmeAcknowledgementHandler
{
public:
	SmppProfilerTestCases(SmppFixture* fixture)
	: SmeAcknowledgementHandler(fixture) {}

	virtual ~SmppProfilerTestCases() {}

	/**
	 * ���������� �������� ����������� � �������.
	 */
	void updateReportOptionsCorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * ���������� �������� ���������.
	 */
	void updateCodePageCorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * ���������� ������� ������������� �������.
	 */
	void updateProfileIncorrect(bool sync, uint8_t dataCoding);

	/**
	 * �������� ��������� �� profiler ������������ ���������.
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
