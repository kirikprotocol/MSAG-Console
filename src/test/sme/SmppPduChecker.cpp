#include "SmppPduChecker.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "test/conf/TestConfig.hpp"

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
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants
using namespace smsc::smpp::DataCoding; //constants

inline bool SmppPduChecker::checkTransmitter()
{
	if (fixture->smeType != SME_TRANSMITTER && fixture->smeType != SME_TRANSCEIVER)
	{
		__require__(fixture->smeType == SME_RECEIVER);
		return false;
	}
	return true;
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
	//��� sme
	if (!checkTransmitter())
	{
		res.insert(ESME_RINVBNDSTS);
	}
	//��� �������� �� bind ������
	SmeType destType = fixture->routeChecker->isDestReachable(
		pdu->get_message().get_source(), pdu->get_message().get_dest());
	if (destType == SME_NO_ROUTE)
	{
		res.insert(ESME_RINVDSTADR);
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
		res.insert(ESME_RSYSERR); //transaction rollback � ������� ��������
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
	set<uint32_t> res;
	//��������
	if (!checkTransmitter())
	{
		res.insert(ESME_RINVBNDSTS);
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
	if (!replacePduData)
	{
		res.insert(ESME_RINVMSGID);
	}
	else
	{
		__require__(pduData->strProps.count("smsId"));
		__require__(replacePduData->strProps.count("smsId"));
		if (pduData->strProps["smsId"] != replacePduData->strProps["smsId"])
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
	//�������� ������ ��������� pdu
	time_t respDelay = respTime - monitor->pduData->sendTime;
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
		case PDU_NOT_EXPECTED_FLAG: //������� ��� ������� �����
			__tc_fail__(1);
			break;
		//case PDU_COND_REQUIRED_FLAG:
		default: //������� ������ ������ ����
			__unreachable__("Invalid response flag");
	}
	__tc_ok_cond__;
	//�������� ������
	__tc__("submitSm.resp.checkHeader");
	__check__(1, respPdu.get_header().get_commandLength() > 16);
	__check__(2, respPdu.get_header().get_commandLength() <= 81);
	__check__(3, respPdu.get_header().get_commandId() == SUBMIT_SM_RESP);
	__check__(4, respPdu.get_header().get_sequenceNumber() ==
		monitor->pduData->pdu->get_sequenceNumber());
	__tc_ok_cond__;
	//�������� ������
	if (respPdu.get_header().get_commandStatus() == ESME_ROK)
	{
		//�������� smsId � delivery receipt ��������
		monitor->pduData->strProps["smsId"] = respPdu.get_messageId();
	}
	else
	{
		__require__(!monitor->pduData->replacedByPdu);
		__require__(!monitor->pduData->replacePdu);
	}
	//set<uint32_t> checkRes = checkSubmitSm(monitor->pduData);
	const set<uint32_t>& checkRes = monitor->pduData->checkRes;
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
	//�������� ������ ��������� pdu
	time_t respDelay = respTime - monitor->pduData->sendTime;
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
		case PDU_NOT_EXPECTED_FLAG: //������� ��� ������� �����
			__tc_fail__(1);
			break;
		//case PDU_COND_REQUIRED_FLAG:
		default: //������� ������ ������ ����
			__unreachable__("Invalid response flag");
	}
	__tc_ok_cond__;
	//�������� �����
	__tc__("replaceSm.resp.checkHeader");
	__check__(1, respPdu.get_header().get_commandLength() == 16);
	__check__(2, respPdu.get_header().get_commandId() == REPLACE_SM_RESP);
	__check__(3, respPdu.get_header().get_sequenceNumber() ==
		monitor->pduData->pdu->get_sequenceNumber());
	__tc_ok_cond__;
	//�������� ������
	if (respPdu.get_header().get_commandStatus() != ESME_ROK)
	{
		__require__(!monitor->pduData->replacedByPdu);
		__require__(!monitor->pduData->replacePdu);
	}
	//set<uint32_t> checkRes = checkReplaceSm(monitor->pduData);
	const set<uint32_t>& checkRes = monitor->pduData->checkRes;
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

void SmppPduChecker::processGenericNack(GenericNackMonitor* monitor,
	PduGenericNack& respPdu, time_t respTime)
{
	__require__(monitor);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	__tc__("processGenericNack.checkHeader");
	//��������
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
					default: //������ ���� ESME_RINVCMDID
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

