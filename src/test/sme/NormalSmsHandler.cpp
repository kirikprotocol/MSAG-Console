#include "NormalSmsHandler.hpp"
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
using namespace smsc::test::smpp;
using namespace smsc::test::core;
using namespace smsc::test::util;

Category& NormalSmsHandler::getLog()
{
	static Category& log = Logger::getCategory("NormalSmsHandler");
	return log;
}

void NormalSmsHandler::compareMsgText(PduSubmitSm& origPdu, PduDeliverySm& pdu)
{
	__require__(fixture->profileReg);
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	Address destAddr;
	SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
	time_t profileUpdateTime;
	int codePage = fixture->profileReg->getProfile(destAddr, profileUpdateTime).codepage;
	if (time(NULL) <= profileUpdateTime + timeCheckAccuracy)
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
			__tc__("processDeliverySm.normalSms.checkDataCoding");
			if (dc != DATA_CODING_SMSC_DEFAULT)
			{
				__tc_fail__(1);
			}
			__tc_ok_cond__;
			if (dc == DATA_CODING_SMSC_DEFAULT)
			{
				if (origDc == DATA_CODING_SMSC_DEFAULT)
				{
					__tc__("processDeliverySm.normalSms.checkTextEqualDataCoding");
				}
				else
				{
					__tc__("processDeliverySm.normalSms.checkTextDiffDataCoding");
				}
				__tc_fail2__(compare(origDc, origSm, origSmLen, dc, sm, smLen), 0);
				__tc_fail2__(compare(origDc, origMp, origMpLen, dc, mp, mpLen), 10);
				__tc_ok_cond__;
			}
			break;
		case ProfileCharsetOptions::Ucs2:
			if (dc == origDc)
			{
				__tc__("processDeliverySm.normalSms.checkTextEqualDataCoding");
			}
			else
			{
				__tc__("processDeliverySm.normalSms.checkTextDiffDataCoding");
			}
			__tc_fail2__(compare(origDc, origSm, origSmLen, dc, sm, smLen), 0);
			__tc_fail2__(compare(origDc, origMp, origMpLen, dc, mp, mpLen), 10);
			__tc_ok_cond__;
			break;
		default:
			__unreachable__("Invalid profile");
	}
}

void NormalSmsHandler::updateDeliveryReceiptMonitor(DeliveryMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime,
	uint32_t smeTimeout)
{
	__require__(monitor && pduReg);
	__decl_tc__;
	DeliveryReceiptMonitor* rcptMonitor = pduReg->getDeliveryReceiptMonitor(
		monitor->pduData->msgRef);
	__require__(rcptMonitor);
	__cfg_int__(timeCheckAccuracy);
	RespPduFlag respFlag = isAccepted(deliveryStatus);
	pduReg->removeMonitor(rcptMonitor);
	switch (rcptMonitor->regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			__require__(rcptMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
			break;
		case FINAL_SMSC_DELIVERY_RECEIPT:
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			switch (monitor->getFlag())
			{
				case PDU_REQUIRED_FLAG:
					rcptMonitor->reschedule(monitor->getCheckTime());
					break;
				case PDU_MISSING_ON_TIME_FLAG:
					__unreachable__("Invalid monitor state");
					//break;
				case PDU_RECEIVED_FLAG:
					if (rcptMonitor->regDelivery == FAILURE_SMSC_DELIVERY_RECEIPT)
					{
						__tc__("processDeliverySm.deliveryReceipt.failureDeliveryReceipt");
						__tc_ok__;
						rcptMonitor->setNotExpected();
					}
					else
					{
						__require__(rcptMonitor->regDelivery == FINAL_SMSC_DELIVERY_RECEIPT);
						rcptMonitor->reschedule(recvTime);
					}
					break;
				case PDU_ERROR_FLAG:
					rcptMonitor->reschedule(recvTime);
					break;
				case PDU_NOT_EXPECTED_FLAG:
					__require__(rcptMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
				case PDU_EXPIRED_FLAG:
					__tc__("processDeliverySm.deliveryReceipt.expiredDeliveryReceipt");
					__tc_ok__;
					rcptMonitor->reschedule(max(monitor->getValidTime(),
						recvTime + (time_t) smeTimeout - (time_t) timeCheckAccuracy));
					break;
				default:
					__unreachable__("Invalid monitor flag");
			}
			break;
		default:
			__unreachable__("Invalid registered delivery flag");
	}
	rcptMonitor->deliveryFlag = monitor->getFlag();
	rcptMonitor->deliveryStatus = deliveryStatus;
	pduReg->registerMonitor(rcptMonitor);
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
		__tc__("processDeliverySm.normalSms");
		//� ���������� pdu ��� user_message_reference
		if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(1);
			throw TCException();
		}
		//�������� pduReg ������
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(origAddr);
		const SmeInfo* smeInfo = fixture->smeReg->getSme(origAddr);
		__require__(pduReg);
		__require__(smeInfo);
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
		//� ������ ������� delivery_sm �������� ������ submit_sm_resp
		//����� ��� �� ����������
		if (!monitor->pduData->valid)
		{
			__tc_fail__(3);
			//throw TCException();
		}
		__tc_ok_cond__;
		__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* origPdu =
			reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
		//��������� ������������ ��������
		__tc__("processDeliverySm.normalSms.checkRoute");
		__tc_fail2__(fixture->routeChecker->checkRouteForNormalSms(*origPdu, pdu), 0);
		__tc_ok_cond__;
		//�������� ���� ���������� � ������������ pdu
		__tc__("processDeliverySm.normalSms.checkMandatoryFields");
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
			compareMsgText(*origPdu, pdu);
		}
		//optional
		__tc__("processDeliverySm.normalSms.checkOptionalFields");
		//��������� message_payload, ������� ����������� � compareMsgText()
		__tc_fail2__(SmppUtil::compareOptional(
			pdu.get_optional(), origPdu->get_optional(), OPT_MSG_PAYLOAD), 0);
		__tc_ok_cond__;
		//�������� ��������� ��������� ��������
		__tc__("processDeliverySm.normalSms.scheduleChecks");
		__tc_fail2__(monitor->checkSchedule(recvTime), 0);
		__tc_ok_cond__;
		//��������� �������
		uint32_t deliveryStatus =
			fixture->respSender->sendDeliverySmResp(pdu);
		RespPduFlag respFlag = isAccepted(deliveryStatus);
		//�������� ������ delivery ��������
		__tc__("processDeliverySm.normalSms.checkAllowed");
		pduReg->removeMonitor(monitor);
		__tc_fail2__(monitor->update(recvTime, respFlag), 0);
		pduReg->registerMonitor(monitor);
		__tc_ok_cond__;
		//�������� ������ delivery receipt ��������
		updateDeliveryReceiptMonitor(monitor, pduReg, deliveryStatus, recvTime,
			smeInfo->timeout);
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

