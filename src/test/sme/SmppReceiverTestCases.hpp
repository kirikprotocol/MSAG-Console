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
#include "SmppPduChecker.hpp"

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
const char* const TC_SEND_DELIVERY_SM_RESP = "sendDeliverySmResp";

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
class SmppReceiverTestCases : BaseTestCases, public SmppBaseReceiver
{
public:
	SmppReceiverTestCases(const SmeSystemId& systemId, const Address& smeAddr,
		const SmeRegistry* smeReg, const AliasRegistry* aliasReg,
		const RouteRegistry* routeReg, ResultHandler* handler,
		RouteChecker* routeChecker, SmppPduChecker* pduChecker);

	virtual ~SmppReceiverTestCases() {}

	void setSession(SmppSession* sess) { session = sess; }
	
	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp.
	 */
	TCResult* sendDeliverySmResp(PduDeliverySm& pdu, bool sendResp,
		bool& sync, bool& accepted, int num);

	/**
	 * ��������� submit_sm_resp pdu ��� ������������ submit_sm ��������.
	 */
	virtual void processSubmitSmResp(PduSubmitSmResp &pdu);

	/**
	 * ��������� ������������ deliver_sm pdu.
	 */
	virtual void processDeliverySm(PduDeliverySm &pdu);
	
	/**
	 * ��������� ��������� ������������ �� ������ sme �������.
	 */
	TCResult* processNormalSms(PduDeliverySm &pdu, time_t recvTime, bool accepted);

	/**
	 * ������������� �������� (delivery receipts) �������� ���������.
	 */
	TCResult* processDeliveryReceipt(PduDeliverySm &pdu,
		time_t recvTime, bool accepted);

	/**
	 * ������������� ����������� (intermediate notifications) �������� ���������.
	 */
	TCResult* processIntermediateNotification(PduDeliverySm &pdu,
		time_t recvTime, bool accepted);
	
	//not implemented
	virtual void processGenericNack(PduGenericNack &pdu);
	virtual void processDataSm(PduDataSm &pdu);
	virtual void processMultiResp(PduMultiSmResp &pdu);
	virtual void processReplaceSmResp(PduReplaceSmResp &pdu);
	virtual void processDataSmResp(PduDataSmResp &pdu);
	virtual void processQuerySmResp(PduQuerySmResp &pdu);
	virtual void processCancelSmResp(PduCancelSmResp &pdu);
	virtual void processAlertNotification(PduAlertNotification &pdu);

	/**
	 * ���������� ���������� ������ � smpp receiver.
	 */
	virtual void handleError(int errorCode);

protected:
	virtual Category& getLog();

private:
	SmppSession* session;
	const SmeSystemId systemId;
	const Address smeAddr;
	const SmeRegistry* smeReg;
	PduRegistry* pduReg;
	const AliasRegistry* aliasReg;
	const RouteRegistry* routeReg;
	ResultHandler* resultHandler;
	RouteChecker* routeChecker;
	SmppPduChecker* pduChecker;
};

}
}
}

#endif /* TEST_SME_SMPP_RECEIVER_TEST_CASES */
