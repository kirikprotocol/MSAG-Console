#include "SmppPduChecker.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/TestConfig.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::smpp::SmppUtil;
using namespace smsc::test; //config constants
using namespace smsc::test::core; //constants
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants

static const int NO_ROUTE = 21;
static const int BAD_VALID_TIME = 22;
static const int BAD_WAIT_TIME = 23;

SmppPduChecker::SmppPduChecker(PduRegistry* _pduReg,
	const RouteChecker* _routeChecker)
	: pduReg(_pduReg), routeChecker(_routeChecker) {}
	
set<int> SmppPduChecker::checkSubmitSm(PduData* pduData)
{
	__require__(pduData && pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	time_t waitTime = SmppUtil::string2time(
		pdu->get_message().get_scheduleDeliveryTime(), pduData->submitTime);
	time_t validTime = SmppUtil::string2time(
		pdu->get_message().get_validityPeriod(), pduData->submitTime);
	__require__(pduData->validTime == validTime);
	//__require__(pduData->waitTime == waitTime);
	set<int> res;
	//без проверки на bound sme
	if (!routeChecker->isDestReachable(pdu->get_message().get_dest(), false))
	{
		res.insert(NO_ROUTE);
	}
	if (validTime < pduData->submitTime ||
		validTime > pduData->submitTime + maxValidPeriod)
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
	PduData* pduData, PduSubmitSmResp& respPdu, time_t respTime)
{
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
	//проверка флагов получения pdu
	time_t respDelay = respTime - pduData->submitTime;
	switch (pduData->responseFlag)
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			if (respDelay < 0)
			{
				res.push_back(4);
			}
			else if (respDelay > timeCheckAccuracy)
			{
				res.push_back(5);
			}
			pduData->responseFlag = PDU_RECEIVED_FLAG;
			break;
		case PDU_RECEIVED_FLAG: //респонс уже получен ранее
			res.push_back(6);
			break;
		case PDU_NOT_EXPECTED_FLAG: //респонс всегда должен быть
		default:
			__unreachable__("Unknown pduData->responseFlag");
	}
	if (respPdu.get_header().get_commandStatus() != ESME_ROK)
	{
		switch (pduData->deliveryFlag)
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				pduData->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
				break;
			case PDU_RECEIVED_FLAG:
				res.push_back(7);
				break;
			case PDU_NOT_EXPECTED_FLAG:
				//ok
				break;
			default:
				__unreachable__("Unknown pduData->deliveryFlag");
		}
		switch (pduData->deliveryReceiptFlag)
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				pduData->deliveryReceiptFlag = PDU_NOT_EXPECTED_FLAG;
				break;
			case PDU_RECEIVED_FLAG:
				res.push_back(8);
				break;
			case PDU_NOT_EXPECTED_FLAG:
				//ok
				break;
			default:
				__unreachable__("Unknown pduData->deliveryReceiptFlag");
		}
		switch (pduData->intermediateNotificationFlag)
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				pduData->intermediateNotificationFlag = PDU_NOT_EXPECTED_FLAG;
				break;
			case PDU_RECEIVED_FLAG:
				res.push_back(9);
				break;
			case PDU_NOT_EXPECTED_FLAG:
				//ok
				break;
			default:
				__unreachable__("Unknown pduData->intermediateNotificationFlag");
		}
	}
	//commandStatus
	set<int> pduRes = checkSubmitSm(pduData);
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
				//replaceData->responseFlag
				if (replaceData->deliveryFlag == PDU_RECEIVED_FLAG)
				{
					res.push_back(11);
				}
				if (replaceData->deliveryReceiptFlag == PDU_RECEIVED_FLAG)
				{
					res.push_back(12);
				}
				if (replaceData->intermediateNotificationFlag == PDU_RECEIVED_FLAG)
				{
					res.push_back(13);
				}
				replaceData->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
				replaceData->deliveryReceiptFlag = PDU_NOT_EXPECTED_FLAG;
				replaceData->intermediateNotificationFlag = PDU_NOT_EXPECTED_FLAG;
				replaceData = replaceData->replacePdu;
			}
			break;
		case ESME_RINVDSTADR: //Invalid Dest Addr
			if (!pduRes.count(NO_ROUTE))
			{
				res.push_back(-ESME_RINVDSTADR);
				res.push_back(NO_ROUTE);
			}
			break;
		case ESME_RINVSCHED: //Invalid Scheduled Delivery Time
			if (!pduRes.count(BAD_WAIT_TIME))
			{
				res.push_back(-ESME_RINVSCHED);
				res.push_back(BAD_WAIT_TIME);
			}
			break;
		case ESME_RINVEXPIRY: //Invalid message validity period
			if (!pduRes.count(BAD_VALID_TIME))
			{
				res.push_back(-ESME_RINVEXPIRY);
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
	return res;
}

}
}
}

