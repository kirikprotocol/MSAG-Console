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
		__trace2__("%s = {%s, %s}", #field, s1.str().c_str(), s2.str().c_str()); \
		__tc_fail__(failureCode); \
	}

#define __compareCStr__(failureCode, field) \
	if ((pdu.field && !origPdu->field) || \
		(!pdu.field && origPdu->field) || \
		(pdu.field && origPdu->field && strcmp(pdu.field, origPdu->field))) { \
		__trace2__("%s = {%s, %s}", #field, pdu.field, origPdu->field); \
		__tc_fail__(failureCode); \
	}

#define __compareOStr__(failureCode, field, fieldSize) \
	if ((pdu.field && !origPdu->field) || \
		(!pdu.field && origPdu->field)) { \
		__tc_fail__(failureCode); \
	} else if (pdu.field && origPdu->field) { \
		if ((pdu.fieldSize != origPdu->fieldSize) || \
			strncmp(pdu.field, origPdu->field, pdu.fieldSize)) { \
			__trace2__("%s = {%s(%d), %s(%d)}", #field, pdu.field, \
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
	const Address& addr, const SmeRegistry* _smeReg,
	const AliasRegistry* _aliasReg, const RouteRegistry* _routeReg,
	RouteChecker* _routeChecker, SmppPduChecker* _pduChecker, CheckList* _chkList)
	: systemId(_systemId), smeAddr(addr), smeReg(_smeReg),
	aliasReg(_aliasReg), routeReg(_routeReg),
	routeChecker(_routeChecker), pduChecker(_pduChecker), chkList(_chkList)
{
	__require__(smeReg);
	//__require__(aliasReg);
	//__require__(routeReg);
	//__require__(routeChecker);
	__require__(pduChecker);
	//__require__(chkList);
	pduReg = smeReg->getPduRegistry(smeAddr); //может быть NULL
}

Category& SmppReceiverTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppReceiverTestCases");
	return log;
}

bool SmppReceiverTestCases::sendDeliverySmResp(PduDeliverySm& pdu, int num)
{
	int numTransmitter = 3; int numResp = 10;
	TCSelector s(num, numTransmitter * numResp);
	__decl_tc12__;
	try
	{
		//выбрать синхронный или асинхронный трансмитер
		bool sync;
		SmppTransmitter* transmitter;
		int num1 = s.value1(numTransmitter);
		switch (num1)
		{
			case 1:
				__tc1__("sendDeliverySmResp.sync");
				transmitter = session->getSyncTransmitter();
				sync = true;
				break;
			default:
				__require__(num1 <= numTransmitter);
				__tc1__("sendDeliverySmResp.async");
				transmitter = session->getAsyncTransmitter();
				sync = false;
				break;
		}
		//отправить респонс
		bool accepted;
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		int num2 = s.value2(numTransmitter);
		switch (num2)
		{
			case 1: //не отправлять респонс
				__tc2__("sendDeliverySmResp.notSend");
				__trace__("sendDeliverySmRespNo");
				accepted = false;
				break;
			case 2: //отправить респонс с кодом ошибки 0x1-0xff
				__tc2__("sendDeliverySmResp.sendWithErrCode");
				respPdu.get_header().set_commandStatus(rand1(0xff));
				__trace2__("sendDeliverySmResp%sBeforeErr1", (sync ? "Sync" : "Async"));
				transmitter->sendDeliverySmResp(respPdu);
				__trace2__("sendDeliverySmResp%sAfterErr1", (sync ? "Sync" : "Async"));
				accepted = false;
				break;
			case 3: //отправить респонс с кодом ошибки:
				//0x100-0x3ff - Reserved for SMPP extension
				//0x400-0x4ff - Reserved for SMSC vendor specific
				__tc2__("sendDeliverySmResp.sendWithErrCode");
				respPdu.get_header().set_commandStatus(rand2(0x100, 0x4ff));
				__trace2__("sendDeliverySmResp%sBeforeErr2", (sync ? "Sync" : "Async"));
				transmitter->sendDeliverySmResp(respPdu);
				__trace2__("sendDeliverySmResp%sAfterErr2", (sync ? "Sync" : "Async"));
				accepted = false;
				break;
			case 4: //отправить респонс с кодом ошибки >0x500 - Reserved
				__tc2__("sendDeliverySmResp.sendWithErrCode");
				respPdu.get_header().set_commandStatus(rand2(0x500, INT_MAX));
				__trace2__("sendDeliverySmResp%sBeforeErr3", (sync ? "Sync" : "Async"));
				transmitter->sendDeliverySmResp(respPdu);
				__trace2__("sendDeliverySmResp%sAfterErr3", (sync ? "Sync" : "Async"));
				accepted = false;
				break;
			case 5: //отправить респонс с неправильным sequence_number
				__tc2__("sendDeliverySmResp.sendInvalidSequenceNumber");
				respPdu.get_header().set_sequenceNumber(INT_MAX);
				respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
				__trace2__("sendDeliverySmResp%sBeforeInvalidSeqNum", (sync ? "Sync" : "Async"));
				transmitter->sendDeliverySmResp(respPdu);
				__trace2__("sendDeliverySmResp%sAfterInvalidSeqNum", (sync ? "Sync" : "Async"));
				accepted = false;
				break;
			default: //сказать что все ok и прекратить повторные доставки
				__require__(num2 < numResp);
				__tc2__("sendDeliverySmResp.sendOk");
				respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
				__trace2__("sendDeliverySmResp%sBeforeOk", (sync ? "Sync" : "Async"));
				transmitter->sendDeliverySmResp(respPdu);
				__trace2__("sendDeliverySmResp%sAfterOk", (sync ? "Sync" : "Async"));
				accepted = true;
				break;
		}
		__tc12_ok_cond__;
		return accepted;
	}
	catch(...)
	{
		__tc12_fail__(s.value());
		error();
	}
}

void SmppReceiverTestCases::processSubmitSmResp(PduSubmitSmResp &pdu)
{
	__dumpPdu__("processSubmitSmRespBefore", systemId, &pdu);
	getLog().debug("[%d]\tprocessSubmitSmResp(): sequenceNumber = %u",
		thr_self(), pdu.get_header().get_sequenceNumber());
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
			__dumpPdu__("processSubmitSmRespAfter", systemId, &pdu);
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
	getLog().debug("[%d]\tprocessReplaceSmResp(): sequenceNumber = %u",
		thr_self(), pdu.get_header().get_sequenceNumber());
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
			__dumpPdu__("processReplaceSmRespAfter", systemId, &pdu);
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
	getLog().debug("[%d]\tprocessDeliverySm(): sequenceNumber = %u",
		thr_self(), pdu.get_header().get_sequenceNumber());
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
		bool accepted = sendDeliverySmResp(pdu, RAND_TC);
	}
	__dumpPdu__("processDeliverySmAfter", systemId, &pdu);
}

void SmppReceiverTestCases::processNormalSms(PduDeliverySm& pdu, time_t recvTime)
{
	__decl_tc__;
	__tc__("processDeliverySm.normalSms");
	try
	{
		//перкрыть pduReg класса
		PduRegistry* pduReg;
		Address origAlias;
		SmppUtil::convert(pdu.get_message().get_source(), &origAlias);
		auto_ptr<const Address> origAddr = aliasReg->findAddressByAlias(origAlias);
		if (!origAddr.get())
		{
			__tc_fail__(1);
		}
		else if (!(pduReg = smeReg->getPduRegistry(*origAddr)))
		{
			ostringstream os;
			os << "origAlias = " << origAlias << ", origAddr = " << *origAddr;
			__trace2__("processNormalSms(2): %s",
				os.str().c_str());
			__tc_fail__(2);
		}
		//в полученной pdu нет user_message_reference
		else if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(3);
		}
		else
		{
			//получить оригинальную pdu
			MutexGuard mguard(pduReg->getMutex());
			PduData* pduData =
				pduReg->getPdu(pdu.get_optional().get_userMessageReference());
			//для user_message_reference из полученной pdu
			//нет соответствующего оригинального pdu
			if (!pduData)
			{
				__tc_fail__(4);
			}
			else if (pduData->pdu->get_commandId() != SUBMIT_SM)
			{
			   	__tc_fail__(5);
			}
			else
			{
				__tc_ok__;
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
				bool accepted = sendDeliverySmResp(pdu, RAND_TC);
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
			else if (pduData->pdu->get_commandId() != SUBMIT_SM)
			{
				__tc_fail__(4);
			}
			else
			{
				__tc_ok__;
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
				bool accepted = sendDeliverySmResp(pdu, RAND_TC);
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
			else if (pduData->pdu->get_commandId() != SUBMIT_SM)
			{
				__tc_fail__(4);
			}
			else
			{
				__tc_ok__;
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
				bool accepted = sendDeliverySmResp(pdu, RAND_TC);
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
	abort();
}

}
}
}

