#ifndef TEST_SME_SMPP_RECEIVER_TEST_CASES
#define TEST_SME_SMPP_RECEIVER_TEST_CASES

#include "sme/SmppBase.hpp"
#include "smpp/smpp.h"
#include "SmppFixture.hpp"
#include "NormalSmsHandler.hpp"
#include "test/util/BaseTestCases.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sme::SmppBaseReceiver;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using namespace smsc::smpp; //pdu

/**
 * ���� ����� ��� ��������� ����������� ����������� pdu.
 */
class SmppReceiverTestCases : BaseTestCases, public SmppBaseReceiver
{
public:
	SmppReceiverTestCases(SmppFixture* _fixture)
	: fixture(_fixture), chkList(_fixture->chkList), defaultHandler(fixture)
	{ fixture->receiver = this; }

	virtual ~SmppReceiverTestCases() {}

	/**
	 * ��������� submit_sm_resp pdu.
	 */
	virtual void processSubmitSmResp(PduSubmitSmResp& pdu);

	/**
	 * ��������� data_sm_resp pdu.
	 */
	virtual void processDataSmResp(PduDataSmResp& pdu);

	/**
	 * ��������� replace_sm_resp pdu.
	 */
	virtual void processReplaceSmResp(PduReplaceSmResp& pdu);

	/**
	 * ��������� deliver_sm pdu.
	 */
	virtual void processDeliverySm(PduDeliverySm& pdu);

	/**
	 * ��������� data_sm pdu.
	 */
	virtual void processDataSm(PduDataSm& pdu);

	/**
	 * ��������� ������������ query_sm pdu.
	 */
	virtual void processQuerySmResp(PduQuerySmResp& pdu);
	
	/**
	 * ��������� ������������ cancel_sm pdu.
	 */
	virtual void processCancelSmResp(PduCancelSmResp& pdu);
	
	//not implemented
	virtual void processGenericNack(PduGenericNack& pdu);
	virtual void processMultiResp(PduMultiSmResp& pdu);
	virtual void processAlertNotification(PduAlertNotification& pdu);

	/**
	 * ���������� ���������� ������ � smpp receiver.
	 */
	virtual void handleError(int errorCode);

protected:
	SmppFixture* fixture;
	CheckList* chkList;
	NormalSmsHandler defaultHandler;

	virtual Category& getLog();
};

}
}
}

#endif /* TEST_SME_SMPP_RECEIVER_TEST_CASES */
