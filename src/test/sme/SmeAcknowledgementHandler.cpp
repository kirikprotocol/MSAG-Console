#include "SmeAcknowledgementHandler.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/core/PduUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::test::conf::TestConfig;
using smsc::test::core::DeliveryReceiptMonitor;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::test::smpp;
using namespace smsc::test::core;
using namespace smsc::test::util;

void SmeAcknowledgementHandler::updateDeliveryReceiptMonitor(SmeAckMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime)
{
	__require__(monitor && pduReg);
	__require__(deliveryStatus == ESME_ROK);
	__decl_tc__;
	DeliveryReceiptMonitor* rcptMonitor = pduReg->getDeliveryReceiptMonitor(
		monitor->pduData->msgRef, monitor->pduData);
	__require__(rcptMonitor);
	//проверить если delivery receipt пришел раньше sme ack
	if (rcptMonitor->getFlag() == PDU_RECEIVED_FLAG)
	{
		return;
	}
	pduReg->removeMonitor(rcptMonitor);
	switch (rcptMonitor->regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			__require__(rcptMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
			break;
		case FINAL_SMSC_DELIVERY_RECEIPT:
			rcptMonitor->reschedule(recvTime);
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			rcptMonitor->setNotExpected();
			break;
		default:
			__unreachable__("Invalid registered delivery flag");
	}
	rcptMonitor->deliveryFlag = monitor->getFlag();
	rcptMonitor->deliveryStatus = deliveryStatus;
	pduReg->registerMonitor(rcptMonitor);
}

#define __compare__(errCode, field, value) \
	if (value != pdu.field) { __tc_fail__(errCode); }

void SmeAcknowledgementHandler::processPdu(PduDeliverySm& pdu, time_t recvTime)
{
	__trace__("processSmeAcknowledgement()");
	__decl_tc__;
	try
	{
		__tc__("processDeliverySm.smeAck");
		//обязательные для sme acknowledgement опциональные поля
		if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(1);
			throw TCException();
		}
		Address destAddr;
		SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
		//перкрыть pduReg класса
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(destAddr);
		__require__(pduReg);
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		SmeAckMonitor* monitor = pduReg->getSmeAckMonitor(
			pdu.get_optional().get_userMessageReference());
		//для user_message_reference из полученной pdu
		//нет соответствующего оригинального pdu
		if (!monitor)
		{
			__tc_fail__(2);
			throw TCException();
		}
		if (!monitor->pduData->valid)
		{
			__tc_fail__(3);
			throw TCException();
		}
		__tc_ok_cond__;
		__tc__("processDeliverySm.smeAck.checkAllowed");
		switch (monitor->getFlag())
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				//ok
				break;
			case PDU_RECEIVED_FLAG:
				__tc_fail__(1);
				throw TCException();
			case PDU_NOT_EXPECTED_FLAG:
			case PDU_EXPIRED_FLAG:
			case PDU_ERROR_FLAG:
				__tc_fail__(2);
				throw TCException();
			default:
				__unreachable__("Unknown flag");
		}
		__tc_ok_cond__;
		//проверка pdu
		__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* origPdu =
			reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
		//правильность маршрута
		__tc__("processDeliverySm.smeAck.checkRoute");
		__tc_fail2__(fixture->routeChecker->checkRouteForAcknowledgementSms(
			*origPdu, pdu), 0);
		__tc_ok_cond__;
		//проверить содержимое полученной pdu
		__tc__("processDeliverySm.smeAck.checkFields");
		//поля header проверяются в processDeliverySm()
		//поля message проверяются в ackHandler->processSmeAcknowledgement()
		//правильность адресов проверяется в fixture->routeChecker->checkRouteForAcknowledgementSms()
		__compare__(1, get_message().get_esmClass(), ESM_CLASS_NORMAL_MESSAGE);
		__tc_ok_cond__;
		pduReg->removeMonitor(monitor);
		processSmeAcknowledgement(monitor, pdu, recvTime);
		pduReg->registerMonitor(monitor);
		//для sme acknoledgement не проверяю повторную доставку
		__tc__("processDeliverySm.smeAck.recvTimeChecks");
		__cfg_int__(timeCheckAccuracy);
		if (recvTime < monitor->startTime)
		{
			__tc_fail__(1);
		}
		else if (recvTime > monitor->startTime + timeCheckAccuracy)
		{
			__tc_fail__(2);
		}
		__tc_ok_cond__;
		//отправить респонс, только ESME_ROK разрешено
		uint32_t deliveryStatus = fixture->respSender->sendDeliverySmResp(pdu);
		__require__(deliveryStatus == ESME_ROK);
		RespPduFlag respFlag = isAccepted(deliveryStatus);
		//обновить статус delivery receipt монитора
		updateDeliveryReceiptMonitor(monitor, pduReg, deliveryStatus, recvTime);
	}
	catch (TCException&)
	{
		//отправить респонс, только ESME_ROK разрешено
		uint32_t deliveryStatus = fixture->respSender->sendDeliverySmResp(pdu);
		__require__(deliveryStatus == ESME_ROK);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

}
}
}

