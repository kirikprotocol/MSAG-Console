#include "SmppReceiverTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "core/synchronization/Mutex.hpp"

#define __compare__(failureCode, field) \
	if ((pdu.field) != (origPdu->field)) { \
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

#define __compareOStr__(failureCode, field, fieldSize) \
	if ((pdu.field && !origPdu->field) || \
		(!pdu.field && origPdu->field)) { \
		__tc_fail__(failureCode); \
	} else if (pdu.field && origPdu->field) { \
		if ((pdu.fieldSize != origPdu->fieldSize) || \
			strncmp(pdu.field, origPdu->field, pdu.fieldSize)) { \
			__trace2__("%s: = %s(%d) != %s(%d)", #field, pdu.field, \
				(int) pdu.fieldSize, origPdu->field, (int) origPdu->fieldSize); \
			__tc_fail__(failureCode); \
		} \
	}

#define __compareAddr__(failureCode, field) \
	if ((pdu.field && !origPdu->field) || \
		(!pdu.field && origPdu->field) || \
		(pdu.field && origPdu->field && !SmppUtil::compareAddresses(pdu.field, origPdu->field))) \
	{ __tc_fail__(failureCode); }

#define __checkForNull__(failureCode, field) \
	if (pdu.field) { __tc_fail__(failureCode); }

#define __compare_optional__(failureCode, field, expr) \
	if ((p1.has_##field() && !p2.has_##field()) || \
		(!p1.has_##field() && p2.has_##field()) || \
		(p1.has_##field() && p2.has_##field() && (expr))) \
		{ __tc_fail__(failureCode); }

#define __compare_optional_ostr__(failureCode, field) \
	__compare_optional__(failureCode, field, \
		p1.size_##field() != p2.size_##field() || \
		strncmp(p1.get_##field(), p2.get_##field(), p1.size_##field()))

//#define __compareTime__(field) \

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
	__dumpPdu__("processSubmitSmRespBefore", fixture->systemId, &pdu);
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
			//__dumpPdu__("processSubmitSmRespAfter", fixture->systemId, &pdu);
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
	__dumpPdu__("processReplaceSmRespBefore", fixture->systemId, &pdu);
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
			//__dumpPdu__("processReplaceSmRespAfter", fixture->systemId, &pdu);
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processDeliverySm(PduDeliverySm &pdu)
{
	__dumpPdu__("processDeliverySmBefore", fixture->systemId, &pdu);
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
		switch (pdu.get_message().get_esmClass() & ESM_CLASS_MESSAGE_TYPE_BITS)
		{
			case ESM_CLASS_NORMAL_MESSAGE:
				processNormalSms(pdu, recvTime); //включает в себя processSmeAcknowledgement()
				break;
			case ESM_CLASS_DELIVERY_RECEIPT:
				processDeliveryReceipt(pdu, recvTime);
				break;
			case ESM_CLASS_INTERMEDIATE_NOTIFICATION:
				processIntermediateNotification(pdu, recvTime);
				break;
			default:
				__tc_fail__(101);
		}
	}
	else //отправить респонс
	{
		fixture->respSender->sendDeliverySmResp(pdu);
	}
	//__dumpPdu__("processDeliverySmAfter", fixture->systemId, &pdu);
}

RespPduFlag SmppReceiverTestCases::isAccepted(uint32_t status)
{
	switch (status)
	{
		//повторная доставка
		case ESME_ROK:
			return RESP_PDU_OK;
		case ESME_RX_T_APPN:
		case ESME_RMSGQFUL:
			return RESP_PDU_RESCHED;
		case 0xffffffff: //ошибка отправки deliver_sm_resp
			return RESP_PDU_MISSING;
		case ESME_RX_P_APPN:
			__unreachable__("Not supported");
		//все остальные коды ошибок
		default:
			return RESP_PDU_ERROR;
	}
}

void SmppReceiverTestCases::compareMsgText(PduSubmitSm& origPdu, PduDeliverySm& pdu)
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
		//профайл может быть неконсистентным
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
	//игнорирую опциональный language_indicator
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

void SmppReceiverTestCases::updateDeliveryReceiptMonitor(DeliveryMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime)
{
	__require__(monitor && pduReg);
	__decl_tc__;
	DeliveryReceiptMonitor* rcptMonitor = pduReg->getDeliveryReceiptMonitor(
		monitor->pduData->msgRef, monitor->pduData);
	__require__(rcptMonitor);
	time_t nextTime = monitor->calcNextTime(recvTime);
	RespPduFlag respFlag = isAccepted(deliveryStatus);
	switch (rcptMonitor->regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			__require__(rcptMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
			break;
		case FINAL_SMSC_DELIVERY_RECEIPT:
			__tc__("processDeliverySm.deliveryReceipt.deliveryRescheduled");
			pduReg->removeMonitor(rcptMonitor);
			switch (respFlag)
			{
				case RESP_PDU_OK:
				case RESP_PDU_ERROR:
					rcptMonitor->reschedule(recvTime);
					break;
				case RESP_PDU_RESCHED:
				case RESP_PDU_MISSING:
					rcptMonitor->reschedule(nextTime ? nextTime : monitor->getValidTime());
					break;
				default:
					__unreachable__("Invalid resp flag");
			}
			pduReg->registerMonitor(rcptMonitor);
			__tc_ok__;
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			pduReg->removeMonitor(rcptMonitor);
			switch (respFlag)
			{
				case RESP_PDU_OK:
					__tc__("processDeliverySm.deliveryReceipt.failureDeliveryReceipt");
					__tc_ok__;
					rcptMonitor->setNotExpected();
					break;
				case RESP_PDU_ERROR:
					__tc__("processDeliverySm.deliveryReceipt.deliveryRescheduled");
					__tc_ok__;
					rcptMonitor->reschedule(recvTime);
					break;
				case RESP_PDU_RESCHED:
				case RESP_PDU_MISSING:
					if (nextTime)
					{
						__tc__("processDeliverySm.deliveryReceipt.deliveryRescheduled");
						__tc_ok__;
						rcptMonitor->reschedule(nextTime);
					}
					else
					{
						__tc__("processDeliverySm.deliveryReceipt.expiredDeliveryReceipt");
						__tc_ok__;
						rcptMonitor->reschedule(monitor->getValidTime());
					}
					break;
				default:
					__unreachable__("Invalid resp flag");
			}
			pduReg->registerMonitor(rcptMonitor);
			break;
		default:
			__unreachable__("Invalid registered delivery flag");
	}
	rcptMonitor->deliveryFlag = monitor->getFlag();
	rcptMonitor->deliveryStatus = deliveryStatus;
}

void SmppReceiverTestCases::processNormalSms(PduDeliverySm& pdu, time_t recvTime)
{
	__trace__("processNormalSms()");
	__decl_tc__;
	try
	{
		Address origAlias;
		SmppUtil::convert(pdu.get_message().get_source(), &origAlias);
		Address origAddr = fixture->aliasReg->findAddressByAlias(origAlias);
		//проверить тип sme
		if (fixture->smeReg->isExternalSme(origAddr))
		{
			processSmeAcknowledgement(pdu, recvTime);
			return;
		}
		//сначала поиск pdu по деалиасенному адресу отправителя, потом алиасенному
		//из-за специфики теста, т.к. addr -> alias неоднозначное преобразование
		for (;;)
		{
			__tc__("processDeliverySm.normalSms");
			//в полученной pdu нет user_message_reference
			if (!pdu.get_optional().has_userMessageReference())
			{
				__tc_fail__(1);
				break;
			}
			//перкрыть pduReg класса
			PduRegistry* pduReg = fixture->smeReg->getPduRegistry(origAddr);
			if (!pduReg)
			{
				__trace2__("processNormalSms(): pduReg not found for addr = %s", str(origAddr).c_str());
				if (origAddr == origAlias)
				{
					__tc_fail__(2);
					break;
				}
			}
			else
			{
				//получить оригинальную pdu
				MutexGuard mguard(pduReg->getMutex());
				DeliveryMonitor* monitor = pduReg->getDeliveryMonitor(
					pdu.get_optional().get_userMessageReference(),
					pdu.get_message().get_serviceType());
				//для user_message_reference из полученной pdu
				//нет соответствующего оригинального pdu
				if (!monitor)
				{
					if (origAddr == origAlias)
					{
						__tc_fail__(3);
						break;
					}
				}
				else
				{
					if (!monitor->pduData->valid)
					{
						__tc_fail__(4);
					}
					__tc_ok__;
					__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
					PduSubmitSm* origPdu =
						reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
					//проверить правильность маршрута
					__tc__("processDeliverySm.normalSms.checkRoute");
					__tc_fail2__(fixture->routeChecker->checkRouteForNormalSms(*origPdu, pdu), 0);
					__tc_ok_cond__;
					//сравнить поля полученной и оригинальной pdu
					__tc__("processDeliverySm.normalSms.checkMandatoryFields");
					//поля хедера проверяются в processDeliverySm()
					//message
					__compareCStr__(1, get_message().get_serviceType());
					//правильность адресов проверяется в fixture->routeChecker->checkRouteForNormalSms()
					//__compareAddr__(get_message().get_source());
					//__compareAddr__(get_message().get_dest());
					__compare__(2, get_message().get_esmClass() & 0xfc); //без 2-ух младших битов
					if ((pdu.get_message().get_esmClass() & ESM_CLASS_MESSAGE_TYPE_BITS) !=
						ESM_CLASS_NORMAL_MESSAGE)
					{
						__tc_fail__(3);
					}
					__compare__(4, get_message().get_protocolId());
					//в действительности, priority задается маршрутом и
					//влияет на порядок доставки сообщений
					__compare__(5, get_message().get_priorityFlag());
					__compare__(6, get_message().get_registredDelivery());
					__tc_ok_cond__;
					//сравнить текст
					if (fixture->profileReg)
					{
						compareMsgText(*origPdu, pdu);
					}
					//optional
					__tc__("processDeliverySm.normalSms.checkOptionalFields");
					//отключить message_payload, который проверяется в compareMsgText()
					__tc_fail2__(SmppUtil::compareOptional(
						pdu.get_optional(), origPdu->get_optional(), OPT_MSG_PAYLOAD), 0);
					__tc_ok_cond__;
					//проверка механизма повторной доставки
					__tc__("processDeliverySm.normalSms.scheduleChecks");
					__tc_fail2__(monitor->checkSchedule(recvTime), 0);
					__tc_ok_cond__;
					//отправить респонс
					uint32_t deliveryStatus =
						fixture->respSender->sendDeliverySmResp(pdu);
					RespPduFlag respFlag = isAccepted(deliveryStatus);
					//обновить статус delivery монитора
					__tc__("processDeliverySm.normalSms.checkAllowed");
					pduReg->removeMonitor(monitor);
						__tc_fail2__(monitor->update(recvTime, respFlag), 0);
					pduReg->registerMonitor(monitor);
					__tc_ok_cond__;
					//обновить статус delivery receipt монитора
					updateDeliveryReceiptMonitor(monitor, pduReg, deliveryStatus, recvTime);
					break; //завершить цикл
				}
			}
			if (origAddr == origAlias)
			{
				break;
			}
			origAddr = origAlias;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processSmeAcknowledgement(PduDeliverySm &pdu,
	time_t recvTime)
{
	__trace__("processSmeAcknowledgement()");
	__decl_tc__;
	try
	{
		__tc__("processDeliverySm.smeAck");
		//найти pduReg
		Address destAddr;
		SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(destAddr);
		if (!pduReg)
		{
			__trace2__("processSmeAcknowledgement(): pduReg not found for addr = %s", str(destAddr).c_str());
			__tc_fail__(1);
		}
		//обязательные для sme acknowledgement опциональные поля
		else if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(2);
		}
		else
		{
			//получить оригинальную pdu
			MutexGuard mguard(pduReg->getMutex());
			SmeAckMonitor* monitor = pduReg->getSmeAckMonitor(
				pdu.get_optional().get_userMessageReference());
			//для user_message_reference из полученной pdu
			//нет соответствующего оригинального pdu
			if (!monitor)
			{
				__tc_fail__(3);
			}
			else
			{
				if (!monitor->pduData->valid)
				{
					__tc_fail__(4);
				}
				__tc_ok__;
				__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
				PduSubmitSm* origPdu =
					reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
				//Сравнить правильность маршрута
				__tc__("processDeliverySm.smeAck.checkRoute");
				__tc_fail2__(fixture->routeChecker->checkRouteForAcknowledgementSms(
					*origPdu, pdu), 0);
				__tc_ok_cond__;
				//проверить содержимое полученной pdu
				__tc__("processDeliverySm.smeAck.checkFields");
				//поля хедера проверяются в processDeliverySm()
				//message
				//__compareCStr__(get_message().get_serviceType());
				//правильность адресов проверяется в fixture->routeChecker->checkRouteForAcknowledgementSms()
				//__compareAddr__(get_message().get_source());
				//__compareAddr__(get_message().get_dest());
				//__compare__(2, get_message().get_protocolId());
				//__compare__(get_message().get_priorityFlag());
				//__compare__(get_message().get_registredDelivery());
				__tc_ok_cond__;
				//передать дальнейшую проверку тест кейсам конкретных sme
				if (fixture->ackHandler)
				{
					pduReg->removeMonitor(monitor);
					fixture->ackHandler->processSmeAcknowledgement(monitor, pdu);
					pduReg->registerMonitor(monitor);
				}
				//правильность тела сообщения и опциональных полей
				//проверяется отдельно для конкретных типов acknoledgement
				//в processSmeAcknowledgement()
				//__compare__(get_message().get_smLength());
				//__compareOStr__(get_message().get_shortMessage());
				//optional
				//__tc_fail2__(SmppUtil::compareOptional(
				//	pdu.get_optional(), origPdu->get_optional()), 10);
				//для sme acknoledgement не проверяю повторную доставку
				__tc__("processDeliverySm.smeAck.recvTimeChecks");
				__cfg_int__(timeCheckAccuracy);
				if (recvTime < monitor->startTime)
				{
					__tc_fail__(1);
				}
				else if (recvTime > monitor->getValidTime() + timeCheckAccuracy)
				{
					__tc_fail__(2);
				}
				__tc_ok_cond__;
				//отправить респонс, только ESME_ROK разрешено
				uint32_t deliveryStatus = fixture->respSender->sendDeliverySmResp(pdu);
				RespPduFlag respFlag = isAccepted(deliveryStatus);
				__require__(respFlag == RESP_PDU_OK);
				//обновить статус
				__tc__("processDeliverySm.smeAck.checkAllowed");
				switch (monitor->getFlag())
				{
					case PDU_REQUIRED_FLAG:
					case PDU_MISSING_ON_TIME_FLAG:
						pduReg->removeMonitor(monitor);
						monitor->setReceived();
						pduReg->registerMonitor(monitor);
						break;
					case PDU_RECEIVED_FLAG:
						__tc_fail__(1);
						break;
					case PDU_NOT_EXPECTED_FLAG:
						__tc_fail__(2);
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
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processDeliveryReceipt(PduDeliverySm &pdu,
	time_t recvTime)
{
	__trace__("processDeliveryReceipt()");
	__decl_tc__;
	try
	{
		__tc__("processDeliverySm.deliveryReceipt");
		//найти pduReg
		Address destAddr;
		SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(destAddr);
		if (!pduReg)
		{
			__trace2__("processDeliveryReceipt(): pduReg not found for addr = %s", str(destAddr).c_str());
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
			DeliveryReceiptMonitor* monitor = pduReg->getDeliveryReceiptMonitor(
				pdu.get_optional().get_userMessageReference(),
				pdu.get_optional().get_receiptedMessageId());
			//для user_message_reference из полученной pdu
			//нет соответствующего оригинального pdu
			if (!monitor)
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
				__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
				PduSubmitSm* origPdu =
					reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
				//Сравнить правильность маршрута
				__tc__("processDeliverySm.deliveryReceipt.checkRoute");
				__tc_fail2__(fixture->routeChecker->checkRouteForAcknowledgementSms(*origPdu, pdu), 0);
				__tc_ok_cond__;
				//проверить содержимое полученной pdu
				__tc__("processDeliverySm.deliveryReceipt.checkFields");
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
					ESM_CLASS_DELIVERY_RECEIPT)
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
				if (monitor->pduData->smsId != pdu.get_optional().get_receiptedMessageId())
				{
					__tc_fail__(5);
				}
				__tc_ok_cond__;
				//расширенная проверка полей
				__tc__("processDeliverySm.deliveryReceipt.checkFieldsExt");
				switch (monitor->regDelivery)
				{
					//delivery receipt получено, но не запрашивался
					case NO_SMSC_DELIVERY_RECEIPT:
						__tc_fail__(1);
						break;
					case FAILURE_SMSC_DELIVERY_RECEIPT:
						//Должна быть причина ошибки
						if (!pdu.get_optional().has_networkErrorCode())
						{
							__tc_fail__(2);
						}
						//Сообщение не может быть успешно доставлено
						switch (pdu.get_optional().get_messageState())
						{
							case SMPP_ENROUTE_STATE:
							case SMPP_DELIVERED_STATE:
							case SMPP_ACCEPTED_STATE:
							case SMPP_UNKNOWN_STATE:
								__tc_fail__(3);
								break;
							case SMPP_EXPIRED_STATE:
							case SMPP_DELETED_STATE:
							case SMPP_UNDELIVERABLE_STATE:
							case SMPP_REJECTED_STATE:
								//ok
								break;
							default:
								__unreachable__("Invalid state");
						}
						break;
					case FINAL_SMSC_DELIVERY_RECEIPT:
						//Статус должен быть финальным
						switch (pdu.get_optional().get_messageState())
						{
							case SMPP_ENROUTE_STATE:
							case SMPP_ACCEPTED_STATE:
							case SMPP_UNKNOWN_STATE:
								__tc_fail__(4);
								break;
							case SMPP_DELIVERED_STATE:
							case SMPP_EXPIRED_STATE:
							case SMPP_DELETED_STATE:
							case SMPP_UNDELIVERABLE_STATE:
							case SMPP_REJECTED_STATE:
								//ok
								break;
							default:
								__unreachable__("Invalid state");
						}
						break;
					default:
						__unreachable__("Invalid registered delivery option");
				}
				__tc_ok_cond__;
				__tc__("processDeliverySm.deliveryReceipt.checkErrorCode");
				//проверить информацию о доставленной pdu (код ошибки)
				if (pdu.get_optional().has_networkErrorCode() &&
					!monitor->deliveryStatus)
				{
					__tc_fail__(1);
				}
				else if (!pdu.get_optional().has_networkErrorCode() &&
					monitor->deliveryStatus)
				{
					__tc_fail__(2);
				}
				else if (pdu.get_optional().has_networkErrorCode() &&
					monitor->deliveryStatus)
				{
					uint8_t networkType =
						*pdu.get_optional().get_networkErrorCode();
					uint16_t errCode = ntohs(*(uint16_t*) (
						pdu.get_optional().get_networkErrorCode() + 1));
					if (networkType != 3) //GSM
					{
						__tc_fail__(3);
					}
					if (errCode != monitor->deliveryStatus) //пока просто заглушка
					{
						__tc_fail__(4);
					}
				}
				__tc_ok_cond__;
				//проверить информацию о доставленной pdu (статус)
				__tc__("processDeliverySm.deliveryReceipt.checkState");
				switch (pdu.get_optional().get_messageState())
				{
					case SMPP_ENROUTE_STATE:
					case SMPP_ACCEPTED_STATE:
					case SMPP_UNKNOWN_STATE:
						__tc_fail__(1); //ничего не произошло, нотификация не нужна
						break;
					case SMPP_DELIVERED_STATE:
						if (monitor->deliveryFlag != PDU_RECEIVED_FLAG)
						{
							__tc_fail__(2);
						}
						break;
					case SMPP_EXPIRED_STATE:
					case SMPP_DELETED_STATE:
					case SMPP_UNDELIVERABLE_STATE:
					case SMPP_REJECTED_STATE:
						if (monitor->deliveryFlag != PDU_NOT_EXPECTED_FLAG)
						{
							__tc_fail__(3);
						}
						break;
					default:
						__unreachable__("Invalid state");
				}
				__tc_ok_cond__;
				//проверка механизма повторной доставки
				__tc__("processDeliverySm.deliveryReceipt.scheduleChecks");
				__tc_fail2__(monitor->checkSchedule(recvTime), 0);
				__tc_ok_cond__;
				//отправить респонс
				uint32_t deliveryStatus = fixture->respSender->sendDeliverySmResp(pdu);
				RespPduFlag respFlag = isAccepted(deliveryStatus);
				//обновить статус
				__tc__("processDeliverySm.deliveryReceipt.checkAllowed");
				pduReg->removeMonitor(monitor);
					__tc_fail2__(monitor->update(recvTime, respFlag), 0);
				pduReg->registerMonitor(monitor);
				__tc_ok_cond__;
			}
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processIntermediateNotification(
	PduDeliverySm &pdu, time_t recvTime)
{
	/*
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
	*/
}

void SmppReceiverTestCases::processGenericNack(PduGenericNack &pdu)
{
	__dumpPdu__("processGenericNack", fixture->systemId, &pdu);
	__decl_tc__;
	__tc__("processGenericNack");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processDataSm(PduDataSm &pdu)
{
	//__dumpPdu__("processDataSm", fixture->systemId, &pdu);
	__decl_tc__;
	__tc__("processDataSm");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processMultiResp(PduMultiSmResp &pdu)
{
	//__dumpPdu__("processMultiResp", fixture->systemId, &pdu);
	__decl_tc__;
	__tc__("processMultiResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processDataSmResp(PduDataSmResp &pdu)
{
	//__dumpPdu__("processDataSmResp", fixture->systemId, &pdu);
	__decl_tc__;
	__tc__("processDataSmResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processQuerySmResp(PduQuerySmResp &pdu)
{
	//__dumpPdu__("processQuerySmResp", fixture->systemId, &pdu);
	__decl_tc__;
	__tc__("processQuerySmResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processCancelSmResp(PduCancelSmResp &pdu)
{
	//__dumpPdu__("processCancelSmResp", fixture->systemId, &pdu);
	__decl_tc__;
	__tc__("processCancelSmResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processAlertNotification(PduAlertNotification &pdu)
{
	//__dumpPdu__("processAlertNotification", fixture->systemId, &pdu);
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

