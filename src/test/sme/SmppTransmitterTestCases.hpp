#ifndef TEST_SME_SMPP_TRANSMITTER_TEST_CASES
#define TEST_SME_SMPP_TRANSMITTER_TEST_CASES

#include "sme/SmppBase.hpp"
#include "smpp/smpp.h"
#include "test/util/BaseTestCases.hpp"
#include "test/core/SmeRegistry.hpp"
#include "test/core/PduRegistry.hpp"
#include "SmppPduChecker.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sme::SmppSession;
using namespace smsc::smpp; //pdu
using namespace smsc::test::util; //CheckList, BaseTestCases
using namespace smsc::test::core; //SmeRegistry, PduRegistry, ...

class SmppTransmitterTestCases : BaseTestCases
{
public:
	SmppTransmitterTestCases(SmppSession* session, const SmeSystemId& systemId,
		const Address& smeAddr, const SmeRegistry* smeReg,
		RouteChecker* routeChecker, SmppPduChecker* pduChecker, CheckList* chkList);
	
	virtual ~SmppTransmitterTestCases() {}

	/**
	 * �������� submit_sm pdu ������ sme.
	 */
	void submitSm(bool sync, int num);

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
	void sendDeliverySmRespOk(PduDeliverySm& pdu, int num);

	/**
	 * �������� ����������� ��� ������������ deliver_sm_resp � ����� ������.
	 */
	void sendDeliverySmRespErr(PduDeliverySm& pdu, int num);

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
	
protected:
	virtual Category& getLog();

private:
	SmppSession* session;
	const SmeSystemId systemId;
	const Address smeAddr;
	const SmeRegistry* smeReg;
	PduRegistry* pduReg;
	RouteChecker* routeChecker;
	SmppPduChecker* pduChecker;
	CheckList* chkList;

	PduData* getNonReplaceEnrotePdu();
	PduData* getReplaceEnrotePdu();
	PduData* getNonReplaceRescheduledEnrotePdu();
	template <class Message>
	void checkRegisteredDelivery(Message& m);
	PduData* registerSubmitSm(PduSubmitSm* pdu, PduData* replacePduData,
		time_t submitTime);
	void processSubmitSmSync(PduData* pduData, PduSubmitSmResp* respPdu,
		time_t respTime);
	void processSubmitSmAsync(PduData* pduData, PduSubmitSmResp* respPdu);
	PduData* registerReplaceSm(PduReplaceSm* pdu, PduData* replacePduData,
		time_t submitTime);
	void processReplaceSmSync(PduData* pduData, PduReplaceSmResp* respPdu,
		time_t respTime);
	void processReplaceSmAsync(PduData* pduData, PduReplaceSmResp* respPdu);
};

}
}
}

#endif /* TEST_SME_SMPP_TRANSMITTER_TEST_CASES */

