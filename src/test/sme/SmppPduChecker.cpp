#include "SmppPduChecker.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/conf/TestConfig.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::smpp::SmppUtil;
using smsc::test::util::TestCase;
using smsc::test::util::SyncTestCase;
using smsc::test::conf::TestConfig;
using namespace std;
using namespace smsc::test::core; //constants
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants

static const int NO_ROUTE = 1;
static const int BAD_VALID_TIME = 2;
static const int BAD_WAIT_TIME = 3;

SmppPduChecker::SmppPduChecker(PduRegistry* _pduReg,
	const RouteChecker* _routeChecker, CheckList* _chkList)
	: pduReg(_pduReg), routeChecker(_routeChecker), chkList(_chkList)
{
	__require__(routeChecker);
	__require__(pduReg);
	//__require__(chkList);
}
	
set<int> SmppPduChecker::checkSubmitSm(PduData* pduData)
{
	__require__(pduData && pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	time_t waitTime = SmppUtil::getWaitTime(
		pdu->get_message().get_scheduleDeliveryTime(), pduData->submitTime);
	time_t validTime = SmppUtil::getValidTime(
		pdu->get_message().get_validityPeriod(), pduData->submitTime);
	__require__(pduData->validTime == validTime);
	//__require__(pduData->waitTime == waitTime);
	set<int> res;
	//без проверки на bound sme
	if (!routeChecker->isDestReachable(pdu->get_message().get_source(),
		pdu->get_message().get_dest(), false))
	{
		res.insert(NO_ROUTE);
	}
	__cfg_int__(maxValidPeriod);
	if (!validTime || validTime < pduData->submitTime ||
		validTime > pduData->submitTime + maxValidPeriod)
	{
		res.insert(BAD_VALID_TIME);
	}
	if (!waitTime || (validTime && waitTime > validTime))
	{
		res.insert(BAD_WAIT_TIME);
	}
	return res;
}

void SmppPduChecker::processSubmitSmResp(PduData* pduData,
	PduSubmitSmResp& respPdu, time_t respTime)
{
	__decl_tc__;
	__tc__("processSubmitSmResp.checkHeader");
	//проверки
	if (respPdu.get_header().get_commandLength() < 17 ||
		respPdu.get_header().get_commandLength() > 81)
	{
		__tc_fail__(1);
	}
	if (respPdu.get_header().get_commandId() != SUBMIT_SM_RESP)
	{
		__tc_fail__(2);
	}
	__tc_ok_cond__;
	//дальнейшая обработка
	if (respPdu.get_header().get_commandStatus() == ESME_ROK)
	{
		pduData->smsId = respPdu.get_messageId();
	}
	pduData->submitStatus = respPdu.get_header().get_commandStatus();
	processResp(pduData, respPdu, respTime);
}

void SmppPduChecker::processReplaceSmResp(PduData* pduData,
	PduReplaceSmResp& respPdu, time_t respTime)
{
	__decl_tc__;
	__tc__("processReplaceSmResp.checkHeader");
	//проверки
	if (respPdu.get_header().get_commandLength() != 16)
	{
		__tc_fail__(1);
	}
	if (respPdu.get_header().get_commandId() != REPLACE_SM_RESP)
	{
		__tc_fail__(2);
	}
	__tc_ok_cond__;
	//дальнейшая обработка
	pduData->submitStatus = respPdu.get_header().get_commandStatus();
	processResp(pduData, respPdu, respTime);
}

//Resp = PduSubmitSmResp, PduReplaceSmResp
template <class Resp>
void SmppPduChecker::processResp(PduData* pduData,
	Resp& respPdu, time_t respTime)
{
	__require__(pduData);
	__require__(respTime);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	//обновить smsId и sequenceNumber из респонса
	pduReg->updatePdu(pduData);
	//проверка ошибок
	__tc__("processResp.checkHeader");
	if (respPdu.get_header().get_sequenceNumber() !=
		pduData->pdu->get_sequenceNumber())
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	//проверка флагов получения pdu
	time_t respDelay = respTime - pduData->submitTime;
	__tc__("processResp.checkDuplicates");
	switch (pduData->responseFlag)
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			__tc_ok__;
			__tc__("processResp.checkTime");
			if (respDelay < 0)
			{
				__tc_fail__(1);
			}
			else if (respDelay > timeCheckAccuracy)
			{
				__tc_fail__(2);
			}
			pduData->responseFlag = PDU_RECEIVED_FLAG;
			break;
		case PDU_RECEIVED_FLAG: //респонс уже получен ранее
			__tc_fail__(1);
			break;
		case PDU_NOT_EXPECTED_FLAG: //респонс всегда должен быть
			//break;
		default:
			__unreachable__("Unknown pduData->responseFlag");
	}
	__tc_ok_cond__;
	if (respPdu.get_header().get_commandStatus() != ESME_ROK)
	{
		__tc__("processResp.checkDelivery");
		switch (pduData->deliveryFlag)
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				pduData->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
				break;
			case PDU_RECEIVED_FLAG:
				__tc_fail__(1);
				break;
			case PDU_NOT_EXPECTED_FLAG:
				break;
			default:
				__unreachable__("Unknown pduData->deliveryFlag");
		}
		__tc_ok_cond__;
		__tc__("processResp.checkDeliveryReceipt");
		switch (pduData->deliveryReceiptFlag)
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				pduData->deliveryReceiptFlag = PDU_NOT_EXPECTED_FLAG;
				break;
			case PDU_RECEIVED_FLAG:
				__tc_fail__(1);
				break;
			case PDU_NOT_EXPECTED_FLAG:
				break;
			default:
				__unreachable__("Unknown pduData->deliveryReceiptFlag");
		}
		__tc_ok_cond__;
		__tc__("processResp.checkIntermediateNotification");
		switch (pduData->intermediateNotificationFlag)
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				pduData->intermediateNotificationFlag = PDU_NOT_EXPECTED_FLAG;
				break;
			case PDU_RECEIVED_FLAG:
				__tc_fail__(1);
				break;
			case PDU_NOT_EXPECTED_FLAG:
				break;
			default:
				__unreachable__("Unknown pduData->intermediateNotificationFlag");
		}
		__tc_ok_cond__;
		if (pduData->replacedByPdu)
		{
			pduData->replacedByPdu->replacePdu = NULL;
			pduData->replacedByPdu = NULL;
			__unreachable__("Pdu without response replaced");
		}
		if (pduData->replacePdu)
		{
			pduData->replacePdu->replacedByPdu = NULL;
			pduReg->updatePdu(pduData->replacePdu); //восстановить msgRef, smsId для replace_sm
			pduData->replacePdu = NULL;
		}
	}
	//commandStatus
	set<int> pduRes = checkSubmitSm(pduData);
	switch (respPdu.get_header().get_commandStatus())
	{
		case ESME_ROK: //No Error
			__tc__("processResp.checkCmdStatusOk");
			{
				vector<int> chkRes(pduRes.begin(), pduRes.end());
				__tc_fail2__(chkRes, 10);
			}
			//если данная pdu замещает предыдущую pdu
			for (PduData* replaceData = pduData->replacePdu; replaceData; )
			{
				//replaceData->responseFlag
				if (replaceData->deliveryFlag == PDU_RECEIVED_FLAG)
				{
					__tc_fail__(1);
				}
				if (replaceData->deliveryReceiptFlag == PDU_RECEIVED_FLAG)
				{
					__tc_fail__(2);
				}
				if (replaceData->intermediateNotificationFlag == PDU_RECEIVED_FLAG)
				{
					__tc_fail__(3);
				}
				replaceData->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
				replaceData->deliveryReceiptFlag = PDU_NOT_EXPECTED_FLAG;
				replaceData->intermediateNotificationFlag = PDU_NOT_EXPECTED_FLAG;
				replaceData = replaceData->replacePdu;
			}
			__tc_ok_cond__;
			break;
		case ESME_RINVDSTADR: //Invalid Dest Addr
			__tc__("processResp.checkCmdStatusInvalidDestAddr");
			if (!pduRes.count(NO_ROUTE))
			{
				__tc_fail__(1);
			}
			__tc_ok_cond__;
			break;
		case ESME_RINVSCHED: //Invalid Scheduled Delivery Time
			__tc__("processResp.checkCmdStatusInvalidWaitTime");
			if (!pduRes.count(BAD_WAIT_TIME))
			{
				__tc_fail__(1);
			}
			__tc_ok_cond__;
			break;
		case ESME_RINVEXPIRY: //Invalid message validity period
			__tc__("processResp.checkCmdStatusInvalidValidTime");
			if (!pduRes.count(BAD_VALID_TIME))
			{
				__tc_fail__(1);
			}
			__tc_ok_cond__;
			break;
		default:
			__tc__("processResp.checkCmdStatusOther");
			__tc_fail__(respPdu.get_header().get_commandStatus());
	}
}

}
}
}

