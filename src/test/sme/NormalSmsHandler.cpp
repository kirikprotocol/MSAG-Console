#include "NormalSmsHandler.hpp"
#include "SmppTransmitterTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/core/PduUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "profiler/profiler.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::sms::Address;
using smsc::util::Logger;
using smsc::test::conf::TestConfig;
using namespace smsc::profiler;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::smpp::DataCoding;
using namespace smsc::test::smpp;
using namespace smsc::test::core;
using namespace smsc::test::util;

NormalSmsHandler::NormalSmsHandler(SmppFixture* _fixture)
: fixture(_fixture), chkList(_fixture->chkList) {}

Category& NormalSmsHandler::getLog()
{
	static Category& log = Logger::getCategory("NormalSmsHandler");
	return log;
}

vector<int> NormalSmsHandler::checkRoute(PduSubmitSm& pdu1, PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, destAlias1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAlias1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//������������ destAddr
	const RouteHolder* routeHolder = NULL;
	const Address destAddr = fixture->aliasReg->findAddressByAlias(destAlias1);
	if (destAddr != destAddr2)
	{
		res.push_back(1);
	}
	else
	{
		//������������ ��������
		routeHolder = fixture->routeReg->lookup(origAddr1, destAddr2);
		if (!routeHolder)
		{
			res.push_back(2);
		}
		else if (fixture->smeInfo.systemId != routeHolder->route.smeSystemId)
		{
			res.push_back(3);
		}
	}
	//������������ origAddr
	if (routeHolder)
	{
		if (fixture->smeInfo.wantAlias)
		{
			const Address origAlias = fixture->aliasReg->findAliasByAddress(origAddr1);
			if (origAlias != origAlias2)
			{
				res.push_back(4);
			}
		}
		else if (origAddr1 != origAlias2)
		{
			res.push_back(5);
		}
	}
	return res;
}

void NormalSmsHandler::compareMsgText(PduSubmitSm& origPdu, PduDeliverySm& pdu,
	time_t recvTime)
{
	__require__(fixture->profileReg);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	Address destAddr;
	SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
	time_t profileUpdateTime;
	int codePage = fixture->profileReg->getProfile(destAddr, profileUpdateTime).codepage;
	if (abs(recvTime - profileUpdateTime) < timeCheckAccuracy)
	{
		//������� ����� ���� ���������������
		return;
	}
	//pdu
	uint8_t dc = pdu.get_message().get_dataCoding();
	const char* sm = pdu.get_message().get_shortMessage();
	uint8_t smLen = pdu.get_message().get_smLength();
	const char* mp = pdu.get_optional().has_messagePayload() ?
		pdu.get_optional().get_messagePayload() : NULL;
	int mpLen = pdu.get_optional().has_messagePayload() ?
		pdu.get_optional().size_messagePayload() : 0;
	//origPdu
	uint8_t origDc = origPdu.get_message().get_dataCoding();
	const char* origSm = origPdu.get_message().get_shortMessage();
	uint8_t origSmLen = origPdu.get_message().get_smLength();
	const char* origMp = origPdu.get_optional().has_messagePayload() ?
		origPdu.get_optional().get_messagePayload() : NULL;
	int origMpLen = origPdu.get_optional().has_messagePayload() ?
		origPdu.get_optional().size_messagePayload() : 0;
	//��������� ������������ language_indicator
	switch(codePage)
	{
		case ProfileCharsetOptions::Default:
			__tc__("deliverySm.normalSms.checkDataCoding");
			if (dc != DEFAULT)
			{
				__tc_fail__(1);
			}
			__tc_ok_cond__;
			if (dc == DEFAULT)
			{
				if (origDc == DEFAULT)
				{
					__tc__("deliverySm.normalSms.checkTextEqualDataCoding");
				}
				else
				{
					__tc__("deliverySm.normalSms.checkTextDiffDataCoding");
				}
				__tc_fail2__(compare(origDc, origSm, origSmLen, dc, sm, smLen, false), 0);
				__tc_fail2__(compare(origDc, origMp, origMpLen, dc, mp, mpLen, false), 10);
				__tc_ok_cond__;
			}
			break;
		case ProfileCharsetOptions::Ucs2:
			if (dc == origDc)
			{
				__tc__("deliverySm.normalSms.checkTextEqualDataCoding");
			}
			else
			{
				__tc__("deliverySm.normalSms.checkTextDiffDataCoding");
			}
			__tc_fail2__(compare(origDc, origSm, origSmLen, dc, sm, smLen, false), 0);
			__tc_fail2__(compare(origDc, origMp, origMpLen, dc, mp, mpLen, false), 10);
			__tc_ok_cond__;
			break;
		default:
			__unreachable__("Invalid profile");
	}
}

