#include "SmppResponsePduChecker.hpp"

namespace smsc {
namespace test {
namespace sme {

using namespace smsc::smpp::SmppCommandSet; //constants

SmppResponsePduChecker::SmppResponsePduChecker()
{
}
	
vector<int> SmppResponsePduChecker::checkSubmitSmResp(
	PduSubmitSm& pdu, PduSubmitSmResp& respPdu)
{
	vector<int> res;
	if (respPdu.get_header().get_commandLength() != 81)
	{
		res.push_back(1);
	}
	if (respPdu.get_header().get_commandId() != SUBMIT_SM_RESP)
	{
		res.push_back(2);
	}
	if (respPdu.get_header().get_sequenceNumber() !=
		pdu.get_header().get_sequenceNumber())
	{
		res.push_back(3);
	}
	//для commandStatus нужны более детальные проверки
	if (respPdu.get_header().get_commandStatus())
	{
		res.push_back(-respPdu.get_header().get_commandStatus());
	}
	return res;
}

}
}
}

