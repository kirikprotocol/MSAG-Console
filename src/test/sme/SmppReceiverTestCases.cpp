#include "SmppReceiverTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::sme::SmppTransmitter;
using smsc::test::conf::TestConfig;
using namespace smsc::profiler;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::util;
using namespace smsc::test::core; //constants
using namespace smsc::test::smpp; //constants, SmppUtil

Category& SmppReceiverTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppReceiverTestCases");
	return log;
}

void SmppReceiverTestCases::processSubmitSmResp(PduSubmitSmResp &pdu)
{
	__dumpPdu__("processSubmitSmRespBefore", fixture->smeInfo.systemId, &pdu);
	time_t respTime = time(NULL);
	if (!fixture->pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("processSubmitSmResp.async");
	try
	{
		//получить оригинальную pdu
		MutexGuard mguard(fixture->pduReg->getMutex());
		ResponseMonitor* monitor = fixture->pduReg->getResponseMonitor(
			pdu.get_header().get_sequenceNumber());
		//для sequence number из респонса нет соответствующего pdu
		if (!monitor)
		{
			__tc_fail__(1);
		}
		else
		{
			//проверить и обновить response и delivery receipt мониторы
			//по данным из респонса
			fixture->pduChecker->processSubmitSmResp(monitor, pdu, respTime);
			//__dumpPdu__("processSubmitSmRespAfter", fixture->smeInfo.systemId, &pdu);
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processReplaceSmResp(PduReplaceSmResp &pdu)
{
	__dumpPdu__("processReplaceSmRespBefore", fixture->smeInfo.systemId, &pdu);
	time_t respTime = time(NULL);
	if (!fixture->pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("processReplaceSmResp.async");
	try
	{
		//получить оригинальную pdu
		MutexGuard mguard(fixture->pduReg->getMutex());
		ResponseMonitor* monitor = fixture->pduReg->getResponseMonitor(
			pdu.get_header().get_sequenceNumber());
		//для sequence number из респонса нет соответствующего pdu
		if (!monitor)
		{
			__tc_fail__(1);
		}
		else
		{
			//проверить и обновить response и delivery receipt мониторы
			//по данным из респонса
			fixture->pduChecker->processReplaceSmResp(monitor, pdu, respTime);
			//__dumpPdu__("processReplaceSmRespAfter", fixture->smeInfo.systemId, &pdu);
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

#define __checkForNull__(failureCode, field) \
	if (pdu.field) { __tc_fail__(failureCode); }

void SmppReceiverTestCases::processDeliverySm(PduDeliverySm &pdu)
{
	__dumpPdu__("processDeliverySmBefore", fixture->smeInfo.systemId, &pdu);
	__require__(fixture->session);
	__decl_tc__;
	__tc__("processDeliverySm.checkFields");
	time_t recvTime = time(NULL);
	//общая проверка полей
	//header
	//pdu.get_header().get_commandLength()
	if (pdu.get_header().get_commandId() != DELIVERY_SM)
	{
		__tc_fail__(1);
	}
	if (pdu.get_header().get_commandStatus())
	{
		__tc_fail__(2);
	}
	//pdu.get_header().get_sequenceNumber()
	//message
	__checkForNull__(3, get_message().get_scheduleDeliveryTime());
	__checkForNull__(4, get_message().get_validityPeriod());
	__checkForNull__(5, get_message().get_replaceIfPresentFlag());
	__checkForNull__(6, get_message().get_smDefaultMsgId());
	__tc_ok_cond__;
	//обработать deliver_sm pdu
	if (fixture->pduReg)
	{
		Address origAlias;
		SmppUtil::convert(pdu.get_message().get_source(), &origAlias);
		const Address origAddr = fixture->aliasReg->findAddressByAlias(origAlias);
		//проверить тип sme
		PduHandlerMap::iterator it = fixture->pduHandler.find(origAddr);
		if (it == fixture->pduHandler.end())
		{
			defaultHandler.processPdu(pdu, origAddr, recvTime);
		}
		else
		{
			PduHandler* handler = it->second;
			__require__(handler);
			handler->processPdu(pdu, recvTime);
		}
	}
	else //отправить респонс
	{
		fixture->respSender->sendDeliverySmResp(pdu);
	}
	//__dumpPdu__("processDeliverySmAfter", fixture->smeInfo.systemId, &pdu);
}

/*
void SmppReceiverTestCases::processIntermediateNotification(
	PduDeliverySm &pdu, time_t recvTime)
{
	__trace__("processIntermediateNotification()");
	__decl_tc__;
	try
	{
		__tc__("processDeliverySm.intermediateNotification");
		//найти pduReg
		Address destAddr;
		SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(destAddr);
		if (!pduReg)
		{
			__trace2__("processIntermediateNotification(): pduReg not found for addr = %s", str(destAddr).c_str());
			__tc_fail__(1);
		}
		//обязательные для delivery receipt опциональные поля
		else if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(2);
		}
		else if (!pdu.get_optional().has_messageState())
		{
			__tc_fail__(3);
		}
		else if (!pdu.get_optional().has_receiptedMessageId())
		{
			__tc_fail__(4);
		}
		else
		{
			//получить оригинальную pdu
			MutexGuard mguard(pduReg->getMutex());
			IntermediateNotificationMonitor* monitor =
				pduReg->getIntermediateNotification(pdu.get_optional().get_userMessageReference());
			IntermediateNotificationPduData* pduData = NULL;
			__trace2__("iterate pdu for pduReg = %p, msgRef = %d",
				pduReg, (int) pdu.get_optional().get_userMessageReference());
			for (int i = 0; i < tmp.size(); i++)
			{
				if (!tmp[i]->isIntermediateNotification())
				{
					continue;
				}
				__require__(!pduData);
				pduData = tmp[i];
				//break;
			}
			//для user_message_reference из полученной pdu
			//нет соответствующего оригинального pdu
			if (!pduData)
			{
				__tc_fail__(5);
			}
			else
			{
				if (!monitor->pduData->valid)
				{
					__tc_fail__(6);
				}
				__tc_ok__;
				__require__(pduData->submitPduData->pdu->get_commandId() == SUBMIT_SM);
				PduSubmitSm* origPdu =
					reinterpret_cast<PduSubmitSm*>(pduData->pdu);
				//Сравнить правильность маршрута
				__tc__("processDeliverySm.intermediateNotification.checkRoute");
				__tc_fail2__(fixture->routeChecker->checkRouteForAcknowledgementSms(*origPdu, pdu), 0);
				__tc_ok_cond__;
				//проверить содержимое полученной pdu
				__tc__("processDeliverySm.intermediateNotification.checkFields");
				//поля хедера проверяются в processDeliverySm()
				//message
				//__compareCStr__(get_message().get_serviceType());
				//правильность адресов частично проверяется в fixture->routeChecker->checkRouteForAcknowledgementSms()
				__cfg_addr__(smscAlias);
				Address srcAlias;
				SmppUtil::convert(pdu.get_message().get_source(), &srcAlias);
				if (srcAlias != smscAlias)
				{
					__tc_fail__(1);
				}
				//__compareAddr__(get_message().get_dest());
				if (pdu.get_message().get_esmClass() !=
					ESM_CLASS_INTERMEDIATE_NOTIFICATION)
				{
					__tc_fail__(2);
				}
				__compare__(3, get_message().get_protocolId());
				//__compare__(get_message().get_priorityFlag());
				//__compare__(get_message().get_registredDelivery());
				__compare__(4, get_message().get_dataCoding());
				//__compare__(get_message().get_smLength());
				//__compareOStr__(get_message().get_shortMessage(),
				//optional
				//__tc_fail2__(SmppUtil::compareOptional(
				//	pdu.get_optional(), origPdu->get_optional()), 10);
				//intermediate notification получено, но не запрашивался
				//(origPdu->get_message().get_registredDelivery() &
				//	INTERMEDIATE_NOTIFICATION_REQUESTED)
				if (pduData->smsId != pdu.get_optional().get_receiptedMessageId())
				{
					__tc_fail__(5);
				}
				__tc_ok_cond__;
				*/
				/*
				__tc__("processDeliverySm.intermediateNotification.checkProfile");
				if (pduData->reportOptions == ProfileReportOptions::ReportNone)
				{
					__tc_fail__(1);
				}
				__tc_ok_cond__;
				//__tc__("processDeliverySm.intermediateNotification.checkStatus");
				*/
				/*
				//время доставки попадает в ожидаемый интервал
				__tc__("processDeliverySm.intermediateNotification.checkTime");
				if (recvTime < pduData->submitTime)
				{
					__tc_fail__(1);
				}
				__cfg_int__(timeCheckAccuracy);
				if (recvTime > pduData->validTime + timeCheckAccuracy)
				{
					__tc_fail__(2);
				}
				__tc_ok_cond__;
				//отправить респонс
				uint32_t deliveryStatus = fixture->respSender->sendDeliverySmResp(pdu);
				RespPduFlag respFlag = isAccepted(deliveryStatus);
				//разрешена ли доставка pdu
				__tc__("processDeliverySm.intermediateNotification.checkAllowed");
				switch (pduData->flag)
				{
					case PDU_REQUIRED_FLAG:
					case PDU_MISSING_ON_TIME_FLAG:
						pduData->flag = PDU_RECEIVED_FLAG;
						break;
					case PDU_RECEIVED_FLAG:
						//повторная доставка или какая-либо другая нотификация
						break;
					case PDU_NOT_EXPECTED_FLAG:
						__tc_fail__(1);
						break;
					default:
						__unreachable__("Unknown flag");
				}
				__tc_ok_cond__;
			}
		}
	}
	catch(...)
	{
		error();
		__tc_fail__(100);
	}
}
*/

void SmppReceiverTestCases::processGenericNack(PduGenericNack &pdu)
{
	__dumpPdu__("processGenericNack", fixture->smeInfo.systemId, &pdu);
	__decl_tc__;
	__tc__("processGenericNack");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processDataSm(PduDataSm &pdu)
{
	//__dumpPdu__("processDataSm", fixture->smeInfo.systemId, &pdu);
	__decl_tc__;
	__tc__("processDataSm");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processMultiResp(PduMultiSmResp &pdu)
{
	//__dumpPdu__("processMultiResp", fixture->smeInfo.systemId, &pdu);
	__decl_tc__;
	__tc__("processMultiResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processDataSmResp(PduDataSmResp &pdu)
{
	//__dumpPdu__("processDataSmResp", fixture->smeInfo.systemId, &pdu);
	__decl_tc__;
	__tc__("processDataSmResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processQuerySmResp(PduQuerySmResp &pdu)
{
	//__dumpPdu__("processQuerySmResp", fixture->smeInfo.systemId, &pdu);
	__decl_tc__;
	__tc__("processQuerySmResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processCancelSmResp(PduCancelSmResp &pdu)
{
	//__dumpPdu__("processCancelSmResp", fixture->smeInfo.systemId, &pdu);
	__decl_tc__;
	__tc__("processCancelSmResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processAlertNotification(PduAlertNotification &pdu)
{
	//__dumpPdu__("processAlertNotification", fixture->smeInfo.systemId, &pdu);
	__decl_tc__;
	__tc__("processAlertNotification");
	__tc_fail__(100);
}

void SmppReceiverTestCases::handleError(int errorCode)
{
	__trace2__("handleError(): errorCode = %d", errorCode);
	//abort();
}

}
}
}