void NormalSmsHandler::registerIntermediateNotificationMonitor(
	const DeliveryMonitor* monitor, PduRegistry* pduReg, uint32_t deliveryStatus,
	time_t recvTime, time_t respTime)
{
	__require__(monitor && pduReg);
	//intermediate notification monitor ������������ ������ ����� ������
	//��������� ������� �������� � rescheduling
	if (monitor->getLastAttempt())
	{
		return;
	}
	IntermediateNotificationMonitor* m =
		pduReg->getIntermediateNotificationMonitor(monitor->msgRef);
	//__require__(!m);
	if (m)
	{
		__warning2__("registerIntermediateNotificationMonitor(): monitor = %p already registered", m);
		return;
	}
	__decl_tc__;
	uint8_t regDelivery =
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData);
	//flag
	PduFlag flag;
	switch (regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			return;
		case FINAL_SMSC_DELIVERY_RECEIPT:
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			flag = PDU_REQUIRED_FLAG;
			break;
		default:
			__unreachable__("Invalid regDelivery");
	}
	if (monitor->pduData->intProps.count("ussdServiceOp"))
	{
		__tc__("deliverySm.reports.intermediateNotification.ussdServiceOp");
		__tc_ok__;
		return;
	}
	//startTime
	time_t startTime;
	RespPduFlag respFlag = isAccepted(deliveryStatus);
	switch (respFlag)
	{
		case RESP_PDU_OK:
		case RESP_PDU_ERROR:
			return; //��������� �������� �� �����
		case RESP_PDU_RESCHED:
			startTime = respTime;
			break;
		case RESP_PDU_MISSING:
			startTime = recvTime + fixture->smeInfo.timeout - 1;
			break;
		default:
			__unreachable__("Invalid respFlag");
	}
	//register
	IntermediateNotificationMonitor* notifMonitor =
		new IntermediateNotificationMonitor(monitor->msgRef, startTime,
			monitor->pduData, PDU_REQUIRED_FLAG);
	notifMonitor->state = ENROUTE;
	notifMonitor->deliveryStatus = deliveryStatus;
	pduReg->registerMonitor(notifMonitor);
}

