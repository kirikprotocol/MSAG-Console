#ifndef TEST_SME_SMPP_TRANSMITTER_TEST_CASES
#define TEST_SME_SMPP_TRANSMITTER_TEST_CASES

#include "sme/SmppBase.hpp"
#include "smpp/smpp.h"
#include "test/util/BaseTestCases.hpp"
#include "test/core/SmeRegistry.hpp"
#include "test/core/PduRegistry.hpp"
#include "test/core/ProfileRegistry.hpp"
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
		ProfileRegistry* profileReg, RouteChecker* routeChecker,
		SmppPduChecker* pduChecker, CheckList* chkList);
	
	virtual ~SmppTransmitterTestCases() {}

	/**
	 * Отправка корректной submit_sm pdu другим sme.
	 */
	void submitSmCorrect(bool sync, int num);

	/**
	 * Отправка некорректной submit_sm pdu другим sme.
	 */
	void submitSmIncorrect(bool sync, int num);

	/**
	 * Заполнение и отправка submit_sm pdu с недопустимыми значениями полей.
	 */
	void submitSmAssert(int num);

	/**
	 * Замещение ранее отправленной submit_sm pdu.
	 */
	void replaceSm(bool sync, int num);

	/**
	 * Заполнение и отправка replace_sm pdu с недопустимыми значениями полей.
	 */
	void replaceSmAssert(int num);
	
	/**
	 * Отправка синхронного или асинхронного deliver_sm_resp со статусом ok.
	 */
	uint32_t sendDeliverySmRespOk(PduDeliverySm& pdu, bool sync);

	/**
	 * Отправка синхронного или асинхронного deliver_sm_resp с кодом ошибки
	 * и последующей повторной доставкой.
	 */
	uint32_t sendDeliverySmRespRetry(PduDeliverySm& pdu, bool sync, int num);

	/**
	 * Отправка синхронного или асинхронного deliver_sm_resp с кодом ошибки
	 * и последующем прекращением доставки.
	 */
	uint32_t sendDeliverySmRespError(PduDeliverySm& pdu, bool sync, int num);

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
	ProfileRegistry* profileReg;
	RouteChecker* routeChecker;
	SmppPduChecker* pduChecker;
	CheckList* chkList;

	PduData* getNonReplaceEnrotePdu();
	PduData* getReplaceEnrotePdu();
	PduData* getNonReplaceRescheduledEnrotePdu();
	template <class Message>
	void checkRegisteredDelivery(Message& m);
	void setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu);
	PduData* registerSubmitSm(PduSubmitSm* pdu, PduData* replacePduData,
		time_t submitTime);
	void processSubmitSmSync(PduData* pduData, PduSubmitSmResp* respPdu,
		time_t respTime);
	void processSubmitSmAsync(PduData* pduData, PduSubmitSmResp* respPdu);
	void sendSubmitSmPdu(PduSubmitSm* pdu, PduData* existentPduData, bool sync);
	PduData* setupRandomCorrectReplaceSmPdu(PduReplaceSm* pdu);
	PduData* registerReplaceSm(PduReplaceSm* pdu, PduData* replacePduData,
		time_t submitTime);
	void processReplaceSmSync(PduData* pduData, PduReplaceSmResp* respPdu,
		time_t respTime);
	void processReplaceSmAsync(PduData* pduData, PduReplaceSmResp* respPdu);
	void sendReplaceSmPdu(PduReplaceSm* pdu, PduData* replacePduData, bool sync);
	void sendDeliverySmResp(PduDeliverySmResp& pdu, bool sync);
};

}
}
}

#endif /* TEST_SME_SMPP_TRANSMITTER_TEST_CASES */

