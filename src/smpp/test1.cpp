/*
	$Id$
*/

#include "smpp/smpp.h"

using namespace smsc::smpp;

int main(void)
{
	printf("begin test");
	try{
		int fd = open("/tmp/smpp_pdu_test1",O_RDWR|C_CREATE|O_TRUNC);
		SmppStream stream;
		assignStreamWith(stream,fd,false);
		PduSubmitSm pdu;
		SmppHeader& pduHeader = pdu.get_smppHeader();
		pduHeader.set_commandId(SmppCommandSet::SUBMIT_SM);
		PduPartSm& sm = pdu.get_message();
		sm.set_shortMessage("short message");
		PduAddress& source = sm.get_source();
		source.set_typeOfNumber(TypeOfNumberValue::NETWORK_SPECIFIC);
		source.set_numberingPlan(NumberingPlanValue::INTERNET);
		source.set_value("1.1.1.1");
		PduAddress& dest = sm.get_dest();
		dest.set_typeOfNumber(TypeOfNumberValue::NETWORK_SPECIFIC);
		dest.set_numberingPlan(NumberingPlanValue::INTERNET);
		dest.set_value("1.1.1.1");
		fillSmppPdu(stream,pdu);
	}
	catch(...)
	{
		__warning__("catch excpetion");
	}
	unlink("/tmp/smpp_pdu_test1");
	printf("end test");
}
