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

	void sendReplaceSmPdu(PduReplaceSm* pdu, PduData* replacePduData, bool sync);

	void sendQuerySmPdu(PduQuerySm* pdu, bool sync);

	void sendCancelSmPdu(PduCancelSm* pdu, bool sync);

	void sendDeliverySmResp(PduDeliverySmResp& pdu, bool sync, int delay = 0);

	void sendInvalidPdu(SmppHeader* pdu, bool sync);

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
	static uint8_t getRegisteredDelivery(PduData* pduData);
	
protected:
	SmppFixture* fixture;
	CheckList* chkList;

	virtual Category& getLog();
	void updateReplacePduMonitors(PduData* pduData, time_t submitTime);
	void registerNormalSmeMonitors(PduSubmitSm* pdu, PduData* existentPduData,
		const Profile& profile, uint16_t msgRef, time_t waitTime, time_t validTime,
		PduData* pduData);
	void registerExtSmeMonitors(PduSubmitSm* pdu, uint16_t msgRef, time_t waitTime,
		time_t validTime, PduData* pduData);
	void registerNullSmeMonitors(PduSubmitSm* pdu, uint16_t msgRef, time_t waitTime,
		time_t validTime, uint32_t deliveryStatus, PduData* pduData);
	PduData* registerSubmitSm(PduSubmitSm* pdu, PduData* existentPduData,
		time_t submitTime, PduData::IntProps* intProps,
		PduData::StrProps* strProps, PduData::ObjProps* objProps, PduType pduType);
	void processSubmitSmSync(PduData* pduData, PduSubmitSmResp* respPdu,
		time_t respTime);
	void processSubmitSmAsync(PduData* pduData);
	PduData* registerReplaceSm(PduReplaceSm* pdu, PduData* replacePduData,
		time_t submitTime);
	void processReplaceSmSync(PduData* pduData, PduReplaceSmResp* respPdu,
		time_t respTime);
	void processReplaceSmAsync(PduData* pduData);
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

