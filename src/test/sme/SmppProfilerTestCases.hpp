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
 * ���� ����� ��� profiler ����� smpp ��������.
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
	 * ���������� ������� ����������� �������.
	 */
	void updateProfileCorrect(bool sync, uint8_t dataCoding, int num);

	/**
	 * ���������� ������� ������������� �������.
	 */
	void updateProfileIncorrect(bool sync, uint8_t dataCoding);

	/**
	 * �������� ��������� �� profiler ������������ ���������.
	 */
	virtual void processSmeAcknowledgement(PduData* pduData, PduDeliverySm &pdu);

protected:
	virtual Category& getLog();
	void sendUpdateProfilePdu(PduSubmitSm* pdu, const string& text,
		bool sync, uint8_t dataCoding, PduData::IntProps& intProps);
};

}
}
}

#endif /* TEST_SME_SMPP_PROFILER_TEST_CASES */
