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

SmeAcknowledgementHandler::SmeAcknowledgementHandler(SmppFixture* _fixture,
	const string& _smeServiceType, uint8_t _smeProtocolId)
: fixture(_fixture), chkList(_fixture->chkList), smeServiceType(_smeServiceType),
	smeProtocolId(_smeProtocolId) {}

SmeAcknowledgementHandler::~SmeAcknowledgementHandler()
{
	for (int i = 0; i < smeAddr.size(); i++) { delete smeAddr[i]; }
	for (int i = 0; i < smeAlias.size(); i++) { delete smeAlias[i]; }
}

void SmeAcknowledgementHandler::addSmeAddr(const Address& addr)
{
	smeAddr.push_back(new Address(addr));
}

void SmeAcknowledgementHandler::addSmeAlias(const Address& alias)
{
	smeAlias.push_back(new Address(alias));
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
	if (!fixture->smeInfo.wantAlias)
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
	else if (fixture->smeInfo.systemId != routeHolder->route.smeSystemId)
	{
		res.push_back(5);
	}
	return res;
}

#define __compare__(errCode, field, value) \
	if (value != field) { __tc_fail__(errCode); }

void SmeAcknowledgementHandler::processPdu(PduDeliverySm& pdu, time_t recvTime)
{
	__trace__("processSmeAcknowledgement()");
	__decl_tc__;
	try
	{
		__tc__("deliverySm.smeAck");
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
		__tc_ok_cond__;
		__tc__("deliverySm.smeAck.checkAllowed");
		switch (monitor->getFlag())
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
			case PDU_COND_REQUIRED_FLAG:
				//ok
				break;
			case PDU_NOT_EXPECTED_FLAG:
				__tc_fail__(1);
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
		__tc__("deliverySm.smeAck.checkRoute");
		__tc_fail2__(checkRoute(*origPdu, pdu), 0);
		__tc_ok_cond__;
		//проверить содержимое полученной pdu
		__tc__("deliverySm.smeAck.checkFields");
		//поля header проверяются в processDeliverySm()
		//поля message проверяются в ackHandler->processSmeAcknowledgement()
		//правильность адресов проверяется в checkRoute()
		__compare__(1, nvl(pdu.get_message().get_serviceType()), smeServiceType);
		__compare__(2, pdu.get_message().get_esmClass(), ESM_CLASS_NORMAL_MESSAGE);
		Address srcAlias;
		SmppUtil::convert(pdu.get_message().get_source(), &srcAlias);
		bool addrOk = false;
		if (fixture->smeInfo.wantAlias)
		{
			for (int i = 0 ; i < smeAlias.size(); i++)
			{
				if (srcAlias == *smeAlias[i])
				{
					addrOk = true;
					break;
				}
			}
		}
		else
		{
			for (int i = 0 ; i < smeAddr.size(); i++)
			{
				if (srcAlias == *smeAddr[i])
				{
					addrOk = true;
					break;
				}
			}
		}
		if (!addrOk)
		{
			__tc_fail__(3);
		}
		__compare__(4, pdu.get_message().get_protocolId(), smeProtocolId);
		__compare__(5, pdu.get_message().get_priorityFlag(), 0);
		__compare__(6, pdu.get_message().get_registredDelivery(), 0);
		__tc_ok_cond__;
		//для sme acknoledgement не проверяю повторную доставку
		__tc__("deliverySm.smeAck.recvTimeChecks");
		__cfg_int__(timeCheckAccuracy);
		if (recvTime < monitor->getCheckTime())
		{
			__tc_fail__(1);
		}
		else if (recvTime > monitor->getCheckTime() + timeCheckAccuracy)
		{
			__tc_fail__(2);
		}
		__tc_ok_cond__;
		pduReg->removeMonitor(monitor);
		processSmeAcknowledgement(monitor, pdu, recvTime);
		pduReg->registerMonitor(monitor); //тест кейсы на финализированные pdu
		//отправить респонс, только ESME_ROK разрешено
		//отправка должна быть без задержек
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendDeliverySmResp(pdu);
		__require__(deliveryResp.first == ESME_ROK);
		__require__(abs(deliveryResp.second - time(NULL)) <= 1);
	}
	catch (TCException&)
	{
		//отправить респонс, только ESME_ROK разрешено
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendDeliverySmResp(pdu);
		__require__(deliveryResp.first == ESME_ROK);
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

