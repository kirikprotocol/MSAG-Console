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
using namespace smsc::test::sms; //constants
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants

static const int NO_ROUTE = 1;
static const int INVALID_VALID_TIME = 2;
static const int INVALID_WAIT_TIME = 3;
static const int INVALID_DATA_CODING = 4;
static const int INVALID_SERVICE_TYPE = 5;
static const int INVALID_SOURCE_ADDR = 6;
static const int TRANSACTION_ROLLBACK = 7;

set<int> SmppPduChecker::checkSubmitSm(PduData* pduData)
{
	__require__(pduData && pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	time_t waitTime = SmppUtil::getWaitTime(
		pdu->get_message().get_scheduleDeliveryTime(), pduData->submitTime);
	time_t validTime = SmppUtil::getValidTime(
		pdu->get_message().get_validityPeriod(), pduData->submitTime);
	Address srcAddr;
	SmppUtil::convert(pdu->get_message().get_source(), &srcAddr);
	set<int> res;
	//без проверки на bound sme
	if (!fixture->routeChecker->isDestReachable(pdu->get_message().get_source(),
		pdu->get_message().get_dest(), false))
	{
		res.insert(NO_ROUTE);
	}
	__cfg_int__(maxValidPeriod);
	if (!validTime || validTime < pduData->submitTime ||
		validTime > pduData->submitTime + maxValidPeriod)
	{
		res.insert(INVALID_VALID_TIME);
	}
	if (!waitTime || (validTime && waitTime > validTime))
	{
		res.insert(INVALID_WAIT_TIME);
	}
	switch (pdu->get_message().get_dataCoding())
	{
		case DATA_CODING_SMSC_DEFAULT:
		case DATA_CODING_UCS2:
			break;
		default:
			res.insert(INVALID_DATA_CODING);
	}
	if (strlen(nvl(pdu->get_message().get_serviceType())) > MAX_SERVICE_TYPE_LENGTH)
	{
		res.insert(INVALID_SERVICE_TYPE);
	}
	if (fixture->smeInfo.rangeOfAddress.length() && fixture->smeAddr != srcAddr)
	{
		res.insert(INVALID_SOURCE_ADDR);
	}
	if (string("-----") == nvl(pdu->get_message().get_serviceType()))
	{
		res.insert(TRANSACTION_ROLLBACK);
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

#define __check__(tcId, errCode) \
	__tc__(tcId); \
	if (!pduRes.count(errCode)) { \
		__tc_fail__(1); \
	} \
	__tc_ok_cond__;

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
					__trace2__("replaceData = {%s}", replaceData->str().c_str());
				}
			}
			break;
		case ESME_RINVDSTADR: //Invalid Dest Addr
			__check__("processResp.checkCmdStatusInvalidDestAddr", NO_ROUTE);
			break;
		case ESME_RINVSCHED: //Invalid Scheduled Delivery Time
			__check__("processResp.checkCmdStatusInvalidWaitTime", INVALID_WAIT_TIME);
			break;
		case ESME_RINVEXPIRY: //Invalid message validity period
			__check__("processResp.checkCmdStatusInvalidValidTime", INVALID_VALID_TIME);
			break;
		case ESME_RINVDCS:
			__check__("processResp.checkCmdStatusInvalidDataCoding", INVALID_DATA_CODING);
			break;
		case ESME_RINVSERTYP:
			__check__("processResp.checkCmdStatusInvalidServiceType", INVALID_SERVICE_TYPE);
			break;
		case ESME_RINVSRCADR:
			__check__("processResp.checkCmdStatusInvalidSourceAddr", INVALID_SOURCE_ADDR);
			break;
		case ESME_RSYSERR:
			__check__("processResp.checkCmdStatusSystemError", TRANSACTION_ROLLBACK);
			break;
		default:
			__tc__("processResp.checkCmdStatusOther");
			__tc_fail__(respPdu.get_header().get_commandStatus());
	}
}

void SmppPduChecker::processGenericNack(GenericNackMonitor* monitor,
	PduGenericNack& respPdu, time_t respTime)
{
	__require__(monitor);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	__tc__("processGenericNack.checkHeader");
	//проверки
	if (respPdu.get_header().get_commandLength() != 16)
	{
		__tc_fail__(1);
	}
	if (respPdu.get_header().get_commandId() != GENERIC_NACK)
	{
		__tc_fail__(2);
	}
	if (respPdu.get_header().get_sequenceNumber() !=
		monitor->pduData->pdu->get_sequenceNumber())
	{
		__tc_fail__(3);
	}
	__tc_ok_cond__;
	__tc__("processGenericNack.checkTime");
	time_t respDelay = respTime - monitor->pduData->submitTime;
	if (respDelay < 0)
	{
		__tc_fail__(1);
	}
	else if (respDelay > timeCheckAccuracy)
	{
		__tc_fail__(2);
	}
	__tc_ok_cond__;
	switch (respPdu.get_header().get_commandStatus())
	{
		case ESME_RINVCMDLEN:
			__tc__("processGenericNack.checkStatusInvalidCommandLength");
			if (monitor->pduData->pdu->get_commandLength() != 16)
			{
				__tc_fail__(1);
			}
			else
			{
				switch (monitor->pduData->pdu->get_commandId())
				{
					case UNBIND:
						__tc_fail__(2);
						break;
					case ENQUIRE_LINK:
						__tc_fail__(3);
						break;
					case SUBMIT_SM:
					case DELIVERY_SM:
					case BIND_TRANCIEVER:
					case GENERIC_NACK:
						//ok;
						break;
					default: //должно быть ESME_RINVCMDID
						__tc_fail__(4);
				}
			}
			__tc_ok_cond__;
			break;
		case ESME_RINVCMDID:
			__tc__("processGenericNack.checkStatusInvalidCommandId");
			switch (monitor->pduData->pdu->get_commandId())
			{
				case BIND_TRANSMITTER:
				case BIND_RECIEVER:
				case BIND_TRANCIEVER:
				case UNBIND:
				case UNBIND_RESP:
				case SUBMIT_SM:
				//case SUBMIT_MULTI:
				//case DATA_SM:
				//case DATA_SM_RESP:
				case DELIVERY_SM_RESP:
				//case QUERY_SM:
				//case CANCEL_SM:
				//case REPLACE_SM:
				case ENQUIRE_LINK:
				case ENQUIRE_LINK_RESP:
				case GENERIC_NACK:
					__tc_fail__(1);
					break;
			}
			__tc_ok_cond__;
			break;
		default:
			__tc__("processGenericNack.checkStatusOther");
			__tc_fail__(respPdu.get_header().get_commandStatus());
	}
}

}
}
}

