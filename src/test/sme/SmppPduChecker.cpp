#include "SmppPduChecker.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include "system/status.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::smpp::SmppUtil;
using smsc::test::conf::TestConfig;
using smsc::test::sms::operator==;
using smsc::test::sms::operator!=;
using smsc::test::smpp::operator==;
using smsc::test::smpp::operator!=;
using namespace std;
using namespace smsc::test::core; //constants
using namespace smsc::test::sms; //constants
using namespace smsc::test::smpp; //constants
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants
using namespace smsc::smpp::DataCoding; //constants
using namespace smsc::system;


inline bool SmppPduChecker::checkTransmitter()
{
	if (fixture->smeType != SME_TRANSMITTER && fixture->smeType != SME_TRANSCEIVER)
	{
		__require__(fixture->smeType == SME_RECEIVER);
		return false;
	}
	return true;
}

#define __check_len__(errCode, field, maxLen) \
	if (strlen(nvl(field)) > maxLen) { \
		/*res.insert(errCode);*/ \
		res.insert(ESME_RSYSERR); \
		return res; \
	}

set<uint32_t> SmppPduChecker::checkSubmitSm(PduData* pduData)
{
	__require__(pduData && pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	time_t waitTime = SmppUtil::getWaitTime(
		pdu->get_message().get_scheduleDeliveryTime(), pduData->sendTime);
	time_t validTime = SmppUtil::getValidTime(
		pdu->get_message().get_validityPeriod(), pduData->sendTime);
	Address srcAddr;
	SmppUtil::convert(pdu->get_message().get_source(), &srcAddr);
	set<uint32_t> res;
	//тип sme
	if (!checkTransmitter())
	{
		res.insert(ESME_RINVBNDSTS);
		return res;
	}
	if (pdu->get_message().size_shortMessage() &&
		pdu->get_optional().has_messagePayload())
	{
		res.insert(ESME_RSUBMITFAIL);
	}
	//неправильная длина полей
	__check_len__(ESME_RINVSERTYP,
		pdu->get_message().get_serviceType(), MAX_SERVICE_TYPE_LENGTH);
	__check_len__(ESME_RINVSRCADR,
		pdu->get_message().get_source().get_value(), MAX_ADDRESS_LENGTH);
	__check_len__(ESME_RINVDSTADR,
		pdu->get_message().get_dest().get_value(), MAX_ADDRESS_LENGTH);
	__check_len__(ESME_RINVEXPIRY,
		pdu->get_message().get_validityPeriod(), MAX_SMPP_TIME_LENGTH);
	__check_len__(ESME_RINVSCHED,
		pdu->get_message().get_scheduleDeliveryTime(), MAX_SMPP_TIME_LENGTH);
	//map
	if (pduData->objProps.count("map.msg"))
	{
		SmsMsg* msg = dynamic_cast<SmsMsg*>(pduData->objProps["map.msg"]);
		__require__(msg);
		if (msg->udhi)
		{
			int msgLen = msg->len;
			if (msg->dataCoding == DEFAULT || msg->dataCoding == SMSC7BIT)
			{
				int udhLen = 1 + *(unsigned char*) msg->msg;
				int textLen = msg->len - udhLen;
				for (int i = udhLen; i < msg->len; i++)
				{
					switch (msg->msg[i])
					{
						case '|':
						case '^':
						case '{':
						case '}':
						case '[':
						case ']':
						case '~':
						case '\\':
							textLen++;
							break;
					}
				}
				msgLen = udhLen + (textLen * 7 + 7) / 8;
			}
			if (msgLen > MAX_MAP_SM_LENGTH)
			{
				res.insert(ESME_RSUBMITFAIL);
			}
		}
		else
		{
			int numSegments; //примерное количество сегментов без учета переноса слов
			int msgLen = msg->len;
			if (msg->dataCoding == DEFAULT || msg->dataCoding == SMSC7BIT)
			{
				numSegments = (msg->len + 152) / 153;
			}
			else
			{
				numSegments = (msg->len + 133) / 134;
			}
			if (numSegments > 255)
			{
				res.insert(ESME_RINVMSGLEN);
			}
		}
	}
	//без проверки на bind статус
	SmeType destType = fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest());
	if (destType == SME_NO_ROUTE)
	{
		res.insert(Status::NOROUTE);
	}
	__cfg_int__(maxValidPeriod);
	if (!validTime || validTime < pduData->sendTime ||
		validTime > pduData->sendTime + maxValidPeriod)
	{
		res.insert(ESME_RINVEXPIRY);
	}
	if (!waitTime || (validTime && waitTime > validTime))
	{
		res.insert(ESME_RINVSCHED);
	}
	switch (pdu->get_message().get_dataCoding())
	{
		case DEFAULT:
		case UCS2:
			break;
		default:
			res.insert(ESME_RINVDCS);
	}
	if (strlen(nvl(pdu->get_message().get_serviceType())) > MAX_SERVICE_TYPE_LENGTH)
	{
		res.insert(ESME_RINVSERTYP);
	}
	if (fixture->smeInfo.rangeOfAddress.length() && fixture->smeAddr != srcAddr)
	{
		res.insert(ESME_RINVSRCADR);
	}
	if (string("-----") == nvl(pdu->get_message().get_serviceType()))
	{
		res.insert(ESME_RSYSERR); //transaction rollback с помощью триггера
	}
	return res;
}

