#ifndef TEST_SME_SME_TEST_CASES
#define TEST_SME_SME_TEST_CASES

#include "test/util/Util.hpp"
#include "test/util/BaseTestCases.hpp"
#include "test/core/RouteRegistry.hpp"
#include "test/core/MessageRegistry.hpp"
#include "sme/sme.hpp"
#include "smpp/smpp_structures.h"

namespace smsc {
namespace test {
namespace sme {

using std::string;
using log4cpp::Category;
using smsc::sme::BaseSme;
using smsc::sms::SMS;
using smsc::sms::Address;
using smsc::smpp::SmppHeader;
using smsc::test::util::TCResult;
using smsc::test::core::RouteRegistry;
using smsc::test::core::MessageRegistry;

//implemented
const char* const TC_BIND_REGISTERED_SME = "bindRegisteredSme";
const char* const TC_BIND_NON_REGISTERED_SME = "bindNonRegisteredSme";
const char* const TC_PROCESS_SMS = "processSms";
const char* const TC_PROCESS_INTERMEDIATE_NOTIFICATION_SMS =
	"processIntermediateNotificationSms";
const char* const TC_PROCESS_DELIVERY_RECEIPT_SMS = "processDeliveryReceiptSms";
const char* const TC_PROCESS_NORMAL_SMS = "processNormalSms";
const char* const TC_UNBIND_BOUNDED = "unbindBounded";
const char* const TC_UNBIND_NON_BOUNDED = "unbindNonBounded";

//esm_class parameter for SMSC -> ESME PDUs (SMPP v3.4, 5.2.12):
//ESME -> SMSC PDUs:
//xxxxxx00 - Default SMSC Mode (e.g. Store and Forward)
//xxxxxx01 - Datagram mode
//xxxxxx10 - Forward (i.e. Transaction) mode
//xxxxxx11 - Store and Forward mode
static const uint8_t ESM_CLASS_DEFAULT_SMSC_MODE = 0x0;
static const uint8_t ESM_CLASS_DATAGRAM_MODE = 0x1;
static const uint8_t ESM_CLASS_FORWARD_MODE = 0x2;
static const uint8_t ESM_CLASS_STORE_AND_FORWARD_MODE - 0x3;
//SMSC -> ESME PDUs:
//xx0000xx - Default message Type (i.e. normal message)
//xx0001xx - Short Message contains SMSC Delivery Receipt
//xx1000xx - Short Message contains Intermediate Delivery Notification
//остальные типы: SME Delivery Acknowledgement, SME Manual/User Acknowledgment,
//Conversation Abort (Korean CDMA) игнорирую
static const uint8_t ESM_CLASS_MESSAGE_TYPE_BITS = 0x74;
static const uint8_t ESM_CLASS_NORMAL_MESSAGE = 0x0;
static const uint8_t ESM_CLASS_DELIVERY_RECEIPT = 0x4;
static const uint8_t ESM_CLASS_INTERMEDIATE_NOTIFICATION = 0x20;

//registered_delivery (SMPP v3.4, 5.2.17):
//SMSC Delivery Receipt:
//xxxxxx00 - No SMSC Delivery Receipt requested (default)
//xxxxxx01 - SMSC Delivery Receipt requested where final delivery outcome
//           is delivery success or failure
//xxxxxx10 - SMSC Delivery Receipt requested where the final delivery
//           outcome is delivery failure
static const uint8_t SMSC_DELIVERY_RECEIPT_BITS = 0x3;
static const uint8_t NO_SMSC_DELIVERY_RECEIPT = 0x0;
static const uint8_t FINAL_SMSC_DELIVERY_RECEIPT = 0x1;
static const uint8_t FAILURE_SMSC_DELIVERY_RECEIPT = 0x2;
//SME originated Acknowledgement - игнорируем
//Intermediate Notification:
//xxx0xxxx - No Intermediate notification requested
//xxx1xxxx - Intermediate notification requested
static const uint8_t INTERMEDIATE_NOTIFICATION_REQUESTED = 0x10;

/**
 * Абстрактный класс для обработки результатов тест кейсов для
 * асинхронных pdu.
 */
struct ResultHandler
{
	virtual process(Result* res) = NULL;
};

/**
 * Этот класс содержит все test cases необходимые для тестирования sme.
 * 
 * @author bryz
 */
class SmeTestCases : BaseTestCases, SmppBaseReceiver
{
public:
	SmeTestCases(const SmeConfig& config, const Address& addr,
		const EService type, const SmeRegistry* smeReg,
		const RouteRegistry* routeReg, ResultHandler* handler); //throws Exception
	
