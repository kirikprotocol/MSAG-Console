#ifndef TEST_SME_SMPP_RESPONSE_PDU_CHECKER
#define TEST_SME_SMPP_RESPONSE_PDU_CHECKER

#include "smpp/smpp_structures.h"
#include "SmppFixture.hpp"
#include <set>
#include <vector>

namespace smsc {
namespace test {
namespace sme {

using std::set;
using std::vector;
using namespace smsc::test::core; //monitors, PduRegistry, RouteChecker
using smsc::test::util::CheckList;
using namespace smsc::smpp; //pdu

class SmppPduChecker
{
public:
	SmppPduChecker(SmppFixture* _fixture)
	: fixture(_fixture), chkList(_fixture->chkList)
	{ fixture->pduChecker = this; }

	~SmppPduChecker() {}
	
	set<uint32_t> checkSubmitSm(PduData* pduData);
	set<uint32_t> checkDataSm(PduData* pduData);
	set<uint32_t> checkReplaceSm(PduData* pduData, PduData* replacePduData,
		PduFlag replacePduFlag);
	set<uint32_t> checkQuerySm(PduData* pduData, PduData* origPduData);
	set<uint32_t> checkCancelSm(PduData* pduData, PduData* cancelPduData,
		PduFlag cancelPduFlag);
	void processSubmitSmResp(ResponseMonitor* monitor, PduSubmitSmResp& respPdu,
		time_t respTime);
	void processDataSmResp(ResponseMonitor* monitor, PduDataSmResp& respPdu,
		time_t respTime);
	void processReplaceSmResp(ResponseMonitor* monitor, PduReplaceSmResp& respPdu,
		time_t respTime);
	void processQuerySmResp(ResponseMonitor* monitor, PduQuerySmResp& respPdu,
		time_t respTime);
	void processCancelSmResp(ResponseMonitor* monitor, PduCancelSmResp& respPdu,
		time_t respTime);
	void processGenericNack(GenericNackMonitor* monitor, PduGenericNack& respPdu,
		time_t respTime);

private:
	SmppFixture* fixture;
	CheckList* chkList;

	typedef enum
	{
		MSG_OK = 0,
		UDHI_SINGLE_SEGMENT_ERR = 1, //установлен udhi и сообщение разбивается на несколько сегментов
		MAX_SEGMENTS_ERR = 2 //кол-во сегментов > 255
	} MapMsgError;


	bool checkTransmitter();
	MapMsgError checkMapMsg(SmsMsg* msg);
	void checkQuerySmRespStatus(ResponseMonitor* monitor, PduQuerySmResp& respPdu,
		time_t respTime);
	void updateRecipientData(PduData* pduData, time_t respTime);
};

}
}
}

#endif /* TEST_SME_SMPP_RESPONSE_PDU_CHECKER */

