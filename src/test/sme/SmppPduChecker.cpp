#include "SmppPduChecker.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "test/conf/TestConfig.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::smpp::SmppUtil;
using smsc::test::conf::TestConfig;
using namespace std;
using namespace smsc::test::core; //constants
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants

static const int NO_ROUTE = 1;
static const int BAD_VALID_TIME = 2;
static const int BAD_WAIT_TIME = 3;
static const int BAD_DATA_CODING = 4;

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
	__require__(pduData && pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	time_t waitTime = SmppUtil::getWaitTime(
		pdu->get_message().get_scheduleDeliveryTime(), pduData->submitTime);
	time_t validTime = SmppUtil::getValidTime(
		pdu->get_message().get_validityPeriod(), pduData->submitTime);
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
	switch (pdu->get_message().get_dataCoding())
	{
		case DATA_CODING_SMSC_DEFAULT:
		case DATA_CODING_UCS2:
			break;
		default:
			res.insert(BAD_DATA_CODING);
	}
	return res;
}

void SmppPduChecker::processSubmitSmResp(ResponseMonitor* monitor,
	PduSubmitSmResp& respPdu, time_t respTime)
{
	__require__(monitor);
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
	//обновить smsId у delivery receipt монитора
	if (respPdu.get_header().get_commandStatus() == ESME_ROK)
	{
		monitor->pduData->smsId = respPdu.get_messageId();
	}
	//дальнейшая обработка
	processResp(monitor, respPdu, respTime);
}

void SmppPduChecker::processReplaceSmResp(ResponseMonitor* monitor,
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
	processResp(monitor, respPdu, respTime);
}

//Resp = PduSubmitSmResp, PduReplaceSmResp
template <class Resp>
void SmppPduChecker::processResp(ResponseMonitor* monitor,
	Resp& respPdu, time_t respTime)
{
	__require__(monitor);
	__require__(respTime);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	//проверка ошибок
	__tc__("processResp.checkHeader");
	if (respPdu.get_header().get_sequenceNumber() !=
		monitor->pduData->pdu->get_sequenceNumber())
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	//проверка флагов получения pdu
	time_t respDelay = respTime - monitor->pduData->submitTime;
	__tc__("processResp.checkDuplicates");
	switch (monitor->getFlag())
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
			monitor->setFlag(PDU_RECEIVED_FLAG);
			break;
		case PDU_RECEIVED_FLAG: //респонс уже получен ранее
			__tc_fail__(1);
			break;
		default: //респонс всегда должен быть
			__unreachable__("Invalid pduData->responseFlag");
	}
	__tc_ok_cond__;
	if (respPdu.get_header().get_commandStatus() != ESME_ROK)
	{
		monitor->pduData->valid = false;
		if (monitor->pduData->replacedByPdu)
		{
			__unreachable__("Pdu without response replaced");
		}
		PduData* replaceData = monitor->pduData->replacePdu;
		if (replaceData)
		{
			replaceData->replacedByPdu = NULL;
			replaceData->valid = true;
			monitor->pduData->replacePdu = NULL;
		}
	}
	//commandStatus
	set<int> pduRes = checkSubmitSm(monitor->pduData);
	switch (respPdu.get_header().get_commandStatus())
	{
		case ESME_ROK: //No Error
			{
				__tc__("processResp.checkCmdStatusOk");
				vector<int> chkRes(pduRes.begin(), pduRes.end());
				__tc_fail2__(chkRes, 10);
				__tc_ok_cond__;
				//положительный респонс получен, pdu валидная
				monitor->pduData->valid = true;
				//если данная pdu замещает другую pdu
				PduData* replaceData = monitor->pduData->replacePdu;
				if (replaceData)
				{
					replaceData->valid = false;
					replaceData = replaceData->replacePdu;
				}
			}
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
		case ESME_RINVDCS:
			__tc__("processResp.checkCmdStatusInvalidDataCoding");
			if (!pduRes.count(BAD_DATA_CODING))
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

