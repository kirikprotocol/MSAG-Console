#include "SmppReceiverTestCases.hpp"
#include "test/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "core/synchronization/Mutex.hpp"
#include "util/debug.h"

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

//#define __compareTime__(field) \

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::sme::SmppTransmitter;
using namespace smsc::test;
using namespace smsc::test::smpp; //constants, SmppUtil
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants

SmppReceiverTestCases::SmppReceiverTestCases(const SmeSystemId& _systemId,
	const Address& addr, SmppResponseSender* _respSender,
	const SmeRegistry* _smeReg, const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg, RouteChecker* _routeChecker,
	SmppPduChecker* _pduChecker, CheckList* _chkList)
	: systemId(_systemId), smeAddr(addr), respSender(_respSender), smeReg(_smeReg),
	aliasReg(_aliasReg), routeReg(_routeReg), routeChecker(_routeChecker),
	pduChecker(_pduChecker), chkList(_chkList)
{
	__require__(respSender);
	__require__(smeReg);
	//__require__(aliasReg);
	//__require__(routeReg);
	//__require__(routeChecker);
	//__require__(pduChecker);
	//__require__(chkList);
	pduReg = smeReg->getPduRegistry(smeAddr); //может быть NULL
}

Category& SmppReceiverTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppReceiverTestCases");
	return log;
}

