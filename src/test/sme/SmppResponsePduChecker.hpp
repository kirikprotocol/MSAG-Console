#ifndef TEST_SME_SMPP_RESPONSE_PDU_CHECKER
#define TEST_SME_SMPP_RESPONSE_PDU_CHECKER

#include "smpp/smpp_structures.h"
#include <vector>

namespace smsc {
namespace test {
namespace sme {

using std::vector;
using namespace smsc::smpp; //pdu

class SmppResponsePduChecker
{
public:
	SmppResponsePduChecker();
	~SmppResponsePduChecker() {}
	
	vector<int> checkSubmitSmResp(PduSubmitSm& pdu, PduSubmitSmResp& respPdu);
};

}
}
}

#endif /* TEST_SME_SMPP_RESPONSE_PDU_CHECKER */

