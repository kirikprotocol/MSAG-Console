#ifndef TEST_SME_SMPP_RECEIVER_TEST_CASES
#define TEST_SME_SMPP_RECEIVER_TEST_CASES

#include "sme/SmppBase.hpp"
#include "smpp/smpp_structures.h"
#include "test/util/BaseTestCases.hpp"
#include "test/core/SmeRegistry.hpp"
#include "test/core/PduRegistry.hpp"
#include "test/core/AliasRegistry.hpp"
#include "test/core/RouteRegistry.hpp"
#include "test/core/SmeUtil.hpp"
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

/**
 * ����������� ����� ��� ��������� ����������� ���� ������ ���
 * ����������� pdu.
 */
struct ResultHandler
{
	virtual void process(TCResult* res) = NULL;
};

/**
 * ���� ����� ��� ��������� ����������� ����������� pdu.
 */
class SmppReceiverTestCases : BaseTestCases, SmppBaseReceiver
{
public:
	SmppReceiverTestCases(SmppSession* session, const SmeSystemId& systemId,
		const Address& smeAddr, const SmeRegistry* smeReg,
		const AliasRegistry* aliasReg, const RouteRegistry* routeReg,
		ResultHandler* handler, RouteChecker* routeChecker,
		SmppResponsePduChecker* responseChecker);

	virtual ~SmppReceiverTestCases() {}
	
	/**
	 * ��������� submit_sm_resp pdu ��� ������������ submit_sm ��������.
	 */
	virtual void processSubmitSmResp(PduSubmitSmResp &pdu);

	/**
	 * ��������� ������������ deliver_sm pdu.
	 */
	virtual void processDeliverySm(PduDeliverySm &pdu);
	
	//not implemented
	virtual void processGenericNack(const PduGenericNack &pdu);
	virtual void processDataSm(const PduDataSm &pdu);
	virtual void processMultiResp(const PduMultiSmResp &pdu);
	virtual void processReplaceSmResp(const PduReplaceSmResp &pdu);
	virtual void processDataSmResp(const PduDataSmResp &pdu);
	virtual void processQuerySmResp(const PduQuerySmResp &pdu);
	virtual void processCancelSmResp(const PduCancelSmResp &pdu);
	virtual void processAlertNotificatin(const PduAlertNotification &pdu);
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

	TCResult* processNormalSms(PduDeliverySm &pdu);
	TCResult* processDeliveryReceipt(PduDeliverySm &pdu);
	TCResult* processIntermediateNotification(PduDeliverySm &pdu);
};

}
}
}

#endif /* TEST_SME_SMPP_RECEIVER_TEST_CASES */