void SmppReceiverTestCases::processSubmitSmResp(PduSubmitSmResp &pdu)
{
	__dumpPdu__("processSubmitSmRespBefore", systemId, &pdu);
	time_t respTime = time(NULL);
	if (!pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("processSubmitSmResp.async");
	try
	{
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		PduData* pduData = pduReg->getPdu(pdu.get_header().get_sequenceNumber());
		//для sequence number из респонса нет соответствующего pdu
		if (!pduData)
		{
			__tc_fail__(1);
		}
		else
		{
			//проверить и обновить pduData по данным из респонса
			__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* origPdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
			pduChecker->processSubmitSmResp(pduData, pdu, respTime);
			//pduReg->updatePdu(pduData);
			//__dumpPdu__("processSubmitSmRespAfter", systemId, &pdu);
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processReplaceSmResp(PduReplaceSmResp &pdu)
{
	__dumpPdu__("processReplaceSmRespBefore", systemId, &pdu);
	time_t respTime = time(NULL);
	if (!pduReg)
	{
		return;
	}
	__decl_tc__;
	__tc__("processReplaceSmResp.async");
	try
	{
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		PduData* pduData = pduReg->getPdu(pdu.get_header().get_sequenceNumber());
		//для sequence number из респонса нет соответствующего pdu
		if (!pduData)
		{
			__tc_fail__(1);
		}
		else
		{
			//проверить и обновить pduData по данным из респонса
			__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* origPdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
			pduChecker->processReplaceSmResp(pduData, pdu, respTime);
			//pduReg->updatePdu(pduData);
			//__dumpPdu__("processReplaceSmRespAfter", systemId, &pdu);
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::processDeliverySm(PduDeliverySm &pdu)
{
	__dumpPdu__("processDeliverySmBefore", systemId, &pdu);
	__require__(session);
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
	if (pduReg)
	{
		switch (pdu.get_message().get_esmClass() & ESM_CLASS_MESSAGE_TYPE_BITS)
		{
			case ESM_CLASS_NORMAL_MESSAGE:
				processNormalSms(pdu, recvTime);
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
		respSender->sendDeliverySmResp(pdu);
	}
	//__dumpPdu__("processDeliverySmAfter", systemId, &pdu);
}

void SmppReceiverTestCases::processNormalSms(PduDeliverySm& pdu, time_t recvTime)
{
	__trace__("processNormalSms()");
	__decl_tc__;
	__tc__("processDeliverySm.normalSms");
	try
	{
		//в полученной pdu нет user_message_reference
		if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(1);
			return;
		}
		Address origAlias;
		SmppUtil::convert(pdu.get_message().get_source(), &origAlias);
		Address origAddr = aliasReg->findAddressByAlias(origAlias);
		//сначала поиск pdu по деалиасенному адресу отправителя, потом алиасенному
		for (bool cont = true; cont; )
		{
			//перкрыть pduReg класса
			PduRegistry* pduReg = smeReg->getPduRegistry(origAddr);
			if (!pduReg)
			{
				__trace2__("processNormalSms(): pduReg not found for addr = %s", str(origAddr).get());
				if (origAddr == origAlias)
				{
					__tc_fail__(2);
				}
			}
			else
			{
				//получить оригинальную pdu
				MutexGuard mguard(pduReg->getMutex());
				vector<PduData*> tmp =
					pduReg->getPdu(pdu.get_optional().get_userMessageReference());
				PduData* pduData = NULL;
				__trace2__("iterate pdu for pduReg = %p, msgRef = %d",
					pduReg, (int) pdu.get_optional().get_userMessageReference());
				for (int i = 0; i < tmp.size(); i++)
				{
					__require__(tmp[i]->pdu && tmp[i]->pdu->get_commandId() == SUBMIT_SM);
					PduSubmitSm* origPdu = reinterpret_cast<PduSubmitSm*>(tmp[i]->pdu);
					__trace2__("\tserviceType = %s, shortMessage size = %d",
						origPdu->get_message().get_serviceType(),
						(int) origPdu->get_message().size_shortMessage());
					//сравнить по service_type и short_message
					if (!strcmp(pdu.get_message().get_serviceType(), origPdu->get_message().get_serviceType()) &&
						pdu.get_message().size_shortMessage() == origPdu->get_message().size_shortMessage() &&
						!memcmp(pdu.get_message().get_shortMessage(), origPdu->get_message().get_shortMessage(), pdu.get_message().size_shortMessage()))
					{
						pduData = tmp[i];
					}
				}
				//для user_message_reference из полученной pdu
				//нет соответствующего оригинального pdu
				if (!pduData)
				{
					if (origAddr == origAlias)
					{
						__tc_fail__(3);
					}
				}
				else
				{
					__tc_ok__;
					__require__(pduData->pdu->get_commandId() == SUBMIT_SM);
					PduSubmitSm* origPdu =
						reinterpret_cast<PduSubmitSm*>(pduData->pdu);
					//проверить правильность маршрута
					__tc__("processDeliverySm.normalSms.checkRoute");
					__tc_fail2__(routeChecker->checkRouteForNormalSms(*origPdu, pdu), 0);
					__tc_ok_cond__;
					//сравнить поля полученной и оригинальной pdu
					__tc__("processDeliverySm.normalSms.checkMandatoryFields");
					//поля хедера проверяются в processDeliverySm()
					//message
					__compareCStr__(1, get_message().get_serviceType());
					//правильность адресов проверяется в routeChecker->checkRouteForNormalSms()
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
					__compare__(7, get_message().get_dataCoding());
					__compare__(8, get_message().get_smLength());
					__compareOStr__(9, get_message().get_shortMessage(),
						get_message().size_shortMessage());
					__tc_ok_cond__;
					//optional
					__tc__("processDeliverySm.normalSms.checkOptionalFields");
					__tc_fail2__(SmppUtil::compareOptional(
						pdu.get_optional(), origPdu->get_optional()), 0);
					__tc_ok_cond__;
					//проверка механизма повторной доставки
					__tc__("processDeliverySm.normalSms.scheduleChecks");
					__tc_fail2__(pduData->deliveryFlag.checkSchedule(recvTime), 0);
					__tc_ok_cond__;
					//отправить респонс
					bool accepted = respSender->sendDeliverySmResp(pdu);
					//обновить статус
					__tc__("processDeliverySm.normalSms.checkAllowed");
					__tc_fail2__(pduData->deliveryFlag.update(recvTime, accepted), 0);
					__tc_ok_cond__;
					//в случае повторной доставки изменить startTime для delivery receipt
					__tc__("processDeliverySm.normalSms.checkDeliveryReceipt");
					switch (pduData->deliveryReceiptFlag)
					{
						case PDU_REQUIRED_FLAG:
						case PDU_MISSING_ON_TIME_FLAG:
							{
								time_t nextTime = pduData->deliveryFlag.getNextTime(recvTime);
								pduData->deliveryReceiptFlag = PduReceiptFlag(PDU_REQUIRED_FLAG,
									accepted ? recvTime : nextTime, pduData->validTime);
							}
							break;
						case PDU_RECEIVED_FLAG:
							__tc_fail__(1);
							break;
						case PDU_NOT_EXPECTED_FLAG:
							break;
						default:
							__unreachable__("Invalid flag");
					}
					__tc_ok_cond__;
				}
			}
			cont = origAddr != origAlias;
			origAddr = origAlias;
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
	__tc__("processDeliverySm.deliveryReceipt");
	try
	{
		//в полученной pdu нет user_message_reference
		if (!pdu.get_optional().has_receiptedMessageId())
		{
			__tc_fail__(1);
		}
		else if (!pdu.get_optional().has_messageState())
		{
			__tc_fail__(2);
		}
		else
		{
			__require__(pduReg);
			//получить оригинальную pdu
			MutexGuard mguard(pduReg->getMutex());
			PduData* pduData =
				pduReg->getPdu(pdu.get_optional().get_receiptedMessageId());
			//для receipted_message_id из полученной pdu
			//нет соответствующего оригинального pdu
			if (!pduData)
			{
				__tc_fail__(3);
			}
			else
			{
				__tc_ok__;
				__require__(pduData->pdu->get_commandId() == SUBMIT_SM);
				PduSubmitSm* origPdu =
					reinterpret_cast<PduSubmitSm*>(pduData->pdu);
				//Сравнить правильность маршрута
				__tc__("processDeliverySm.deliveryReceipt.checkRoute");
				__tc_fail2__(routeChecker->checkRouteForNotification(*origPdu, pdu), 0);
				__tc_ok_cond__;
				//проверить содержимое полученной pdu
				__tc__("processDeliverySm.deliveryReceipt.checkFields");
				//поля хедера проверяются в processDeliverySm()
				//message
				//__compareCStr__(get_message().get_serviceType());
				//правильность адресов проверяется в routeChecker->checkRouteForNotification()
				//__compareAddr__(get_message().get_source());
				//__compareAddr__(get_message().get_dest());
				if (pdu.get_message().get_esmClass() !=
					ESM_CLASS_DELIVERY_RECEIPT)
				{
					__tc_fail__(1);
				}
				__compare__(2, get_message().get_protocolId());
				//__compare__(get_message().get_priorityFlag());
				//__compare__(get_message().get_registredDelivery());
				__compare__(3, get_message().get_dataCoding());
				//__compare__(get_message().get_smLength());
				//__compareOStr__(get_message().get_shortMessage(),
				//optional
				//__tc_fail2__(SmppUtil::compareOptional(
				//	pdu.get_optional(), origPdu->get_optional()), 10);
				switch (origPdu->get_message().get_registredDelivery() &
					SMSC_DELIVERY_RECEIPT_BITS)
				{
					//delivery receipt получено, но не запрашивался
					case NO_SMSC_DELIVERY_RECEIPT:
						__tc_fail__(10);
						break;
					case FAILURE_SMSC_DELIVERY_RECEIPT:
						//Должна быть причина ошибки
						if (!pdu.get_optional().has_networkErrorCode())
						{
							__tc_fail__(11);
						}
						//Сообщение не может быть успешно доставлено
						switch (pdu.get_optional().get_messageState())
						{
							case SMPP_ENROUTE_STATE:
							case SMPP_DELIVERED_STATE:
							case SMPP_ACCEPTED_STATE:
							case SMPP_UNKNOWN_STATE:
								__tc_fail__(12);
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
								__tc_fail__(13);
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
						//Некорректное значение
						__tc_fail__(14);
				}
				__tc_ok_cond__;
				//проверить информацию о доставленной pdu (код ошибки и статус)
				__tc__("processDeliverySm.deliveryReceipt.checkStatus");
				if (pdu.get_optional().has_networkErrorCode() &&
					!pduData->deliveryStatus)
				{
					__tc_fail__(1);
				}
				else if (!pdu.get_optional().has_networkErrorCode() &&
					pduData->deliveryStatus)
				{
					__tc_fail__(2);
				}
				else if (pdu.get_optional().has_networkErrorCode() &&
					pduData->deliveryStatus)
				{
					uint8_t networkType =
						*pdu.get_optional().get_networkErrorCode();
					uint16_t errCode = ntohs(*(uint16_t*) (
						pdu.get_optional().get_networkErrorCode() + 1));
					if (networkType != 3) //GSM
					{
						__tc_fail__(3);
					}
					if (errCode != pduData->deliveryStatus) //пока просто заглушка
					{
						__tc_fail__(4);
					}
				}
				switch (pdu.get_optional().get_messageState())
				{
					case SMPP_ENROUTE_STATE:
					case SMPP_ACCEPTED_STATE:
					case SMPP_UNKNOWN_STATE:
						__tc_fail__(11);
						break;
					case SMPP_DELIVERED_STATE:
						if (pduData->deliveryReceiptFlag != PDU_RECEIVED_FLAG)
						{
							__tc_fail__(12);
						}
						break;
					case SMPP_EXPIRED_STATE:
					case SMPP_DELETED_STATE:
					case SMPP_UNDELIVERABLE_STATE:
					case SMPP_REJECTED_STATE:
						if (pduData->deliveryReceiptFlag != PDU_NOT_EXPECTED_FLAG)
						{
							__tc_fail__(13);
						}
						break;
					default:
						__unreachable__("Invalid state");
				}
				__tc_ok_cond__;
				//проверка механизма повторной доставки
				__tc__("processDeliverySm.deliveryReceipt.scheduleChecks");
				__tc_fail2__(pduData->deliveryReceiptFlag.checkSchedule(recvTime), 0);
				__tc_ok_cond__;
				//отправить респонс
				bool accepted = respSender->sendDeliverySmResp(pdu);
				//обновить статус
				__tc__("processDeliverySm.deliveryReceipt.checkAllowed");
				__tc_fail2__(pduData->deliveryReceiptFlag.update(recvTime, accepted), 0);
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
	__trace__("processIntermediateNotification()");
	__decl_tc__;
	__tc__("processDeliverySm.intermediateNotification");
	try
	{
		//в полученной pdu нет user_message_reference
		if (!pdu.get_optional().has_receiptedMessageId())
		{
			__tc_fail__(1);
		}
		else if (!pdu.get_optional().has_messageState())
		{
			__tc_fail__(2);
		}
		else
		{
			__require__(pduReg);
			//получить оригинальную pdu
			MutexGuard mguard(pduReg->getMutex());
			PduData* pduData =
				pduReg->getPdu(pdu.get_optional().get_receiptedMessageId());
			//для receipted_message_id из полученной pdu
			//нет соответствующего оригинального pdu
			if (!pduData)
			{
				__tc_fail__(3);
			}
			else
			{
				__tc_ok__;
				__require__(pduData->pdu->get_commandId() == SUBMIT_SM);
				PduSubmitSm* origPdu =
					reinterpret_cast<PduSubmitSm*>(pduData->pdu);
				//Сравнить правильность маршрута
				__tc__("processDeliverySm.intermediateNotification.checkRoute");
				__tc_fail2__(routeChecker->checkRouteForNotification(*origPdu, pdu), 0);
				__tc_ok_cond__;
				//проверить содержимое полученной pdu
				__tc__("processDeliverySm.intermediateNotification.checkFields");
				//поля хедера проверяются в processDeliverySm()
				//message
				//__compareCStr__(get_message().get_serviceType());
				//правильность адресов проверяется в routeChecker->checkRouteForNotification()
				//__compareAddr__(get_message().get_source());
				//__compareAddr__(get_message().get_dest());
				if (pdu.get_message().get_esmClass() !=
					ESM_CLASS_INTERMEDIATE_NOTIFICATION)
				{
					__tc_fail__(1);
				}
				__compare__(2, get_message().get_protocolId());
				//__compare__(get_message().get_priorityFlag());
				//__compare__(get_message().get_registredDelivery());
				__compare__(3, get_message().get_dataCoding());
				//__compare__(get_message().get_smLength());
				//__compareOStr__(get_message().get_shortMessage(),
				//optional
				//__tc_fail2__(SmppUtil::compareOptional(
				//	pdu.get_optional(), origPdu->get_optional()), 10);
				//intermediate notification получено, но не запрашивался
				if (!(origPdu->get_message().get_registredDelivery() &
					INTERMEDIATE_NOTIFICATION_REQUESTED))
				{
					__tc_fail__(4);
				}
				__tc_ok_cond__;
				//__tc__("processDeliverySm.intermediateNotification.checkStatus");
				//время доставки попадает в ожидаемый интервал
				__tc__("processDeliverySm.intermediateNotification.checkTime");
				if (recvTime < pduData->submitTime)
				{
					__tc_fail__(1);
				}
				if (recvTime > pduData->validTime + timeCheckAccuracy)
				{
					__tc_fail__(2);
				}
				__tc_ok_cond__;
				//отправить респонс
				bool accepted = respSender->sendDeliverySmResp(pdu);
				//разрешена ли доставка pdu
				__tc__("processDeliverySm.intermediateNotification.checkAllowed");
				switch (pduData->intermediateNotificationFlag)
				{
					case PDU_REQUIRED_FLAG:
					case PDU_MISSING_ON_TIME_FLAG:
						//игнорирую accepted
						pduData->intermediateNotificationFlag = PDU_RECEIVED_FLAG;
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

void SmppReceiverTestCases::processGenericNack(PduGenericNack &pdu)
{
	__dumpPdu__("processGenericNack", systemId, &pdu);
	__decl_tc__;
	__tc__("processGenericNack");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processDataSm(PduDataSm &pdu)
{
	__dumpPdu__("processDataSm", systemId, &pdu);
	__decl_tc__;
	__tc__("processDataSm");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processMultiResp(PduMultiSmResp &pdu)
{
	__dumpPdu__("processMultiResp", systemId, &pdu);
	__decl_tc__;
	__tc__("processMultiResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processDataSmResp(PduDataSmResp &pdu)
{
	__dumpPdu__("processDataSmResp", systemId, &pdu);
	__decl_tc__;
	__tc__("processDataSmResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processQuerySmResp(PduQuerySmResp &pdu)
{
	__dumpPdu__("processQuerySmResp", systemId, &pdu);
	__decl_tc__;
	__tc__("processQuerySmResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processCancelSmResp(PduCancelSmResp &pdu)
{
	__dumpPdu__("processCancelSmResp", systemId, &pdu);
	__decl_tc__;
	__tc__("processCancelSmResp");
	__tc_fail__(100);
}

void SmppReceiverTestCases::processAlertNotification(PduAlertNotification &pdu)
{
	__dumpPdu__("processAlertNotification", systemId, &pdu);
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

