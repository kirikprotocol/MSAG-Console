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
	set<uint32_t> checkReplaceSm(PduData* pduData, PduData* replacePduData,
		PduFlag replacePduFlag);
	void processSubmitSmResp(ResponseMonitor* monitor, PduSubmitSmResp& respPdu,
		time_t respTime);
	void processReplaceSmResp(ResponseMonitor* monitor, PduReplaceSmResp& respPdu,
		time_t respTime);
	void processGenericNack(GenericNackMonitor* monitor, PduGenericNack& respPdu,
		time_t respTime);

private:
	SmppFixture* fixture;
	CheckList* chkList;

	bool checkTransmitter();
};

}
}
}

#endif /* TEST_SME_SMPP_RESPONSE_PDU_CHECKER */

