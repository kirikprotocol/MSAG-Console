#ifndef TEST_SME_SMPP_PROTOCOL_TEST_CASES
#define TEST_SME_SMPP_PROTOCOL_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"
#include "SmppTransmitterTestCases.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::smeman::SmeInfo;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;

/**
 * ���� ����� �������� ��� test cases ����������� ��� ������������ sme.
 * @author bryz
 */
class SmppProtocolTestCases : public BaseTestCases
{
public:
	SmppProtocolTestCases(SmppFixture* fixture);
	virtual ~SmppProtocolTestCases() {}

	/**
	 * �������� ���������� submit_sm pdu ������ sme.
	 */
	void submitSmCorrect(bool sync, int num);

	/**
	 * �������� ������������ submit_sm pdu ������ sme.
	 */
	void submitSmIncorrect(bool sync, int num);

	/**
	 * ���������� � �������� submit_sm pdu � ������������� ���������� �����.
	 */
	void submitSmAssert(int num);

	/**
	 * ���������� ��������� ����� ������������ submit_sm pdu.
	 */
	void replaceSmCorrect(bool sync, int num);

	/**
	 * ������������ ��������� ����� ������������ submit_sm pdu.
	 */
	void replaceSmIncorrect(bool sync, int num);

	/**
	 * ���������� � �������� replace_sm pdu � ������������� ���������� �����.
	 */
	void replaceSmAssert(int num);

	/**
	 * ������ ������� ������������� sms.
	 */
	void querySmCorrect(bool sync, int num);

	/**
	 * ������ ������� ��������������� sms.
	 */
	void querySmIncorrect(bool sync, int num);

	/**
	 * ������ ������������� sms.
	 */
	void cancelSmCorrect(bool sync, int num);

	/**
	 * ������ ��������������� sms.
	 */
	void cancelSmIncorrect(bool sync, int num);

	/**
	 * �������� ������.
	 */
	void genericNack(bool sync);

	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp �� �������� ok.
	 */
	pair<uint32_t, time_t> sendDeliverySmRespOk(PduDeliverySm& pdu,
		bool sync, bool sendDelay);

	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp � ����� ������
	 * � ����������� ��������� ���������.
	 */
	pair<uint32_t, time_t> sendDeliverySmRespRetry(PduDeliverySm& pdu,
		bool sync, int num);

	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp � ����� ������
	 * � ����������� ������������ ��������.
	 */
	pair<uint32_t, time_t> sendDeliverySmRespError(PduDeliverySm& pdu,
		bool sync, bool sendDelay, int num);

	void sendInvalidPdu(bool sync, int num);

protected:
	SmppFixture* fixture;
	CheckList* chkList;

	virtual Category& getLog();
	PduData* getNonReplaceEnrotePdu(bool deliveryReports);
	PduData* getReplaceEnrotePdu(bool deliveryReports);
	PduData* getNonReplaceRescheduledEnrotePdu(bool deliveryReports);
	PduData* getFinalPdu(bool deliveryReports);
	PduData* getPduByState(State state);
	int getRandomRespDelay();
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_TEST_CASES */
