#include "DeliveryReportHandler.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "profiler/profiler.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::sms::Address;
using smsc::core::synchronization::MutexGuard;
using smsc::test::sms::operator!=;
using smsc::test::conf::TestConfig;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::smpp::DataCoding;
using namespace smsc::profiler;
using namespace smsc::test::core;
using namespace smsc::test::smpp; //SmppUtil, constants
using namespace smsc::test::util;

DeliveryReportHandler::DeliveryReportHandler(SmppFixture* _fixture,
	const Address& _smeAddr, const Address& _smeAlias,
	const SmeSystemId& _smeServiceType, uint8_t _smeProtocolId)
: fixture(_fixture), chkList(_fixture->chkList), smeAddr(_smeAddr),
	smeAlias(_smeAlias), smeServiceType(_smeServiceType),
	smeProtocolId(_smeProtocolId) {}

vector<int> DeliveryReportHandler::checkRoute(SmppHeader* header1,
	SmppHeader* header2) const
{
	__require__(header1 && header2);
	vector<int> res;
	SmsPduWrapper pdu1(header1, 0);
	SmsPduWrapper pdu2(header2, 0);
	Address origAddr1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.getSource(), &origAddr1);
	SmppUtil::convert(pdu2.getSource(), &origAlias2);
	SmppUtil::convert(pdu2.getDest(), &destAddr2);
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

#define __compare__(errCode, field, value) \
	if (value != field) { __tc_fail__(errCode); }

