#include "SmppPduChecker.hpp"
#include "test/smpp/SmppUtil.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::smpp::SmppUtil;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants

static const int NO_ROUTE = 11;
static const int BAD_VALID_TIME = 12;
static const int BAD_WAIT_TIME = 13;

SmppPduChecker::SmppPduChecker(PduRegistry* _pduReg,
	const RouteChecker* _routeChecker)
	: pduReg(_pduReg), routeChecker(_routeChecker) {}
	
set<int> SmppPduChecker::checkSubmitSm(PduSubmitSm* pdu)
{
	set<int> res;
	time_t validTime =
		SmppUtil::string2time(pdu->get_message().get_validityPeriod());
	time_t waitTime =
		SmppUtil::string2time(pdu->get_message().get_scheduleDeliveryTime());
	//без проверки на bound sme
	if (!routeChecker->isDestReachable(pdu->get_message().get_dest(), false))
	{
		res.insert(NO_ROUTE);
	}
	if (validTime < time(NULL) && validTime > __maxValidPeriod__)
	{
		res.insert(BAD_VALID_TIME);
	}
	if (waitTime > validTime)
	{
		res.insert(BAD_WAIT_TIME);
	}
	return res;
}

vector<int> SmppPduChecker::checkSubmitSmResp(
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
	set<int> pduRes = checkSubmitSm(pdu);
	switch (respPdu.get_header().get_commandStatus())
	{
		case ESME_ROK: //No Error
			for (set<int>::const_iterator it = pduRes.begin(); it != pduRes.end(); it++)
			{
				res.push_back(*it);
			}
			//если данная pdu замещает предыдущую pdu
			for (PduData* replaceData = pduData->replacePdu; replaceData; )
			{
				PduData* nextReplaceData = replaceData->replacePdu;
				//replaceData->responseFlag
				replaceData->deliveryFlag = true;
				replaceData->deliveryReceiptFlag = true;
				replaceData->intermediateNotificationFlag = true;
				replaceData->replacePdu = NULL;
				if (replaceData->complete())
				{
					pduReg->removePdu(replaceData);
				}
				replaceData = nextReplaceData;
			}
			pduData->replacePdu = NULL;
			break;
		case ESME_RINVDSTADR: //Invalid Dest Addr
			if (!pduRes.count(NO_ROUTE))
			{
				res.push_back(NO_ROUTE);
			}
			break;
		case ESME_RINVSCHED: //Invalid Scheduled Delivery Time
			if (!pduRes.count(BAD_WAIT_TIME))
			{
				res.push_back(BAD_WAIT_TIME);
			}
			break;
		case ESME_RINVEXPIRY: //Invalid message validity period
			if (!pduRes.count(BAD_VALID_TIME))
			{
				res.push_back(BAD_VALID_TIME);
			}
			break;
		default:
			res.push_back(-respPdu.get_header().get_commandStatus());
			for (set<int>::const_iterator it = pduRes.begin(); it != pduRes.end(); it++)
			{
				res.push_back(*it);
			}
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

}
}
}

