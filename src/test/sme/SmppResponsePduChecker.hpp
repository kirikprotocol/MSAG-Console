#ifndef TEST_SME_SMPP_RESPONSE_PDU_CHECKER
#define TEST_SME_SMPP_RESPONSE_PDU_CHECKER

#include "smpp/smpp_structures.h"
#include "test/core/PduRegistry.hpp"
#include "test/core/RouteChecker.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace sme {

using std::vector;
using smsc::test::core::PduRegistry;
using smsc::test::core::RouteChecker;
using smsc::test::core::PduData;
using namespace smsc::smpp; //pdu

class SmppResponsePduChecker
{
public:
	SmppResponsePduChecker(PduRegistry* pduReg, const RouteChecker* routeChecker);
	~SmppResponsePduChecker() {}
	
	vector<int> checkSubmitSmResp(PduData* pduData, PduSubmitSmResp& respPdu);

private:
	const RouteChecker* routeChecker;
	PduRegistry* pduReg;
};

}
}
}

#endif /* TEST_SME_SMPP_RESPONSE_PDU_CHECKER */

