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

vector<int> SmeAcknowledgementHandler::checkRoute(SmppHeader* header1,
	SmppHeader* header2) const
{
	__require__(header1 && header2);
	vector<int> res;
	SmsPduWrapper pdu1(header1, 0);
	SmsPduWrapper pdu2(header2, 0);
	Address origAddr1, destAlias1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.getSource(), &origAddr1);
	SmppUtil::convert(pdu1.getDest(), &destAlias1);
	SmppUtil::convert(pdu2.getSource(), &origAlias2);
	SmppUtil::convert(pdu2.getDest(), &destAddr2);
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

#define __check__(errCode, cond) \
	if (!(cond)) { __tc_fail__(errCode); }

void SmeAcknowledgementHandler::processPdu(SmppHeader* header, time_t recvTime)
{
	__trace__("processSmeAcknowledgement()");
	__decl_tc__;
	try
	{
		SmsPduWrapper pdu(header, 0);
		__tc__("sms.smeAck.checkDeliverSm");
		__check__(1, pdu.isDeliverSm());
		__tc_ok_cond__;
		Address destAddr;
		SmppUtil::convert(pdu.getDest(), &destAddr);
		//перкрыть pduReg класса
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(destAddr);
		__require__(pduReg);
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		SmeAckMonitor* monitor = pduReg->getSmeAckMonitor(pdu.getMsgRef());
		//для user_message_reference из полученной pdu
		//нет соответствующего оригинального pdu
		if (!monitor)
		{
			__tc_fail__(2);
			__trace2__("getSmeAckMonitor(): pduReg = %p, userMessageReference = %d, monitor = NULL",
				pduReg, (int) pdu.getMsgRef());
			throw TCException();
		}
		__tc_ok_cond__;
		__tc__("sms.smeAck.checkAllowed");
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
		SmsPduWrapper origPdu(monitor->pduData->pdu, 0);
		//правильность маршрута
		__tc__("sms.smeAck.checkRoute");
		__tc_fail2__(checkRoute(monitor->pduData->pdu, header), 0);
		__tc_ok_cond__;
		//проверить содержимое полученной pdu
		__tc__("sms.smeAck.checkFields");
		//поля header проверяются в processDeliverySm()
		//поля message проверяются в ackHandler->processSmeAcknowledgement()
		//правильность адресов проверяется в checkRoute()
		__check__(1, smeServiceType == nvl(pdu.getServiceType()));
		__check__(2, pdu.getEsmClass() == ESM_CLASS_NORMAL_MESSAGE);
		Address srcAlias;
		SmppUtil::convert(pdu.getSource(), &srcAlias);
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
		if (pdu.isDeliverSm())
		{
			__check__(4, pdu.get_message().get_protocolId() == smeProtocolId);
			__check__(5, pdu.get_message().get_priorityFlag() == 0);
		}
		__check__(6, pdu.getRegistredDelivery() == 0);
		__tc_ok_cond__;
		//для sme acknoledgement не проверяю повторную доставку
		__tc__("sms.smeAck.recvTimeChecks");
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
		processSmeAcknowledgement(monitor, header, recvTime);
		pduReg->registerMonitor(monitor); //тест кейсы на финализированные pdu
		//отправить респонс, только ESME_ROK разрешено
		//отправка должна быть без задержек
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendSmsResp(header);
		__require__(deliveryResp.first == ESME_ROK);
		__require__(abs(deliveryResp.second - time(NULL)) <= 1);
	}
	catch (TCException&)
	{
		//отправить респонс, только ESME_ROK разрешено
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendSmsResp(header);
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

