#ifndef TEST_SME_SMPP_RESPONSE_PDU_CHECKER
#define TEST_SME_SMPP_RESPONSE_PDU_CHECKER

#include "smpp/smpp_structures.h"
#include "test/core/PduRegistry.hpp"
#include "test/core/RouteChecker.hpp"
#include <set>
#include <vector>

namespace smsc {
namespace test {
namespace sme {

using std::set;
using std::vector;
using smsc::test::core::PduRegistry;
using smsc::test::core::RouteChecker;
using smsc::test::core::PduData;
using namespace smsc::smpp; //pdu

class SmppPduChecker
{
public:
	SmppPduChecker(PduRegistry* pduReg, const RouteChecker* routeChecker);
	~SmppPduChecker() {}
	
	set<int> checkSubmitSm(PduData* pduData);
	vector<int> checkSubmitSmResp(PduData* pduData, PduSubmitSmResp& respPdu);

private:
	const RouteChecker* routeChecker;
	PduRegistry* pduReg;
};

}
}
}

#endif /* TEST_SME_SMPP_RESPONSE_PDU_CHECKER */

