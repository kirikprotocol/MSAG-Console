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
using smsc::test::util::CheckList;
using namespace smsc::smpp; //pdu

/**
 * Тест кейсы для обработки результатов асинхронных pdu.
 */
class SmppReceiverTestCases : BaseTestCases, public SmppBaseReceiver
{
public:
	SmppReceiverTestCases(SmppFixture* _fixture)
		: fixture(_fixture), chkList(fixture->chkList) {}

	virtual ~SmppReceiverTestCases() {}

	/**
	 * Обработка submit_sm_resp pdu для асинхронного submit_sm реквеста.
	 */
	virtual void processSubmitSmResp(PduSubmitSmResp &pdu);

	/**
	 * Обработка replace_sm_resp pdu для асинхронного replace_sm реквеста.
	 */
	virtual void processReplaceSmResp(PduReplaceSmResp &pdu);

	/**
	 * Обработка асинхронного deliver_sm pdu.
	 */
	virtual void processDeliverySm(PduDeliverySm &pdu);
	
	/**
	 * Сообщения правильно доставляются от одного sme другому.
	 */
	void processNormalSms(PduDeliverySm &pdu, time_t recvTime);

	/**
	 * Ответные сообщение от внутренних sme SC доставляются правильно.
	 */
	void processSmeAcknowledgement(PduDeliverySm &pdu, time_t recvTime);

	/**
	 * Подтверждения доставки (delivery receipts) работают правильно.
	 */
	void processDeliveryReceipt(PduDeliverySm &pdu, time_t recvTime);

	/**
	 * Промежуточные нотификации (intermediate notifications) работают правильно.
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
	 * Отсутствие внутренних ошибок в smpp receiver.
	 */
	virtual void handleError(int errorCode);

protected:
	SmppFixture* fixture;
	CheckList* chkList;

	virtual Category& getLog();
	RespPduFlag isAccepted(uint32_t status);
	void compareMsgText(PduSubmitSm& origPdu, PduDeliverySm& pdu);
};

}
}
}

#endif /* TEST_SME_SMPP_RECEIVER_TEST_CASES */
