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

SmeAcknowledgementHandler::SmeAcknowledgementHandler(SmppFixture* _fixture)
: fixture(_fixture), chkList(_fixture->chkList)
{
	sme = fixture->smeReg->getSme(fixture->systemId);
	__require__(sme);
}

vector<int> SmeAcknowledgementHandler::checkRoute(PduSubmitSm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, destAlias1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAlias1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//правильность destAddr для pdu2
	if (destAddr2 != origAddr1)
	{
		res.push_back(1);
	}
	//правильность origAddr для pdu2
	const RouteHolder* routeHolder = NULL;
	if (!sme->wantAlias)
	{
		const Address destAddr = fixture->aliasReg->findAddressByAlias(destAlias1);
		if (origAlias2 != destAddr)
		{
			res.push_back(2);
		}
		else
		{
			routeHolder = fixture->routeReg->lookup(origAlias2, destAddr2);
		}
	}
	else if (origAlias2 != destAlias1)
	{
		res.push_back(3);
	}
	else
	{
		const Address origAddr2 = fixture->aliasReg->findAddressByAlias(origAlias2);
		routeHolder = fixture->routeReg->lookup(origAddr2, destAddr2);
	}
	//правильность маршрута
	if (!routeHolder)
	{
		res.push_back(4);
	}
	else if (fixture->systemId != routeHolder->route.smeSystemId)
	{
		res.push_back(5);
	}
	return res;
}

void SmeAcknowledgementHandler::updateDeliveryReceiptMonitor(SmeAckMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime)
{
	__require__(monitor && pduReg);
	__require__(deliveryStatus == ESME_ROK);
	__decl_tc__;
	DeliveryReceiptMonitor* rcptMonitor = pduReg->getDeliveryReceiptMonitor(
		monitor->pduData->msgRef);
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
			__trace2__("getSmeAckMonitor(): pduReg = %p, userMessageReference = %d, monitor = NULL",
				pduReg, pdu.get_optional().get_userMessageReference());
			throw TCException();
		}
		//в редких случаях sme ack приходит раньше submit_sm_resp
		if (!monitor->pduData->valid)
		{
			__tc_fail__(3);
			//throw TCException();
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
		__tc_fail2__(checkRoute(*origPdu, pdu), 0);
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

