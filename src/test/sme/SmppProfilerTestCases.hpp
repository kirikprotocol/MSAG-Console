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
	SmppProfilerTestCases(SmppFixture* fixture);

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
	 * ���������� �������� ������.
	 */
	void updateLocaleCorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * ���������� hide �����.
	 */
	void updateHideOptionsCorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * ���������� ������� ������������� �������.
	 */
	void updateProfileIncorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * �������� ��������� �� profiler ������������ ���������.
	 */
	virtual void processSmeAcknowledgement(SmeAckMonitor* monitor,
		SmppHeader* header, time_t recvTime);

protected:
	virtual Category& getLog();
	void updateProfile(Profile& profile, PduData::IntProps* intProps,
		PduData::StrProps* strProps, PduData::ObjProps* objProps);
	void sendUpdateProfilePdu(const string& text, PduData::IntProps* intProps,
		PduData::StrProps* strProps, PduData::ObjProps* objProps, bool sync,
		uint8_t dataCoding);
	AckText* getExpectedResponse(SmeAckMonitor* monitor,
		SmppHeader* header, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_SMPP_PROFILER_TEST_CASES */
