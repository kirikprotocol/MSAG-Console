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
using namespace smsc::test::util; //TCResult, BaseTestCases
using namespace smsc::test::core; //SmeRegistry, PduRegistry, ...

const char* const TC_SUBMIT_SM_SYNC = "submitSmSync";
const char* const TC_SUBMIT_SM_ASYNC = "submitSmAsync";
const char* const TC_SUBMIT_SM_ASSERT = "submitSmAssert";

/**
 * Абстрактный класс для сбора статистики по billing и archived pdu.
 */
struct StatHandler
{
	virtual void updateStat(bool billing, bool archived) = NULL;
};

class SmppTransmitterTestCases : BaseTestCases
{
public:
	SmppTransmitterTestCases(SmppSession* session, const SmeSystemId& systemId,
		const Address& smeAddr, const SmeRegistry* smeReg, StatHandler* statHandler,
		RouteChecker* routeChecker, SmppPduChecker* pduChecker);
	
	virtual ~SmppTransmitterTestCases() {}

	/**
	 * Синхронная отправка submit_sm pdu другим sme.
	 */
	TCResult* submitSmSync(int num);

	/**
	 * Асинхронная отправка submit_sm pdu другим sme.
	 */
	TCResult* submitSmAsync(int num);

	/**
	 * Заполнение и отправка submit_sm pdu с недопустимыми значениями полей.
	 */
	TCResult* submitSmAssert(int num);

	/**
	 * Синхронное замещение ранее отправленной submit_sm pdu.
	 */
	TCResult* replaceSmSync(int num);

	/**
	 * Асинхронное замещение ранее отправленнй submit_sm pdu.
	 */
	TCResult* replaceSmAsync(int num);

	/**
	 * Заполнение и отправка replace_sm pdu с недопустимыми значениями полей.
	 */
	TCResult* replaceSmAssert(int num);
	
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
	StatHandler* statHandler;
	RouteChecker* routeChecker;
	SmppPduChecker* pduChecker;

	void fillSubmitSmPduData(PduData* pduData, PduSubmitSm* pdu,
		PduData* replacePduData);
	vector<int> submitAndRegisterSmSync(PduSubmitSm* pdu, PduData* replacePduData);
	vector<int> submitAndRegisterSmAsync(PduSubmitSm* pdu, PduData* replacePduData);
	TCResult* submitSm(const char* tc, bool sync, int num);
};

}
}
}

#endif /* TEST_SME_SMPP_TRANSMITTER_TEST_CASES */

