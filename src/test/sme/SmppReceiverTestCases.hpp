#ifndef TEST_SME_SMPP_RECEIVER_TEST_CASES
#define TEST_SME_SMPP_RECEIVER_TEST_CASES

#include "sme/SmppBase.hpp"
#include "smpp/smpp.h"
#include "SmppFixture.hpp"
#include "test/util/BaseTestCases.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sme::SmppBaseReceiver;
using smsc::test::util::BaseTestCases;
using smsc::test::core::RespPduFlag;
using smsc::test::core::DeliveryMonitor;
using smsc::test::util::CheckList;
using namespace smsc::smpp; //pdu

/**
 * ���� ����� ��� ��������� ����������� ����������� pdu.
 */
class SmppReceiverTestCases : BaseTestCases, public SmppBaseReceiver
{
public:
	SmppReceiverTestCases(SmppFixture* _fixture)
		: fixture(_fixture), chkList(fixture->chkList) {}

	virtual ~SmppReceiverTestCases() {}

	/**
	 * ��������� submit_sm_resp pdu ��� ������������ submit_sm ��������.
	 */
	virtual void processSubmitSmResp(PduSubmitSmResp &pdu);

	/**
	 * ��������� replace_sm_resp pdu ��� ������������ replace_sm ��������.
	 */
	virtual void processReplaceSmResp(PduReplaceSmResp &pdu);

	/**
	 * ��������� ������������ deliver_sm pdu.
	 */
	virtual void processDeliverySm(PduDeliverySm &pdu);
	
	/**
	 * ��������� ��������� ������������ �� ������ sme �������.
	 */
	void processNormalSms(PduDeliverySm &pdu, time_t recvTime);

	/**
	 * �������� ��������� �� ���������� sme SC ������������ ���������.
	 */
	void processSmeAcknowledgement(PduDeliverySm &pdu, time_t recvTime);

	/**
	 * ������������� �������� (delivery receipts) �������� ���������.
	 */
	void processDeliveryReceipt(PduDeliverySm &pdu, time_t recvTime);

	/**
	 * ������������� ����������� (intermediate notifications) �������� ���������.
	 */
	void processIntermediateNotification(PduDeliverySm &pdu, time_t recvTime);

	//not implemented
	virtual void processGenericNack(PduGenericNack &pdu);
	virtual void processDataSm(PduDataSm &pdu);
	virtual void processMultiResp(PduMultiSmResp &pdu);
	virtual void processDataSmResp(PduDataSmResp &pdu);
	virtual void processQuerySmResp(PduQuerySmResp &pdu);
	virtual void processCancelSmResp(PduCancelSmResp &pdu);
	virtual void processAlertNotification(PduAlertNotification &pdu);

	/**
	 * ���������� ���������� ������ � smpp receiver.
	 */
	virtual void handleError(int errorCode);

protected:
	SmppFixture* fixture;
	CheckList* chkList;

	virtual Category& getLog();
	RespPduFlag isAccepted(uint32_t status);
	void compareMsgText(PduSubmitSm& origPdu, PduDeliverySm& pdu);
	void updateDeliveryReceiptMonitor(DeliveryMonitor* monitor,
		PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime);
};

}
}
}

#endif /* TEST_SME_SMPP_RECEIVER_TEST_CASES */