	virtual ~SmeTestCases();

	/**
	 * Bind для sme зарегистрированной в smsc.
	 */
	TCResult* bindRegisteredSme(int num);

	/**
	 * Bind для sme незарегистрированной в smsc.
	 */
	TCResult* bindNonRegisteredSme(int num);

	/**
	 * Синхронная отправка submit_sm pdu другим sme.
	 */
	TCResult* submitSmSync(int num);

	/**
	 * Асинхронная отправка submit_sm pdu другим sme.
	 */
	TCResult* submitSmAsync(int num);
	
	/*
	virtual PduSubmitSmResp* submit(PduSubmitSm& pdu)=0;
	virtual SmppHeader* sendPdu(SmppHeader& pdu)=0;
	virtual void sendGenericNack(PduGenericNack& pdu)=0;
	virtual void sendDeliverySmResp(PduDeliverySmResp& pdu)=0;
	virtual void sendDataSmResp(PduDataSmResp& pdu)=0;
	virtual PduMultiSmResp* submitm(PduMultiSm& pdu)=0;
	virtual PduDataSmResp* data(PduDataSm& pdu)=0;
	virtual PduQuerySmResp* query(PduQuerySm& pdu)=0;
	virtual PduCancelSmResp* cancel(PduCancelSm& pdu)=0;
	virtual PduReplaceSmResp* replace(PduReplaceSm& pdu)=0;
	*/

	/**
	 * Получение асинхронного submit_sm респонса.
	 */
	virtual void processSubmitSmResp(const PduSubmitSmResp &pdu);

	/**
	 * Получение асинхронного deliver_sm pdu.
	 */
	virtual void processDeliverySm(const PduDeliverySm &pdu);
	
	/*
	virtual void processGenericNack(const PduGenericNack &pdu);
	virtual void processDataSm(const PduDataSm &pdu);
	virtual void processMultiResp(const PduMultiSmResp &pdu);
	virtual void processReplaceSmResp(const PduReplaceSmResp &pdu);
	virtual void processDataSmResp(const PduDataSmResp &pdu);
	virtual void processQuerySmResp(const PduQuerySmResp &pdu);
	virtual void processCancelSmResp(const PduCancelSmResp &pdu);
	virtual void processAlertNotificatin(const PduAlertNotification &pdu);
	virtual void handleError(int errorCode);
	*/

	/**
	 * Проверка неполученых подтверждений доставки, нотификаций и sms от других sme.
	 */
	TCResult* processInvalidSms(MessageRegistry& msgReg);
	
	/**
	 * Unbind для sme соединенной с smsc.
	 */
	TCResult* unbindBounded();

	/**
	 * Unbind для sme несоединенной с smsc.
	 */
	TCResult* unbindNonBounded();

protected:
	virtual Category& getLog();

private:
	SmppSession* session;
	const Address origAddr;
    const EService serviceType;
	const SmeRegistry* smeReg;
	const RouteRegistry* routeReg;
	PduRegistry* pduReg;
	ResultHandler* resultHandler;

	uint32_t bindType; //BIND_RECIEVER, BIND_TRANSMITTER, BIND_TRANCIEVER
	const string systemId;
	const string serviceType;

	bool setupRandomDestAddr(PduAddress* destAddr, int num);
	void setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu, int num);
	const PduSubmitSmResp* submitRegisterPdu(SmppTransmitter* transmitter,
		PduSubmitSm* pdu, bool reachableDestAddr);





	void processNormalSms(const SMS& sms, MessageRegistry& msgReg,
		TCResult* res);

	void processDeliveryReceiptSms(const SMS& sms, MessageRegistry& msgReg,
		TCResult* res);

	void processIntermediateNotificationSms(const SMS& sms,
		MessageRegistry& msgReg, TCResult* res);
};

}
}
}

#endif /* TEST_SME_SME_TEST_CASES */

