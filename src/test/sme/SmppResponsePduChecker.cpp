#include "SmppResponsePduChecker.hpp"
#include "test/smpp/SmppUtil.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::smpp::SmppUtil;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants

SmppResponsePduChecker::SmppResponsePduChecker(PduRegistry* _pduReg,
	const RouteChecker* _routeChecker)
	: pduReg(_pduReg), routeChecker(_routeChecker) {}
	
vector<int> SmppResponsePduChecker::checkSubmitSmResp(
	PduData* pduData, PduSubmitSmResp& respPdu)
{
	__require__(pduData && pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
	vector<int> res;
	//respPdu
	if (respPdu.get_header().get_commandLength() < 17 ||
		respPdu.get_header().get_commandLength() > 81)
	{
		res.push_back(1);
	}
	if (respPdu.get_header().get_commandId() != SUBMIT_SM_RESP)
	{
		res.push_back(2);
	}
	if (respPdu.get_header().get_sequenceNumber() !=
		pduData->pdu->get_sequenceNumber())
	{
		res.push_back(3);
	}
	//time
	if (pduData->submitTime < __checkTime__)
	{
		res.push_back(4);
	}
	//commandStatus
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	time_t validTime =
		SmppUtil::string2time(pdu->get_message().get_validityPeriod());
	time_t waitTime =
		SmppUtil::string2time(pdu->get_message().get_scheduleDeliveryTime());
	switch (respPdu.get_header().get_commandStatus())
	{
		case ESME_ROK: //No Error
			if (!routeChecker->isDestReachable(
				pdu->get_message().get_dest(), false)) //без проверки на bound sme
			{
				res.push_back(11);
			}
			if (validTime < time(NULL))
			{
				res.push_back(12);
			}
			if (validTime > __maxValidPeriod__)
			{
				res.push_back(13);
			}
			if (validTime < waitTime)
			{
				res.push_back(14);
			}
			//если данная pdu замещает предыдущую pdu
			for (PduData* replaceData = pduData->replacePdu; replaceData; )
			{
				PduData* nextReplaceData = replaceData->replacePdu;
				//replaceData->responseFlag
				replaceData->deliveryFlag = true;
				replaceData->deliveryReceiptFlag = true;
				replaceData->intermediateNotificationFlag = true;
				if (replaceData->complete())
				{
					pduReg->removePdu(replaceData);
				}
				replaceData = nextReplaceData;
			}
			pduData->replacePdu = NULL;
			break;
		case ESME_RINVDSTADR: //Invalid Dest Addr
			if (routeChecker->isDestReachable(
				pdu->get_message().get_dest(), false)) //без проверки на bound sme
			{
				res.push_back(15);
			}
			break;
		case ESME_RINVSCHED: //Invalid Scheduled Delivery Time
			if (validTime < waitTime)
			{
				res.push_back(16);
			}
			break;
		case ESME_RINVEXPIRY: //Invalid message validity period
			if (validTime > time(NULL) && validTime <= __maxValidPeriod__)
			{
				res.push_back(17);
			}
			break;
		default:
			res.push_back(-respPdu.get_header().get_commandStatus());
	}
	if (respPdu.get_header().get_commandStatus() != ESME_ROK)
	{
		pduData->deliveryFlag = true;
		pduData->deliveryReceiptFlag = true;
		pduData->intermediateNotificationFlag = true;

	}
	pduData->responseFlag = true;
	return res;
}

/*
ESME_RINVDSTADR 0x0000000B Invalid Dest Addr
ESME_RINVSCHED 0x00000061 Invalid Scheduled Delivery Time
ESME_RINVEXPIRY 0x00000062 Invalid message validity period (Expiry time)
bool SmppTestCases::checkMissingDeliveryPdu(PduData* pduData)
{
	return ();
}
*/

}
}
}

