#ifndef TEST_SME_SMPP_TRANSMITTER_TEST_CASES
#define TEST_SME_SMPP_TRANSMITTER_TEST_CASES

#include "sme/SmppBase.hpp"
#include "test/core/PduUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "SmppFixture.hpp"
#include "SmppPduSender.hpp"
#include "test/util/BaseTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::test::smpp::OPT_ALL;
using namespace smsc::smpp; //pdu
using namespace smsc::test::core;

class SmppTransmitterTestCases : BaseTestCases
{
public:
	SmppTransmitterTestCases(SmppFixture* _fixture)
	: fixture(_fixture), chkList(_fixture->chkList)
	{ fixture->transmitter = this; }
	
	virtual ~SmppTransmitterTestCases() {}

	void setupRandomCorrectSubmitSmPdu(PduSubmitSm* pdu, const Address& destAlias,
		uint64_t mask = OPT_ALL);

	void sendSubmitSmPdu(PduSubmitSm* pdu, PduData* existentPduData, bool sync,
		PduData::IntProps* intProps = NULL, PduData::StrProps* strProps = NULL,
		PduData::ObjProps* objProps = NULL, PduType pduType = PDU_NORMAL);

	void setupRandomCorrectReplaceSmPdu(PduReplaceSm* pdu, PduData* replacePduData);

	void sendReplaceSmPdu(PduReplaceSm* pdu, PduData* replacePduData, bool sync,
		PduData::IntProps* intProps = NULL, PduData::StrProps* strProps = NULL,
		PduData::ObjProps* objProps = NULL);

	void sendQuerySmPdu(PduQuerySm* pdu, PduData* origPduData, bool sync,
		PduData::IntProps* intProps = NULL, PduData::StrProps* strProps = NULL,
		PduData::ObjProps* objProps = NULL);

	void sendCancelSmPdu(PduCancelSm* pdu, PduData* cancelPduData, bool sync,
		PduData::IntProps* intProps = NULL, PduData::StrProps* strProps = NULL,
		PduData::ObjProps* objProps = NULL);

	void sendDeliverySmResp(PduDeliverySmResp& pdu, bool sync, int delay = 0);

	void sendInvalidPdu(SmppHeader* pdu, bool sync);

	/*
	virtual SmppHeader* sendPdu(SmppHeader& pdu)=0;
	virtual void sendGenericNack(PduGenericNack& pdu)=0;
	virtual void sendDataSmResp(PduDataSmResp& pdu)=0;
	virtual PduMultiSmResp* submitm(PduMultiSm& pdu)=0;
	virtual PduDataSmResp* data(PduDataSm& pdu)=0;
	*/
	static uint8_t getRegisteredDelivery(PduData* pduData);
	
protected:
	SmppFixture* fixture;
	CheckList* chkList;

	virtual Category& getLog();
	//общие манипуляции с мониторами
	void cancelMonitor(PduMonitor* monitor, time_t cancelTime,
		bool forceRemoveMonitor);
	void cancelPduMonitors(PduData* pduData, time_t cancelTime,
		bool forceRemoveMonitors, State state);
	void registerTransmitterReportMonitors(uint16_t msgRef, time_t waitTime,
		time_t validTime, PduData* pduData);
	void registerNotBoundReportMonitors(uint16_t msgRef, time_t waitTime,
		time_t validTime, PduData* pduData);
	//submitSm
	void registerNormalSmeMonitors(PduSubmitSm* pdu, PduData* existentPduData,
		uint16_t msgRef, time_t waitTime, time_t validTime, PduData* pduData);
	void registerExtSmeMonitors(PduSubmitSm* pdu, uint16_t msgRef, time_t waitTime,
		time_t validTime, PduData* pduData);
	void registerNullSmeMonitors(PduSubmitSm* pdu, uint16_t msgRef, time_t waitTime,
		time_t validTime, uint32_t deliveryStatus, PduData* pduData);
	PduData* prepareSubmitSm(PduSubmitSm* pdu, PduData* existentPduData,
		time_t submitTime, PduData::IntProps* intProps,
		PduData::StrProps* strProps, PduData::ObjProps* objProps, PduType pduType);
	void processSubmitSmSync(PduData* pduData, PduSubmitSmResp* respPdu,
		time_t respTime);
	void processSubmitSmAsync(PduData* pduData);
	//replaceSm
	void registerReplaceMonitors(PduSubmitSm* resPdu, PduData* replacePduData,
		PduData* pduData);
	PduData* prepareReplaceSm(PduReplaceSm* pdu, PduData* replacePduData,
		time_t submitTime, PduData::IntProps* intProps, PduData::StrProps* strProps,
		PduData::ObjProps* objProps);
	void processReplaceSmSync(PduData* pduData, PduReplaceSm* pdu,
		PduReplaceSmResp* respPdu, time_t respTime);
	void processReplaceSmAsync(PduData* pduData, PduReplaceSm* pdu);
	//querySm
	PduData* prepareQuerySm(PduQuerySm* pdu, PduData* origPduData,
		time_t queryTime, PduData::IntProps* intProps,
		PduData::StrProps* strProps, PduData::ObjProps* objProps);
	void processQuerySmSync(PduData* pduData, PduQuerySmResp* respPdu,
		time_t respTime);
	void processQuerySmAsync(PduData* pduData);
	//cancelSm
	void processCancelledMonitors(PduCancelSm* pdu, PduData* cancelPduData,
		time_t cancelTime, PduData* pduData);
	PduData* prepareCancelSm(PduCancelSm* pdu, PduData* cancelPduData,
		time_t cancelTime, PduData::IntProps* intProps, PduData::StrProps* strProps,
		PduData::ObjProps* objProps);
	void processCancelSmSync(PduData* pduData, PduCancelSmResp* respPdu,
		time_t respTime);
	void processCancelSmAsync(PduData* pduData);
	//generickNack	
	void processGenericNackSync(time_t submitTime, time_t respTime);
	void processGenericNackAsync(PduData* pduData);
};

class DeliverySmRespTask : public PduTask
{
	SmppTransmitterTestCases* transmitter;
    PduDeliverySmResp pdu;
	bool sync;
public:
	DeliverySmRespTask(SmppTransmitterTestCases* _transmitter,
		PduDeliverySmResp& _pdu, bool _sync)
	: transmitter(_transmitter), pdu(_pdu), sync(_sync) {}
	virtual void sendPdu() { transmitter->sendDeliverySmResp(pdu, sync); }
};

}
}
}

#endif /* TEST_SME_SMPP_TRANSMITTER_TEST_CASES */

