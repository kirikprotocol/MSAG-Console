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
using smsc::test::util::TestCaseId;

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
	 * �������� ���������� "�������" submit_sm pdu ������ sme.
	 */
	void submitSmCorrectComplex(bool sync, int num);

	/**
	 * �������� ������������ submit_sm pdu ������ sme.
	 */
	void submitSmIncorrect(bool sync, int num);

	/**
	 * ���������� � �������� submit_sm pdu � ������������� ���������� �����.
	 */
	void submitSmAssert(int num);

	/**
	 * �������� ���������� data_sm pdu ������ sme.
	 */
	void dataSmCorrect(bool sync, int num);

	/**
	 * �������� ������������ data_sm pdu ������ sme.
	 */
	void dataSmIncorrect(bool sync, int num);

	/**
	 * ���������� � �������� submit_sm ��� data_sm pdu � ����������� �����������.
	 */
	void correctDirectives(bool sync, const TestCaseId& num);

	/**
	 * ���������� � �������� submit_sm ��� data_sm pdu � ������������� �����������.
	 */
	void incorrectDirectives(bool sync, const TestCaseId& num);

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
	 * ������ ��������� ������������ sms.
	 */
	void cancelSmSingleCorrect(bool sync, int num);

	/**
	 * ������ ������ ������������ sms.
	 */
	void cancelSmGroupCorrect(bool sync, int num);

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
	pair<uint32_t, time_t> sendDeliverySmRespOk(SmppHeader* header,
		bool sync, bool sendDelay);

	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp � ����� ������
	 * � ����������� ��������� ���������.
	 */
	pair<uint32_t, time_t> sendDeliverySmRespRetry(SmppHeader* header,
		bool sync, int num);

	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp � ����� ������
	 * � ����������� ������������ ��������.
	 */
	pair<uint32_t, time_t> sendDeliverySmRespError(SmppHeader* header,
		bool sync, bool sendDelay, int num);

	/**
	 * �������� ����������� ��� ������������ data_sm_resp �� �������� ok.
	 */
	pair<uint32_t, time_t> sendDataSmRespOk(SmppHeader* header,
		bool sync, bool sendDelay);

	/**
	 * �������� ����������� ��� ������������ data_sm_resp � ����� ������
	 * � ����������� ��������� ���������.
	 */
	pair<uint32_t, time_t> sendDataSmRespRetry(SmppHeader* header,
		bool sync, int num);

	/**
	 * �������� ����������� ��� ������������ data_sm_resp � ����� ������
	 * � ����������� ������������ ��������.
	 */
	pair<uint32_t, time_t> sendDataSmRespError(SmppHeader* header,
		bool sync, bool sendDelay, int num);

	void sendInvalidPdu(bool sync, int num);

protected:
	SmppFixture* fixture;
	CheckList* chkList;

	virtual Category& getLog();
	PduData* getNonReplaceEnrotePdu();
	PduData* getReplaceEnrotePdu();
	PduData* getNonReplaceRescheduledEnrotePdu();
	PduData* getFinalPdu();
	PduData* getPduByState(SmppState state);

	bool setDirective(SmppHeader* header, const string& dir, int& offset);
	bool correctAckDirectives(SmppHeader* header, PduData::IntProps& intProps, int num);
	bool correctDefDirectives(SmppHeader* header, PduData::IntProps& intProps, int num);
	bool correctTemplateDirectives(SmppHeader* header, PduData::IntProps& intProps,
		PduData::StrProps& strProps, int num);
	void correctDirectives(SmppHeader* header, PduData::IntProps& intProps,
		PduData::StrProps& strProps, const TestCaseId& num);
	void incorrectDirectives(SmppHeader* header, PduData::IntProps& intProps, int num);

	void replaceSmIncorrect(PduReplaceSm* pdu, bool sync);
	PduData* getCancelSmGroupParams(bool checkServType, Address& srcAddr,
		Address& destAddr, string& servType);
	int getRandomRespDelay();
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_TEST_CASES */
