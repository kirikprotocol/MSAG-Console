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
	 * ��������� ����� ������������ submit_sm pdu.
	 */
	void replaceSm(bool sync, int num);

	/**
	 * ���������� � �������� replace_sm pdu � ������������� ���������� �����.
	 */
	void replaceSmAssert(int num);
	
	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp �� �������� ok.
	 */
	uint32_t sendDeliverySmRespOk(PduDeliverySm& pdu, bool sync);

	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp � ����� ������
	 * � ����������� ��������� ���������.
	 */
	uint32_t sendDeliverySmRespRetry(PduDeliverySm& pdu, bool sync, int num);

	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp � ����� ������
	 * � ����������� ������������ ��������.
	 */
	uint32_t sendDeliverySmRespError(PduDeliverySm& pdu, bool sync, int num);

protected:
	SmppFixture* fixture;
	CheckList* chkList;
	const SmeInfo* sme;

	virtual Category& getLog();
	PduData* getNonReplaceEnrotePdu();
	PduData* getReplaceEnrotePdu();
	PduData* getNonReplaceRescheduledEnrotePdu();
	PduData* getFinalPdu();
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_TEST_CASES */
