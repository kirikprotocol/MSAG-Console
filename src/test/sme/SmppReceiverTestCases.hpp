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
#include "SmppResponseSender.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sme::SmppBaseReceiver;
using smsc::sme::SmppSession;
using smsc::test::util::CheckList;
using namespace smsc::smpp; //pdu
using namespace smsc::test::util; //BaseTestCases
using namespace smsc::test::core; //SmeRegistry, PduRegistry, ...

/**
 * ���� ����� ��� ��������� ����������� ����������� pdu.
 */
class SmppReceiverTestCases : BaseTestCases, public SmppBaseReceiver
{
public:
	SmppReceiverTestCases(const SmeSystemId& systemId, const Address& smeAddr,
		SmppResponseSender* respSender, const SmeRegistry* smeReg,
		const AliasRegistry* aliasReg, const RouteRegistry* routeReg,
		RouteChecker* routeChecker, SmppPduChecker* pduChecker, CheckList* chkList);

	virtual ~SmppReceiverTestCases() {}

	void setSession(SmppSession* sess) { session = sess; }
	
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
	virtual Category& getLog();

private:
	SmppSession* session;
	const SmeSystemId systemId;
	const Address smeAddr;
	SmppResponseSender* respSender;
	const SmeRegistry* smeReg;
	PduRegistry* pduReg;
	const AliasRegistry* aliasReg;
	const RouteRegistry* routeReg;
	RouteChecker* routeChecker;
	SmppPduChecker* pduChecker;
	CheckList* chkList;
};

}
}
}

#endif /* TEST_SME_SMPP_RECEIVER_TEST_CASES */