void NormalSmsHandler::registerDeliveryReceiptMonitor(const DeliveryMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime, time_t respTime)
{
	__require__(monitor && pduReg);
	__decl_tc__;
	DeliveryReceiptMonitor* m = pduReg->getDeliveryReceiptMonitor(monitor->msgRef);
	if (m)
	{
		//���� ��������� ������� �������� ��������� ���������� ����� validity_period,
		//�� ������� ����� ��� ����������, ��� � ��� � delivery report �����
		//��������������/������������������ � ����� �������
		//__require__(m->getFlag() == PDU_COND_REQUIRED_FLAG);
		if (m->getFlag() != PDU_COND_REQUIRED_FLAG)
		{
			__warning2__("registerDeliveryReceiptMonitor(): monitor = %p status is not cond required", m);
			return;
		}
		pduReg->removeMonitor(m);
		delete m;
	}
	//flag
	PduFlag flag;
	switch (monitor->getFlag())
	{
		case PDU_REQUIRED_FLAG:
			return;
		case PDU_MISSING_ON_TIME_FLAG:
			__unreachable__("not expected");
			break;
		case PDU_COND_REQUIRED_FLAG:
			flag = PDU_COND_REQUIRED_FLAG;
			break;
		case PDU_NOT_EXPECTED_FLAG:
			flag = PDU_REQUIRED_FLAG;
			break;
		default:
			__unreachable__("Invalid flag");
	}
	//regDelivery
	uint8_t regDelivery =
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData);
	switch (regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			return;
		case FINAL_SMSC_DELIVERY_RECEIPT:
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			if (deliveryStatus == ESME_ROK)
			{
				__tc__("deliverySm.reports.deliveryReceipt.failureDeliveryReceipt");
				__tc_ok__;
				return;
			}
			break;
		default:
			__unreachable__("Invalid regDelivery");
	}
	if (monitor->pduData->intProps.count("ussdServiceOp"))
	{
		__tc__("deliverySm.reports.deliveryReceipt.ussdServiceOp");
		__tc_ok__;
		return;
	}
	//startTime & state
	time_t startTime;
	State state;
	RespPduFlag respFlag = isAccepted(deliveryStatus);
	switch (respFlag)
	{
		case RESP_PDU_OK:
			startTime = respTime;
			state = DELIVERED;
			break;
		case RESP_PDU_ERROR:
			startTime = respTime;
			state = UNDELIVERABLE;
			break;
		case RESP_PDU_RESCHED:
			__tc__("deliverySm.reports.deliveryReceipt.expiredDeliveryReceipt");
			__tc_ok__;
			startTime = monitor->getValidTime();
			state = EXPIRED;
			break;
		case RESP_PDU_MISSING:
			__tc__("deliverySm.reports.deliveryReceipt.expiredDeliveryReceipt");
			__tc_ok__;
			startTime = max(recvTime + (time_t) (fixture->smeInfo.timeout - 1),
				monitor->getValidTime());
			state = EXPIRED;
			break;
		default:
			__unreachable__("Invalid respFlag");
	}
	//register
	DeliveryReceiptMonitor* rcptMonitor = new DeliveryReceiptMonitor(
		monitor->msgRef, startTime, monitor->pduData,flag);
	rcptMonitor->state = state;
	rcptMonitor->deliveryStatus = deliveryStatus;
	pduReg->registerMonitor(rcptMonitor);
}

void NormalSmsHandler::registerDeliveryReportMonitors(const DeliveryMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime, time_t respTime)
{
	registerIntermediateNotificationMonitor(monitor, pduReg, deliveryStatus,
		recvTime, respTime);
	registerDeliveryReceiptMonitor(monitor, pduReg, deliveryStatus,
		recvTime, respTime);
}

