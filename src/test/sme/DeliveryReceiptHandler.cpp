#include "DeliveryReceiptHandler.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::sms::Address;
using smsc::core::synchronization::MutexGuard;
using smsc::test::sms::operator!=;
using smsc::test::conf::TestConfig;
using smsc::test::smpp::SmppUtil;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::core;
using namespace smsc::test::util;

DeliveryReceiptHandler::DeliveryReceiptHandler(SmppFixture* _fixture)
: fixture(_fixture), chkList(_fixture->chkList) {}

vector<int> DeliveryReceiptHandler::checkRoute(PduSubmitSm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//правильность destAddr для pdu2
	if (destAddr2 != origAddr1)
	{
		res.push_back(1);
	}
	//правильность маршрута
	const RouteHolder* routeHolder = NULL;
	if (fixture->smeInfo.wantAlias)
	{
		const Address origAddr2 = fixture->aliasReg->findAddressByAlias(origAlias2);
		routeHolder = fixture->routeReg->lookup(origAddr2, destAddr2);
	}
	else
	{
		routeHolder = fixture->routeReg->lookup(origAlias2, destAddr2);
	}
	if (!routeHolder)
	{
		res.push_back(3);
	}
	else if (fixture->smeInfo.systemId != routeHolder->route.smeSystemId)
	{
		res.push_back(4);
	}
	return res;
}

void DeliveryReceiptHandler::processPdu(PduDeliverySm& pdu, time_t recvTime)
{
	__trace__("processDeliveryReceipt()");
	__decl_tc__;
	try
	{
		__tc__("processDeliverySm.deliveryReceipt");
		//обязательные для delivery receipt опциональные поля
		if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(1);
			throw TCException();
		}
		//перекрыть pduReg класса
		Address destAddr;
		SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(destAddr);
		__require__(pduReg);
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		DeliveryReceiptMonitor* monitor = pduReg->getDeliveryReceiptMonitor(
			pdu.get_optional().get_userMessageReference());
		//для user_message_reference из полученной pdu
		//нет соответствующего оригинального pdu
		if (!monitor)
		{
			__tc_fail__(2);
			__trace2__("getDeliveryReceiptMonitor(): pduReg = %p, userMessageReference = %d, monitor = NULL",
				pduReg, pdu.get_optional().get_userMessageReference());
			throw TCException();
		}
		//в редких случаях delivery receipt приходит раньше submit_sm_resp
		if (!monitor->pduData->valid)
		{
			__tc_fail__(3);
			//throw TCException();
		}
		__tc_ok_cond__;
		__tc__("processDeliverySm.deliveryReceipt.checkAllowed");
		switch (monitor->getFlag())
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
				//ok
				break;
			case PDU_RECEIVED_FLAG:
				if (!monitor->pduData->intProps.count("skipReceivedCheck"))
				{
					__tc_fail__(1);
				}
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
		__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* origPdu =
			reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
		__require__(origPdu);
		//Сравнить правильность маршрута
		__tc__("processDeliverySm.deliveryReceipt.checkRoute");
		__tc_fail2__(checkRoute(*origPdu, pdu), 0);
		__tc_ok_cond__;
		//проверка полей pdu
		pduReg->removeMonitor(monitor);
		processDeliveryReceipt(monitor, pdu, recvTime);
		pduReg->registerMonitor(monitor);
		//для delivery receipt не проверяю повторную доставку
		__tc__("processDeliverySm.deliveryReceipt.recvTimeChecks");
		__cfg_int__(timeCheckAccuracy);
		if (recvTime < monitor->getStartTime())
		{
			__trace2__("expected time = %ld", monitor->getStartTime());
			__tc_fail__(1);
		}
		else if (recvTime > monitor->getStartTime() + timeCheckAccuracy)
		{
			__trace2__("expected time = %ld", monitor->getStartTime());
			__tc_fail__(2);
		}
		__tc_ok_cond__;
		//отправить респонс, только ESME_ROK разрешено
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendDeliverySmResp(pdu);
		__require__(deliveryResp.first == ESME_ROK);
		RespPduFlag respFlag = isAccepted(deliveryResp.first);
	}
	catch (TCException&)
	{
		//отправить респонс, только ESME_ROK разрешено
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendDeliverySmResp(pdu);
		__require__(deliveryResp.first == ESME_ROK);
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

