#ifndef TEST_SME_SMPP_RECEIVER_TEST_CASES
#define TEST_SME_SMPP_RECEIVER_TEST_CASES

#include "sme/SmppBase.hpp"
#include "smpp/smpp.h"
#include "test/util/BaseTestCases.hpp"
#include "test/core/SmeRegistry.hpp"
#include "test/core/PduRegistry.hpp"
#include "test/core/AliasRegistry.hpp"
#include "test/core/RouteRegistry.hpp"
#include "test/core/RouteChecker.hpp"
#include "SmppResponsePduChecker.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sme::SmppBaseReceiver;
using smsc::sme::SmppSession;
using namespace smsc::smpp; //pdu
using namespace smsc::test::util; //TCResult, BaseTestCases
using namespace smsc::test::core; //SmeRegistry, PduRegistry, ...

//implemented
const char* const TC_PROCESS_SUBMIT_SM_RESP = "processSubmitSmResp";
const char* const TC_PROCESS_DELIVERY_SM = "processDeliverySm";
const char* const TC_PROCESS_NORMAL_SMS = "processNormalSms";
const char* const TC_PROCESS_DELIVERY_RECEIPT = "processDeliveryReceipt";
const char* const TC_PROCESS_INTERMEDIATE_NOTIFICATION =
	"processIntermediateNotification";
const char* const TC_HANDLE_ERROR = "handleError";

/**
 * Абстрактный класс для обработки результатов тест кейсов для
 * асинхронных pdu.
 */
struct ResultHandler
{
	virtual void process(TCResult* res) = NULL;
};

/**
 * Тест кейсы для обработки результатов асинхронных pdu.
 */
class SmppReceiverTestCases : BaseTestCases, public SmppBaseReceiver
{
public:
	SmppReceiverTestCases(const SmeSystemId& systemId, const Address& smeAddr,
		const SmeRegistry* smeReg, const AliasRegistry* aliasReg,
		const RouteRegistry* routeReg, ResultHandler* handler,
		RouteChecker* routeChecker, SmppResponsePduChecker* responseChecker);

	virtual ~SmppReceiverTestCases() {}

	void setSession(SmppSession* sess) { session = sess; }
	
	/**
	 * Получение submit_sm_resp pdu для асинхронного submit_sm реквеста.
	 */
	virtual void processSubmitSmResp(PduSubmitSmResp &pdu);

	/**
	 * Получение асинхронного deliver_sm pdu.
	 */
	virtual void processDeliverySm(PduDeliverySm &pdu);
	
	/**
	 * Сообщения правильно доставляются от одного sme другому.
	 */
	TCResult* processNormalSms(PduDeliverySm &pdu);

	/**
	 * Подтверждения доставки (delivery receipts) работают правильно.
	 */
	TCResult* processDeliveryReceipt(PduDeliverySm &pdu);

	/**
	 * Промежуточные нотификации (intermediate notifications) работают правильно.
	 */
	TCResult* processIntermediateNotification(PduDeliverySm &pdu);
	
	//not implemented
	virtual void processGenericNack(PduGenericNack &pdu);
	virtual void processDataSm(PduDataSm &pdu);
	virtual void processMultiResp(PduMultiSmResp &pdu);
	virtual void processReplaceSmResp(PduReplaceSmResp &pdu);
	virtual void processDataSmResp(PduDataSmResp &pdu);
	virtual void processQuerySmResp(PduQuerySmResp &pdu);
	virtual void processCancelSmResp(PduCancelSmResp &pdu);
	virtual void processAlertNotificatin(PduAlertNotification &pdu);

	/**
	 * Отсутствие внутренних ошибок в smpp receiver.
	 */
	virtual void handleError(int errorCode);

protected:
	virtual Category& getLog();

private:
	SmppSession* session;
	const SmeSystemId systemId;
	const Address& smeAddr;
	const SmeRegistry* smeReg;
	PduRegistry* pduReg;
	const AliasRegistry* aliasReg;
	const RouteRegistry* routeReg;
	ResultHandler* resultHandler;
	RouteChecker* routeChecker;
	SmppResponsePduChecker* responseChecker;
};

}
}
}

#endif /* TEST_SME_SMPP_RECEIVER_TEST_CASES */