#define __compare__(failureCode, field) \
	if (pdu.field != origPdu->field) { \
		ostringstream s1, s2; \
		s1 << (pdu.field); \
		s2 << (origPdu->field); \
		__trace2__("%s: %s != %s", #field, s1.str().c_str(), s2.str().c_str()); \
		__tc_fail__(failureCode); \
	}

#define __compareCStr__(failureCode, field) \
	if ((pdu.field && !origPdu->field) || \
		(!pdu.field && origPdu->field) || \
		(pdu.field && origPdu->field && strcmp(pdu.field, origPdu->field))) { \
		__trace2__("%s: %s != %s", #field, pdu.field, origPdu->field); \
		__tc_fail__(failureCode); \
	}

void NormalSmsHandler::processPdu(PduDeliverySm& pdu, const Address origAddr,
	time_t recvTime)
{
	__trace__("processNormalSms()");
	__decl_tc__;
	try
	{
		__tc__("deliverySm.normalSms");
		//� ���������� pdu ��� user_message_reference
		if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(1);
			throw TCException();
		}
		//�������� pduReg ������
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(origAddr);
		__require__(pduReg);
		//�������� ������������ pdu
		MutexGuard mguard(pduReg->getMutex());
		DeliveryMonitor* monitor = pduReg->getDeliveryMonitor(
			pdu.get_optional().get_userMessageReference());
		//��� user_message_reference �� ���������� pdu
		//��� ���������������� ������������� pdu
		if (!monitor)
		{
			__tc_fail__(2);
			__trace2__("getDeliveryMonitor(): pduReg = %p, userMessageReference = %d, monitor = NULL",
				pduReg, pdu.get_optional().get_userMessageReference());
			throw TCException();
		}
		__tc_ok_cond__;
		__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* origPdu =
			reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
		//��������� ������������ ��������
		__tc__("deliverySm.normalSms.checkRoute");
		__tc_fail2__(checkRoute(*origPdu, pdu), 0);
		__tc_ok_cond__;
		//�������� ���� ���������� � ������������ pdu
		__tc__("deliverySm.normalSms.checkMandatoryFields");
		//���� ������ ����������� � processDeliverySm()
		//message
		__compareCStr__(1, get_message().get_serviceType());
		//������������ ������� ����������� � fixture->routeChecker->checkRouteForNormalSms()
		//__compareAddr__(get_message().get_source());
		//__compareAddr__(get_message().get_dest());
		__compare__(2, get_message().get_esmClass() & 0xfc); //��� 2-�� ������� �����
		if ((pdu.get_message().get_esmClass() & ESM_CLASS_MESSAGE_TYPE_BITS) !=
			ESM_CLASS_NORMAL_MESSAGE)
		{
			__tc_fail__(3);
		}
		__compare__(4, get_message().get_protocolId());
		//� ����������������, priority �������� ��������� �
		//������ �� ������� �������� ���������
		__compare__(5, get_message().get_priorityFlag());
		__compare__(6, get_message().get_registredDelivery());
		__tc_ok_cond__;
		//�������� �����
		if (fixture->profileReg)
		{
			compareMsgText(*origPdu, pdu, recvTime);
		}
		//optional
		__tc__("deliverySm.normalSms.checkOptionalFields");
		//��������� message_payload, ������� ����������� � compareMsgText()
		__tc_fail2__(SmppUtil::compareOptional(pdu.get_optional(),
			origPdu->get_optional(), OPT_MSG_PAYLOAD + OPT_RCPT_MSG_ID), 0);
		__tc_ok_cond__;
		//�������� ��������� ��������� ��������
		__tc__("deliverySm.normalSms.scheduleChecks");
		__tc_fail2__(monitor->checkSchedule(recvTime), 0);
		__tc_ok_cond__;
		//��������� �������
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendDeliverySmResp(pdu);
		RespPduFlag respFlag = isAccepted(deliveryResp.first);
		//�������� ������ delivery ��������
		__tc__("deliverySm.normalSms.checkAllowed");
		pduReg->removeMonitor(monitor);
		__tc_fail2__(monitor->update(recvTime, respFlag), 0);
		switch (respFlag)
		{
			case RESP_PDU_OK:
				monitor->state = DELIVERED;
				break;
			case RESP_PDU_ERROR:
				monitor->state = UNDELIVERABLE;
				break;
			case RESP_PDU_RESCHED:
			case RESP_PDU_MISSING:
				switch (monitor->getFlag())
				{
					case PDU_REQUIRED_FLAG:
					case PDU_COND_REQUIRED_FLAG:
						monitor->state = ENROUTE;
						break;
					case PDU_NOT_EXPECTED_FLAG:
						monitor->state = EXPIRED;
						break;
					default: //PDU_MISSING_ON_TIME_FLAG
						__unreachable__("Invalid flag");
				}
				break;
			default:
				__unreachable__("Invalid respFlag");
		}
		monitor->deliveryStatus = deliveryResp.first;
		//��� ussd ������������ ������� ��������
		if (monitor->pduData->intProps.count("ussdServiceOp"))
		{
			__tc__("deliverySm.normalSms.ussdServiceOp");
			__tc_ok__;
			monitor->setNotExpected();
		}
		pduReg->registerMonitor(monitor); //���� ����� �� ���������������� pdu
		__tc_ok_cond__;
		//���������������� delivery report ��������
		SmeType smeType = fixture->smeReg->getSmeBindType(origAddr);
		if (smeType != SME_TRANSMITTER)
		{
			__require__(smeType == SME_RECEIVER || smeType == SME_TRANSCEIVER);
			registerDeliveryReportMonitors(monitor, pduReg, deliveryResp.first,
				recvTime, deliveryResp.second);
		}
	}
	catch (TCException&)
	{
		//��������� �������
		fixture->respSender->sendDeliverySmResp(pdu);
	}
	catch (...)
	{
		__tc_fail__(100);
		error();
	}
}

}
}
}