void DeliveryReportHandler::processPdu(SmppHeader* header, time_t recvTime)
{
	__trace__("processDeliveryReport()");
	__decl_tc__;
	static const char UNKNOWN = ' ';
	static const char DELIVERY_RECEIPT = '*';
	static const char INTERMEDIATE_NOTIFICATION = '$';
	static const char SMS_CANCELLED_NOTIFICATION = '#';
	try
	{
		SmsPduWrapper pdu(header, 0);
		__tc__("sms.reports.checkDeliverSm");
		__compare__(1, pdu.isDeliverSm(), true);
		__tc_ok_cond__;
		//перекрыть pduReg класса
		Address destAddr;
		SmppUtil::convert(pdu.getDest(), &destAddr);
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(destAddr);
		__require__(pduReg);
		//тип pdu
		char pduType = UNKNOWN;
		switch (pdu.getDataCoding())
		{
			case DEFAULT:
			case SMSC7BIT:
				if (pdu.isDeliverSm() && pdu.get_message().size_shortMessage() > 0)
				{
					pduType = *pdu.get_message().get_shortMessage();
				}
				break;
			case UCS2:
				if (pdu.isDeliverSm() && pdu.get_message().size_shortMessage() > 1);
				{
					short tmp;
					memcpy(&tmp, pdu.get_message().get_shortMessage(), 2);
					pduType = ntohs(tmp);
				}
				break;
			default:
				__unreachable__("Invalid delivery report dataCoding");
		}
		switch (pduType)
		{
			case DELIVERY_RECEIPT:
				__tc__("sms.reports.deliveryReceipt.checkAllowed");
				break;
			case INTERMEDIATE_NOTIFICATION:
				__tc__("sms.reports.intermediateNotification.checkAllowed");
				break;
			case SMS_CANCELLED_NOTIFICATION:
				//sms deleted не тестирую
				__tc__("sms.reports.smsCancelledNotification");
				__tc_fail__(-1);
				return;
			default:
				__unreachable__("Invalid pdu type");
		}
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		DeliveryReportMonitor* monitor = NULL;
		switch (pduType)
		{
			case DELIVERY_RECEIPT:
				monitor = pduReg->getDeliveryReceiptMonitor(pdu.getMsgRef());
				break;
			case INTERMEDIATE_NOTIFICATION:
				monitor = pduReg->getIntermediateNotificationMonitor(pdu.getMsgRef());
				break;
			default:
				monitor = NULL;
		}
		//для user_message_reference из полученной pdu
		//нет соответствующего оригинального pdu
		if (!monitor)
		{
			__tc_fail__(1);
			__trace2__("getDeliveryReportMonitor(): pduReg = %p, userMessageReference = %d, pduType = %c, monitor = NULL",
				pduReg, pdu.getMsgRef(), pduType);
			throw TCException();
		}
		switch (monitor->getFlag())
		{
			case PDU_REQUIRED_FLAG:
			case PDU_MISSING_ON_TIME_FLAG:
			case PDU_COND_REQUIRED_FLAG:
				//ok
				break;
			case PDU_NOT_EXPECTED_FLAG:
				__tc_fail__(2);
				throw TCException();
			default:
				__unreachable__("Unknown flag");
		}
		__tc_ok_cond__;
		SmsPduWrapper origPdu(monitor->pduData->pdu, 0);
		//Сравнить правильность маршрута
		__tc__("sms.reports.checkRoute");
		__tc_fail2__(checkRoute(monitor->pduData->pdu, header), 0);
		__tc_ok_cond__;
		//проверить содержимое полученной pdu
		__tc__("sms.reports.checkFields");
		//поля header проверяются в processDeliverySm()
		//поля message проверяются в processDeliveryReport()
		//правильность адресов проверяется в checkRoute()
		__compare__(1, nvl(pdu.getServiceType()), smeServiceType);
		Address srcAlias;
		SmppUtil::convert(pdu.getSource(), &srcAlias);
		if (fixture->smeInfo.wantAlias && srcAlias != smeAlias)
		{
			__tc_fail__(2);
		}
		else if (!fixture->smeInfo.wantAlias && srcAlias != smeAddr)
		{
			__tc_fail__(3);
		}
		//__compare__(4, pdu.getDest(), origPdu.getSource());
		bool statusReport;
		switch (origPdu.getRegistredDelivery() & SMSC_DELIVERY_RECEIPT_BITS)
		{
			case NO_SMSC_DELIVERY_RECEIPT:
			case SMSC_DELIVERY_RECEIPT_RESERVED:
				statusReport = false;
				break;
			case FAILURE_SMSC_DELIVERY_RECEIPT:
				statusReport = monitor->deliveryStatus != ESME_ROK;
				break;
			case FINAL_SMSC_DELIVERY_RECEIPT:
				statusReport = true;
				break;
			default:
				__unreachable__("Invalid reg dilivery");
		}
		__require__(monitor->pduData->objProps.count("senderData"));
		SenderData* senderData =
			dynamic_cast<SenderData*>(monitor->pduData->objProps["senderData"]);
		__require__(senderData->validProfile);
		if (statusReport)
		{
			if (pduType == DELIVERY_RECEIPT)
			{
				__compare__(4, pdu.getEsmClass(), ESM_CLASS_DELIVERY_RECEIPT);
			}
			else
			{
				__require__(pduType == INTERMEDIATE_NOTIFICATION);
				__compare__(5, pdu.getEsmClass(), ESM_CLASS_INTERMEDIATE_NOTIFICATION);
			}
		}
		else if (senderData->profile.reportoptions == ProfileReportOptions::ReportFull)
		{
			__compare__(6, pdu.getEsmClass(), ESM_CLASS_NORMAL_MESSAGE);
		}
		else
		{
			__tc_fail__(7);
		}
		if (pdu.isDeliverSm())
		{
			__compare__(8, pdu.get_message().get_protocolId(), smeProtocolId);
			__compare__(9, pdu.get_message().get_priorityFlag(), 0);
		}
		__compare__(10, pdu.getRegistredDelivery(), 0);
		__tc_ok_cond__;
		//для delivery report не проверяю повторную доставку
		if (pduType == DELIVERY_RECEIPT)
		{
			__tc__("sms.reports.deliveryReceipt.recvTimeChecks");
		}
		else
		{
			__require__(pduType == INTERMEDIATE_NOTIFICATION);
			__tc__("sms.reports.intermediateNotification.recvTimeChecks");
		}
		__cfg_int__(timeCheckAccuracy);
		if (recvTime < monitor->getCheckTime())
		{
			__trace2__("expected time = %ld", monitor->getCheckTime());
			__tc_fail__(1);
		}
		else if (recvTime > monitor->getCheckTime() + timeCheckAccuracy)
		{
			__trace2__("expected time = %ld", monitor->getCheckTime());
			__tc_fail__(2);
		}
		__tc_ok_cond__;
		//проверка полей pdu
		pduReg->removeMonitor(monitor);
		if (pduType == DELIVERY_RECEIPT)
		{
			processDeliveryReceipt(dynamic_cast<DeliveryReceiptMonitor*>(monitor),
				header, recvTime);
		}
		else
		{
			__require__(pduType == INTERMEDIATE_NOTIFICATION);
			processIntermediateNotification(
				dynamic_cast<IntermediateNotificationMonitor*>(monitor),
				header, recvTime);
		}
		pduReg->registerMonitor(monitor); //тест кейсы на финализированные pdu
		//отправить респонс, только ESME_ROK разрешено
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendSmsResp(header);
		__require__(deliveryResp.first == ESME_ROK);
	}
	catch (TCException&)
	{
		//отправить респонс, только ESME_ROK разрешено
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendSmsResp(header);
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