set<uint32_t> SmppPduChecker::checkReplaceSm(PduData* pduData,
	PduData* replacePduData, PduFlag replacePduFlag)
{
	__require__(pduData && pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	time_t waitTime = SmppUtil::getWaitTime(
		pdu->get_message().get_scheduleDeliveryTime(), pduData->sendTime);
	time_t validTime = SmppUtil::getValidTime(
		pdu->get_message().get_validityPeriod(), pduData->sendTime);
	Address srcAddr;
	SmppUtil::convert(pdu->get_message().get_source(), &srcAddr);
	__require__(pduData->strProps.count("smsId"));
	const string& smsId = pduData->strProps["smsId"];
	set<uint32_t> res;
	//тип sme
	if (!checkTransmitter())
	{
		res.insert(ESME_RINVBNDSTS);
		return res;
	}
	//неправильная длина полей
	__check_len__(ESME_RINVMSGID,
		smsId.c_str(), MAX_MSG_ID_LENGTH);
	__check_len__(ESME_RINVSRCADR,
		pdu->get_message().get_source().get_value(), MAX_ADDRESS_LENGTH);
	__check_len__(ESME_RINVEXPIRY,
		pdu->get_message().get_validityPeriod(), MAX_SMPP_TIME_LENGTH);
	__check_len__(ESME_RINVSCHED,
		pdu->get_message().get_scheduleDeliveryTime(), MAX_SMPP_TIME_LENGTH);
	//проверки
	__cfg_int__(maxValidPeriod);
	if (!validTime || validTime < pduData->sendTime ||
		validTime > pduData->sendTime + maxValidPeriod)
	{
		res.insert(ESME_RINVEXPIRY);
	}
	if (!waitTime || (validTime && waitTime > validTime))
	{
		res.insert(ESME_RINVSCHED);
	}
	if (!replacePduData)
	{
		res.insert(ESME_RINVMSGID);
	}
	else
	{
		for (int i = 0; i < smsId.length(); i++)
		{
			if (!isdigit(smsId[i]))
			{
				res.insert(ESME_RINVMSGID);
				break;
			}
		}
		__require__(replacePduData->strProps.count("smsId"));
		if (smsId != replacePduData->strProps["smsId"])
		{
			res.insert(ESME_RINVMSGID);
		}
		__require__(replacePduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* replacePdu = reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
		if (pdu->get_message().get_source() != replacePdu->get_message().get_source())
		{
			res.insert(ESME_RINVSRCADR);
		}
		if (replacePduFlag != PDU_REQUIRED_FLAG)
		{
			res.insert(ESME_RREPLACEFAIL);
		}
	}
	if (fixture->smeInfo.rangeOfAddress.length() && fixture->smeAddr != srcAddr)
	{
		res.insert(ESME_RINVSRCADR);
	}
	return res;
}

set<uint32_t> SmppPduChecker::checkQuerySm(PduData* pduData, PduData* origPduData)
{
	__require__(pduData && pduData->pdu->get_commandId() == QUERY_SM);
	PduQuerySm* pdu = reinterpret_cast<PduQuerySm*>(pduData->pdu);
	set<uint32_t> res;
	//тип sme
	if (!checkTransmitter())
	{
		res.insert(ESME_RINVBNDSTS);
		return res;
	}
	//неправильная длина полей
	__check_len__(ESME_RINVMSGID,
		pdu->get_messageId(), MAX_MSG_ID_LENGTH);
	__check_len__(ESME_RINVSRCADR,
		pdu->get_source().get_value(), MAX_ADDRESS_LENGTH);
	//проверки
	if (pdu->get_messageId())
	{
		for (int i = 0; pdu->get_messageId()[i]; i++)
		{
			if (!isdigit(pdu->get_messageId()[i]))
			{
				res.insert(ESME_RINVMSGID); //для messageId = abc
				res.insert(ESME_RQUERYFAIL); //для messageId = -1
				break;
			}
		}
	}
	if (origPduData && pdu->get_messageId())
	{
		if (!origPduData->strProps.count("smsId"))
		{
			res.insert(ESME_RQUERYFAIL);
		}
		else if (origPduData->strProps["smsId"] != pdu->get_messageId())
		{
			res.insert(ESME_RQUERYFAIL);
		}
		__require__(origPduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* origPdu = reinterpret_cast<PduSubmitSm*>(origPduData->pdu);
		if (pdu->get_source() != origPdu->get_message().get_source())
		{
			res.insert(ESME_RQUERYFAIL);
		}
		//sms в финальном состоянии может быть удалена архиватором
		__require__(origPdu->get_optional().has_userMessageReference());
		DeliveryMonitor* monitor = fixture->pduReg->getDeliveryMonitor(
			origPdu->get_optional().get_userMessageReference());
		if (!monitor || monitor->getFlag() != PDU_REQUIRED_FLAG)
		{
			res.insert(ESME_RQUERYFAIL);
		}
	}
	else
	{
		res.insert(ESME_RQUERYFAIL);
	}
	return res;
}

set<uint32_t> SmppPduChecker::checkCancelSm(PduData* pduData,
	PduData* cancelPduData, PduFlag cancelPduFlag)
{
	__require__(pduData && pduData->pdu->get_commandId() == CANCEL_SM);
	PduCancelSm* pdu = reinterpret_cast<PduCancelSm*>(pduData->pdu);
	set<uint32_t> res;
	//тип sme
	if (!checkTransmitter())
	{
		res.insert(ESME_RINVBNDSTS);
		return res;
	}
	//неправильная длина полей
	__check_len__(ESME_RINVMSGID,
		pdu->get_messageId(), MAX_MSG_ID_LENGTH);
	__check_len__(ESME_RINVSERTYP,
		pdu->get_serviceType(), MAX_SERVICE_TYPE_LENGTH);
	__check_len__(ESME_RINVSRCADR,
		pdu->get_source().get_value(), MAX_ADDRESS_LENGTH);
	__check_len__(ESME_RINVDSTADR,
		pdu->get_dest().get_value(), MAX_ADDRESS_LENGTH);
	static /* const */ PduAddress nullAddr;
	//cancel одиночной sms
	if (pdu->get_messageId())
	{
		for (int i = 0; pdu->get_messageId()[i]; i++)
		{
			if (!isdigit(pdu->get_messageId()[i]))
			{
				res.insert(ESME_RINVMSGID);
				break;
			}
		}
		if (pdu->get_serviceType())
		{
			res.insert(ESME_RCANCELFAIL);
		}
		//какой-то из параметров (source_addr, dest_addr или service_type) сгенерен случайным
		if (!cancelPduData)
		{
			//res.insert(ESME_RCANCELFAIL);
			res.insert(ESME_RINVMSGID); //message_id сгенерен rand_char()
		}
		else if (cancelPduFlag != PDU_REQUIRED_FLAG)
		{
			res.insert(ESME_RCANCELFAIL);
			//res.insert(ESME_RINVMSGID); //если sms уже удалена архиватором
		}
		else
		{
			__require__(cancelPduData->strProps.count("smsId"));
			if (cancelPduData->strProps["smsId"] != nvl(pdu->get_messageId()))
			{
				res.insert(ESME_RCANCELFAIL); //ESME_RINVMSGID
			}
			__require__(cancelPduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* cancelPdu = reinterpret_cast<PduSubmitSm*>(cancelPduData->pdu);
			if (pdu->get_source() != cancelPdu->get_message().get_source())
			{
				res.insert(ESME_RCANCELFAIL); //ESME_RINVSRCADR
			}
			if (pdu->get_dest() != nullAddr &&
				pdu->get_dest() != cancelPdu->get_message().get_dest())
			{
				res.insert(ESME_RCANCELFAIL); //ESME_RINVDSTADR
			}
		}
	}
	//подмножество sms-ок
	else
	{
		__require__(!pdu->get_messageId());
		//какой-то из параметров (source_addr, dest_addr или service_type) сгенерен случайным
		if (!cancelPduData)
		{
			res.insert(ESME_RCANCELFAIL);
		}
		else
		{
			__require__(cancelPduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* cancelPdu = reinterpret_cast<PduSubmitSm*>(cancelPduData->pdu);
			if (pdu->get_source() == nullAddr ||
				pdu->get_source() != cancelPdu->get_message().get_source())
			{
				res.insert(ESME_RCANCELFAIL); //ESME_RINVSRCADR
			}
			if (pdu->get_dest() == nullAddr ||
				pdu->get_dest() != cancelPdu->get_message().get_dest())
			{
				res.insert(ESME_RCANCELFAIL); //ESME_RINVDSTADR
			}
			if (pdu->get_serviceType() &&
				string(pdu->get_serviceType()) != nvl(cancelPdu->get_message().get_serviceType()))
			{
				res.insert(ESME_RCANCELFAIL); //ESME_RINVSERTYP
			}
		}
	}
	return res;
}

#define __check__(errCode, cond) \
	if (!(cond)) { \
		__tc_fail__(errCode); \
	} \

void SmppPduChecker::processSubmitSmResp(ResponseMonitor* monitor,
	PduSubmitSmResp& respPdu, time_t respTime)
{
	__require__(monitor);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	//проверка флагов получения pdu
	time_t respDelay = respTime - monitor->getCheckTime();
	__tc__("submitSm.resp.checkDuplicates");
	switch (monitor->getFlag())
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			__tc_ok__;
			__tc__("submitSm.resp.checkTime");
			__check__(1, respDelay >= 0);
			__check__(2, respDelay <= timeCheckAccuracy);
			monitor->setNotExpected();
			break;
		case PDU_NOT_EXPECTED_FLAG: //респонс уже получен ранее
			__tc_fail__(1);
			break;
		//case PDU_COND_REQUIRED_FLAG:
		default: //респонс всегда должен быть
			__unreachable__("Invalid response flag");
	}
	__tc_ok_cond__;
	//проверки хедера
	__tc__("submitSm.resp.checkHeader");
	__check__(1, respPdu.get_header().get_commandLength() > 16);
	__check__(2, respPdu.get_header().get_commandLength() <= 81);
	__check__(3, respPdu.get_header().get_commandId() == SUBMIT_SM_RESP);
	__check__(4, respPdu.get_header().get_sequenceNumber() ==
		monitor->pduData->pdu->get_sequenceNumber());
	__tc_ok_cond__;
	//проверка ошибок
	if (respPdu.get_header().get_commandStatus() == ESME_ROK)
	{
		//обновить smsId у delivery receipt монитора
		monitor->pduData->strProps["smsId"] = respPdu.get_messageId();
	}
	//set<uint32_t> checkRes = checkSubmitSm(monitor->pduData);
	const set<uint32_t>& checkRes = monitor->pduData->checkRes;
	if (checkRes.size())
	{
		__require__(!monitor->pduData->replacedByPdu);
		__require__(!monitor->pduData->replacePdu);
	}
	switch (respPdu.get_header().get_commandStatus())
	{
		case ESME_ROK:
			{
				__tc__("submitSm.resp.checkCmdStatusOk");
				vector<int> chkRes(checkRes.begin(), checkRes.end());
				__tc_fail2__(chkRes, 0);
			}
			break;
		case ESME_RINVDSTADR:
			__tc__("submitSm.resp.checkCmdStatusInvalidDestAddr");
			__check__(1, checkRes.count(ESME_RINVDSTADR));
			break;
		case ESME_RINVSCHED:
			__tc__("submitSm.resp.checkCmdStatusInvalidWaitTime");
			__check__(1, checkRes.count(ESME_RINVSCHED));
			break;
		case ESME_RINVEXPIRY:
			__tc__("submitSm.resp.checkCmdStatusInvalidValidTime");
			__check__(1, checkRes.count(ESME_RINVEXPIRY));
			break;
		case ESME_RINVDCS:
			__tc__("submitSm.resp.checkCmdStatusInvalidDataCoding");
			__check__(1, checkRes.count(ESME_RINVDCS));
			break;
		case ESME_RINVSERTYP:
			__tc__("submitSm.resp.checkCmdStatusInvalidServiceType");
			__check__(1, checkRes.count(ESME_RINVSERTYP));
			break;
		case ESME_RINVSRCADR:
			__tc__("submitSm.resp.checkCmdStatusInvalidSourceAddr");
			__check__(1, checkRes.count(ESME_RINVSRCADR));
			break;
		case ESME_RSYSERR:
			__tc__("submitSm.resp.checkCmdStatusSystemError");
			__check__(1, checkRes.count(ESME_RSYSERR));
			break;
		case ESME_RINVBNDSTS:
			__tc__("submitSm.resp.checkCmdStatusInvalidBindStatus");
			__check__(1, checkRes.count(ESME_RINVBNDSTS));
			break;
		case ESME_RINVMSGLEN:
			__tc__("submitSm.resp.checkCmdStatusInvalidMsgLen");
			__check__(1, checkRes.count(ESME_RINVMSGLEN));
			break;
		case ESME_RSUBMITFAIL:
			__tc__("submitSm.resp.checkCmdStatusSubmitFailed");
			__check__(1, checkRes.count(ESME_RSUBMITFAIL));
			break;
		case Status::NOROUTE:
			__tc__("submitSm.resp.checkCmdStatusNoRoute");
			__check__(1, checkRes.count(Status::NOROUTE));
			break;
		default:
			__tc__("submitSm.resp.checkCmdStatusOther");
			__tc_fail__(respPdu.get_header().get_commandStatus());
	}
	__tc_ok_cond__;
}

void SmppPduChecker::processReplaceSmResp(ResponseMonitor* monitor,
	PduReplaceSmResp& respPdu, time_t respTime)
{
	__require__(monitor);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	//проверка флагов получения pdu
	time_t respDelay = respTime - monitor->getCheckTime();
	__tc__("replaceSm.resp.checkDuplicates");
	switch (monitor->getFlag())
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			__tc_ok__;
			__tc__("replaceSm.resp.checkTime");
			__check__(1, respDelay >= 0);
			__check__(2, respDelay <= timeCheckAccuracy);
			monitor->setNotExpected();
			break;
		case PDU_NOT_EXPECTED_FLAG: //респонс уже получен ранее
			__tc_fail__(1);
			break;
		//case PDU_COND_REQUIRED_FLAG:
		default: //респонс всегда должен быть
			__unreachable__("Invalid response flag");
	}
	__tc_ok_cond__;
	//проверки полей
	__tc__("replaceSm.resp.checkHeader");
	__check__(1, respPdu.get_header().get_commandLength() == 16);
	__check__(2, respPdu.get_header().get_commandId() == REPLACE_SM_RESP);
	__check__(3, respPdu.get_header().get_sequenceNumber() ==
		monitor->pduData->pdu->get_sequenceNumber());
	__tc_ok_cond__;
	//проверка ошибок
	//set<uint32_t> checkRes = checkReplaceSm(monitor->pduData);
	const set<uint32_t>& checkRes = monitor->pduData->checkRes;
	if (checkRes.size())
	{
		__require__(!monitor->pduData->replacedByPdu);
		__require__(!monitor->pduData->replacePdu);
	}
	switch (respPdu.get_header().get_commandStatus())
	{
		case ESME_ROK:
			{
				__tc__("replaceSm.resp.checkCmdStatusOk");
				vector<int> chkRes(checkRes.begin(), checkRes.end());
				__tc_fail2__(chkRes, 0);
			}
			break;
		case ESME_RINVSCHED:
			__tc__("replaceSm.resp.checkCmdStatusInvalidWaitTime");
			__check__(1, checkRes.count(ESME_RINVSCHED));
			break;
		case ESME_RINVEXPIRY:
			__tc__("replaceSm.resp.checkCmdStatusInvalidValidTime");
			__check__(1, checkRes.count(ESME_RINVEXPIRY));
			break;
		case ESME_RINVSRCADR:
			__tc__("replaceSm.resp.checkCmdStatusInvalidSourceAddr");
			__check__(1, checkRes.count(ESME_RINVSRCADR));
			break;
		case ESME_RSYSERR:
			__tc__("replaceSm.resp.checkCmdStatusSystemError");
			__check__(1, checkRes.count(ESME_RSYSERR));
			break;
		case ESME_RINVBNDSTS:
			__tc__("replaceSm.resp.checkCmdStatusInvalidBindStatus");
			__check__(1, checkRes.count(ESME_RINVBNDSTS));
			break;
		case ESME_RINVMSGID:
			__tc__("replaceSm.resp.checkCmdStatusInvalidMsgId");
			__check__(1, checkRes.count(ESME_RINVMSGID));
			break;
		case ESME_RREPLACEFAIL:
			__tc__("replaceSm.resp.checkCmdStatusReplaceFiled");
			__check__(1, checkRes.count(ESME_RREPLACEFAIL));
			break;
		default:
			__tc__("replaceSm.resp.checkCmdStatusOther");
			__tc_fail__(respPdu.get_header().get_commandStatus());
	}
	__tc_ok_cond__;
}

void SmppPduChecker::checkQuerySmRespStatus(ResponseMonitor* monitor,
	PduQuerySmResp& respPdu, time_t respTime)
{
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);

	DeliveryMonitor* deliveryMonitor = NULL;
	if (monitor->pduData->intProps.count("msgRef"))
	{
		uint16_t msgRef = monitor->pduData->intProps["msgRef"];
		deliveryMonitor = fixture->pduReg->getDeliveryMonitor(msgRef);
		__trace2__("checkQuerySmRespStatus(): monitor = %s", deliveryMonitor->str().c_str());
	}
	time_t finalTime = SmppUtil::string2time(respPdu.get_finalDate(), respTime);
	__trace2__("checkQuerySmRespStatus(): finalTime = %ld", finalTime);
	switch (respPdu.get_messageState())
	{
		case SMPP_ENROUTE_STATE:
			__tc__("querySm.resp.checkFields.enroute");
			__check__(1, !respPdu.get_finalDate());
			__check__(2, respPdu.get_errorCode() == 0);
			__check__(3, deliveryMonitor);
			if (deliveryMonitor)
			{
				switch (deliveryMonitor->getFlag())
				{
					case PDU_REQUIRED_FLAG:
					case PDU_MISSING_ON_TIME_FLAG:
						__require__(deliveryMonitor->state == SMPP_ENROUTE_STATE);
						break;
					case PDU_NOT_EXPECTED_FLAG:
						__check__(4, deliveryMonitor->state == SMPP_EXPIRED_STATE);
						__check__(5, respTime < deliveryMonitor->getValidTime() + timeCheckAccuracy);
						break;
					default: //PDU_COND_REQUIRED_FLAG
						__tc_fail__(6);
				}
			}
			__tc_ok_cond__;
			break;
		case SMPP_DELIVERED_STATE:
			__tc__("querySm.resp.checkFields.delivered");
			if (deliveryMonitor)
			{
				__check__(1, deliveryMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
				__check__(2, deliveryMonitor->state == SMPP_DELIVERED_STATE);
				__check__(3, deliveryMonitor->respTime <= finalTime);
				__check__(4, deliveryMonitor->respTime > finalTime - timeCheckAccuracy);
				//__check__(5, deliveryMonitor->respStatus == respPdu.get_errorCode());
				__tc_ok_cond__;
			}
			break;
		case SMPP_EXPIRED_STATE:
			__tc__("querySm.resp.checkFields.expired");
			if (deliveryMonitor)
			{
				__check__(1, deliveryMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
				__check__(2, deliveryMonitor->state == SMPP_EXPIRED_STATE);
				__check__(3, deliveryMonitor->getValidTime() <= finalTime);
				__check__(4, deliveryMonitor->getValidTime() > finalTime - timeCheckAccuracy);
				//__check__(5, deliveryMonitor->respStatus == respPdu.get_errorCode());
				__tc_ok_cond__;
			}
			break;
		case SMPP_UNDELIVERABLE_STATE:
			__tc__("querySm.resp.checkFields.undeliverable");
			if (deliveryMonitor)
			{
				__check__(1, deliveryMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
				__check__(2, deliveryMonitor->state == SMPP_UNDELIVERABLE_STATE);
				__check__(3, deliveryMonitor->respTime <= finalTime);
				__check__(4, deliveryMonitor->respTime > finalTime - timeCheckAccuracy);
				//__check__(5, deliveryMonitor->respStatus == respPdu.get_errorCode());
				__tc_ok_cond__;
			}
			break;
		case SMPP_DELETED_STATE:
			__tc__("querySm.resp.checkFields.deleted");
			if (deliveryMonitor)
			{
				__check__(1, deliveryMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
				__check__(2, deliveryMonitor->state == SMPP_DELETED_STATE);
				__check__(3, deliveryMonitor->respTime <= finalTime);
				__check__(4, deliveryMonitor->respTime > finalTime - timeCheckAccuracy);
				//__check__(5, deliveryMonitor->respStatus == respPdu.get_errorCode());
				__tc_ok_cond__;
			}
			break;
		default:
			__unreachable__("Invalid state");
	}
}

void SmppPduChecker::processQuerySmResp(ResponseMonitor* monitor,
	PduQuerySmResp& respPdu, time_t respTime)
{
	__require__(monitor && monitor->pduData->pdu->get_commandId() == QUERY_SM);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	PduQuerySm* pdu = reinterpret_cast<PduQuerySm*>(monitor->pduData->pdu);
	//проверка флагов получения pdu
	time_t respDelay = respTime - monitor->getCheckTime();
	__tc__("querySm.resp.checkDuplicates");
	switch (monitor->getFlag())
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			__tc_ok__;
			__tc__("querySm.resp.checkTime");
			__check__(1, respDelay >= 0);
			__check__(2, respDelay <= timeCheckAccuracy);
			monitor->setNotExpected();
			break;
		case PDU_NOT_EXPECTED_FLAG: //респонс уже получен ранее
			__tc_fail__(1);
			break;
		//case PDU_COND_REQUIRED_FLAG:
		default: //респонс всегда должен быть
			__unreachable__("Invalid response flag");
	}
	__tc_ok_cond__;
	//проверки полей
	__tc__("querySm.resp.checkHeader");
	__check__(1, respPdu.get_header().get_commandLength() >= 20 &&
		respPdu.get_header().get_commandLength() <= 100);
	__check__(2, respPdu.get_header().get_commandId() == QUERY_SM_RESP);
	__check__(3, respPdu.get_header().get_sequenceNumber() ==
		monitor->pduData->pdu->get_sequenceNumber());
	__tc_ok_cond__;
	__tc__("querySm.resp.checkFields");
	__check__(1, respPdu.get_errorCode() == rand1(65535));
	if (respPdu.get_header().get_commandStatus() == ESME_ROK)
	{
		__check__(2, !strcmp(nvl(respPdu.get_messageId()), nvl(pdu->get_messageId())));
		checkQuerySmRespStatus(monitor, respPdu, respTime);
	}
	else
	{
		//messageId не проверяю из-за специфики имплементации
		__check__(3, !respPdu.get_finalDate());
		__check__(4, respPdu.get_messageState() == 0);
	}
	__tc_ok_cond__;
	//set<uint32_t> checkRes = checkQuerySm(...);
	const set<uint32_t>& checkRes = monitor->pduData->checkRes;
	switch (respPdu.get_header().get_commandStatus())
	{
		case ESME_ROK:
			{
				__tc__("querySm.resp.checkCmdStatusOk");
				for (set<uint32_t>::const_iterator it = checkRes.begin();
					  it != checkRes.end(); it++)
				{
					//ESME_RQUERYFAIL вставлял на случай если sms уже выгреблась архиватором
					if (*it != ESME_RQUERYFAIL)
					{
						__tc_fail__(*it);
					}
				}
			}
			break;
		case ESME_RINVSRCADR:
			__tc__("querySm.resp.checkCmdStatusInvalidSourceAddr");
			__check__(1, checkRes.count(ESME_RINVSRCADR));
			break;
		case ESME_RINVBNDSTS:
			__tc__("querySm.resp.checkCmdStatusInvalidBindStatus");
			__check__(1, checkRes.count(ESME_RINVBNDSTS));
			break;
		case ESME_RINVMSGID:
			__tc__("querySm.resp.checkCmdStatusInvalidMsgId");
			__check__(1, checkRes.count(ESME_RINVMSGID));
			break;
		case ESME_RQUERYFAIL:
			__tc__("querySm.resp.checkCmdStatusQueryFail");
			__check__(1, checkRes.count(ESME_RQUERYFAIL));
			break;
		default:
			__tc__("querySm.resp.checkCmdStatusOther");
			__tc_fail__(respPdu.get_header().get_commandStatus());
	}
	__tc_ok_cond__;
}

void SmppPduChecker::processCancelSmResp(ResponseMonitor* monitor,
	PduCancelSmResp& respPdu, time_t respTime)
{
	__require__(monitor);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	//проверка флагов получения pdu
	time_t respDelay = respTime - monitor->getCheckTime();
	__tc__("cancelSm.resp.checkDuplicates");
	switch (monitor->getFlag())
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			__tc_ok__;
			__tc__("cancelSm.resp.checkTime");
			__check__(1, respDelay >= 0);
			__check__(2, respDelay <= timeCheckAccuracy);
			monitor->setNotExpected();
			break;
		case PDU_NOT_EXPECTED_FLAG: //респонс уже получен ранее
			__tc_fail__(1);
			break;
		//case PDU_COND_REQUIRED_FLAG:
		default: //респонс всегда должен быть
			__unreachable__("Invalid response flag");
	}
	__tc_ok_cond__;
	//проверки полей
	__tc__("cancelSm.resp.checkHeader");
	__check__(1, respPdu.get_header().get_commandLength() == 16);
	__check__(2, respPdu.get_header().get_commandId() == CANCEL_SM_RESP);
	__check__(3, respPdu.get_header().get_sequenceNumber() ==
		monitor->pduData->pdu->get_sequenceNumber());
	__tc_ok_cond__;
	//проверка ошибок
	//set<uint32_t> checkRes = checkCancelSm(monitor->pduData);
	const set<uint32_t>& checkRes = monitor->pduData->checkRes;
	switch (respPdu.get_header().get_commandStatus())
	{
		case ESME_ROK:
			{
				__tc__("cancelSm.resp.checkCmdStatusOk");
				vector<int> chkRes(checkRes.begin(), checkRes.end());
				__tc_fail2__(chkRes, 0);
			}
			break;
		/*
		case ESME_RINVSRCADR:
			__tc__("cancelSm.resp.checkCmdStatusInvalidSourceAddr");
			__check__(1, checkRes.count(ESME_RINVSRCADR));
			break;
		case ESME_RINVDSTADR:
			__tc__("cancelSm.resp.checkCmdStatusInvalidDestAddr");
			__check__(1, checkRes.count(ESME_RINVDSTADR));
			break;
		*/
		case ESME_RINVBNDSTS:
			__tc__("cancelSm.resp.checkCmdStatusInvalidBindStatus");
			__check__(1, checkRes.count(ESME_RINVBNDSTS));
			break;
		case ESME_RINVMSGID:
			__tc__("cancelSm.resp.checkCmdStatusInvalidMsgId");
			__check__(1, checkRes.count(ESME_RINVMSGID));
			break;
		case ESME_RCANCELFAIL:
			__tc__("cancelSm.resp.checkCmdStatusCancelFailed");
			__check__(1, checkRes.count(ESME_RCANCELFAIL));
			break;
		default:
			__tc__("cancelSm.resp.checkCmdStatusOther");
			__tc_fail__(respPdu.get_header().get_commandStatus());
	}
	__tc_ok_cond__;
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
	time_t respDelay = respTime - monitor->pduData->sendTime;
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

