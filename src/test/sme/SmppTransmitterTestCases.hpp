#ifndef TEST_SME_SMPP_TRANSMITTER_TEST_CASES
#define TEST_SME_SMPP_TRANSMITTER_TEST_CASES

#include "sme/SmppBase.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "SmppFixture.hpp"
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
		PduData::ObjProps* objProps = NULL, bool normalSms = true);

	void setupRandomCorrectReplaceSmPdu(PduReplaceSm* pdu, PduData* replacePduData);

	void sendReplaceSmPdu(PduReplaceSm* pdu, PduData* replacePduData, bool sync);

	void sendDeliverySmResp(PduDeliverySmResp& pdu, bool sync);

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
	SmppFixture* fixture;
	CheckList* chkList;

	virtual Category& getLog();
	template <class Message>
	bool hasDeliveryReceipt(Message& m, Profile& profile);
	template <class Message>
	bool hasIntermediateNotification(Message& m, Profile& profile);
	template <class Message>
	void checkRegisteredDelivery(Message& m);
	PduData* registerSubmitSm(PduSubmitSm* pdu, PduData* existentPduData,
		time_t submitTime, PduData::IntProps* intProps,
		PduData::StrProps* strProps, PduData::ObjProps* objProps, bool normalSms);
	void processSubmitSmSync(PduData* pduData, PduSubmitSmResp* respPdu,
		time_t respTime);
	void processSubmitSmAsync(PduData* pduData);
	PduData* registerReplaceSm(PduReplaceSm* pdu, PduData* replacePduData,
		time_t submitTime);
	void processReplaceSmSync(PduData* pduData, PduReplaceSmResp* respPdu,
		time_t respTime);
	void processReplaceSmAsync(PduData* pduData);
};

}
}
}

#endif /* TEST_SME_SMPP_TRANSMITTER_TEST_CASES */

