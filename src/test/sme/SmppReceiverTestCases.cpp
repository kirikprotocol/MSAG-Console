#include "SmppReceiverTestCases.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "core/synchronization/Mutex.hpp"
#include "util/debug.h"

#define __compare__(failureCode, field) \
	if (pdu.field != origPdu->field) { res->addFailure(failureCode); }

#define __compareStr__(failureCode, field) \
	if ((pdu.field && !origPdu->field) || \
		(!pdu.field && origPdu->field) || \
		(pdu.field && origPdu->field && string(pdu.field) != origPdu->field)) \
	{ res->addFailure(failureCode); }

#define __compareAddr__(failureCode, field) \
	if ((pdu.field && !origPdu->field) || \
		(!pdu.field && origPdu->field) || \
		(pdu.field && origPdu->field && !SmppUtil::compareAddresses(pdu.field, origPdu->field))) \
	{ res->addFailure(failureCode); }

#define __checkForNull__(failureCode, field) \
	if (pdu.field) { res->addFailure(failureCode); }

//#define __compareTime__(field) \

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::sme::SmppTransmitter;
using namespace smsc::test::smpp; //constants, SmppUtil
using namespace smsc::smpp::SmppCommandSet; //constants
using namespace smsc::smpp::SmppStatusSet; //constants

SmppReceiverTestCases::SmppReceiverTestCases(const SmeSystemId& _systemId,
	const Address& addr, const SmeRegistry* _smeReg,
	const AliasRegistry* _aliasReg, const RouteRegistry* _routeReg,
	ResultHandler* handler, RouteChecker* _routeChecker,
	SmppPduChecker* _pduChecker)
	: systemId(_systemId), smeAddr(addr), smeReg(_smeReg),
	aliasReg(_aliasReg), routeReg(_routeReg), resultHandler(handler),
	routeChecker(_routeChecker), pduChecker(_pduChecker)
{
	__require__(smeReg);
	__require__(aliasReg);
	__require__(routeReg);
	__require__(resultHandler);
	__require__(routeChecker);
	__require__(pduChecker);
	pduReg = smeReg->getPduRegistry(smeAddr); //может быть NULL
}

Category& SmppReceiverTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppReceiverTestCases");
	return log;
}

void SmppReceiverTestCases::processSubmitSmResp(PduSubmitSmResp &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processSubmitSmRespBefore", systemId, pdu);
	getLog().debug("[%d]\tprocessSubmitSmResp(): sequenceNumber = %u",
		thr_self(), pdu.get_header().get_sequenceNumber());
	if (!pduReg)
	{
		return;
	}
	TCResult* res = new TCResult(TC_PROCESS_SUBMIT_SM_RESP);
	try
	{
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		PduData* pduData = pduReg->getPdu(pdu.get_header().get_sequenceNumber());
		//для sequence number из респонса нет соответствующего pdu
		if (!pduData)
		{
			res->addFailure(1);
		}
		else
		{
			//проверить и обновить pduData по данным из респонса
			__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
			PduSubmitSm* origPdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
			vector<int> tmp = pduChecker->checkSubmitSmResp(pduData, pdu);
			for (int i = 0; i < tmp.size(); i++)
			{
				res->addFailure(tmp[i] > 0 ? 10 + tmp[i] : tmp[i]);
			}
			//обновить таблицу поиска по SMSId
			pduData->smsId = SmppUtil::convert(pdu.get_messageId());
			pduReg->updatePdu(pduData);
			__dumpPdu__("SmppReceiverTestCases::processSubmitSmRespAfter", systemId, pdu);
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	resultHandler->process(res);
}

void SmppReceiverTestCases::processDeliverySm(PduDeliverySm &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processDeliverySmBefore", systemId, pdu);
	getLog().debug("[%d]\tprocessDeliverySm(): sequenceNumber = %u",
		thr_self(), pdu.get_header().get_sequenceNumber());
	__require__(session);
	TCResult* res = new TCResult(TC_PROCESS_DELIVERY_SM);
	//общая проверка полей
	__checkForNull__(1, get_message().get_scheduleDeliveryTime());
	__checkForNull__(2, get_message().get_validityPeriod());
	__checkForNull__(3, get_message().get_replaceIfPresentFlag());
	__checkForNull__(4, get_message().get_smDefaultMsgId());
	bool okResp = false;
	try
	{
		//отправить респонс
		SmppTransmitter* transmitter;
		bool sync;
		switch (rand1(3))
		{
			case 1:
				sync = true;
				transmitter = session->getSyncTransmitter();
				break;
			default:
				sync = false;
				transmitter = session->getAsyncTransmitter();
		}
		PduDeliverySmResp respPdu;
		respPdu.get_header().set_sequenceNumber(pdu.get_header().get_sequenceNumber());
		switch (rand1(10))
		{
			case 1: //не отправлять респонс
				__trace__("SmppReceiverTestCases::sendDeliverySmRespNo");
				break;
			case 2: //отправить респонс с кодом ошибки
				respPdu.get_header().set_commandStatus(0x3); //Invalid Command ID
				__trace2__("SmppReceiverTestCases::sendDeliverySmResp%sBeforeErr", (sync ? "Sync" : "Async"));
				transmitter->sendDeliverySmResp(respPdu);
				__trace2__("SmppReceiverTestCases::sendDeliverySmResp%sAfterErr", (sync ? "Sync" : "Async"));
				break;
			default: //сказать что все ok и прекратить повторные доставки
				respPdu.get_header().set_commandStatus(ESME_ROK); //No Error
				__trace2__("SmppReceiverTestCases::sendDeliverySmResp%sBeforeOk", (sync ? "Sync" : "Async"));
				transmitter->sendDeliverySmResp(respPdu);
				__trace2__("SmppReceiverTestCases::sendDeliverySmResp%sAfterOk", (sync ? "Sync" : "Async"));
				okResp = true;
		}
		//обработать deliver_sm pdu
		if (okResp && pduReg)
		{
			switch (pdu.get_message().get_esmClass() & ESM_CLASS_MESSAGE_TYPE_BITS)
			{
				case ESM_CLASS_NORMAL_MESSAGE:
					{
						TCResult* res1 = processNormalSms(pdu);
						debug(res1);
						resultHandler->process(res1);
					}
					break;
				case ESM_CLASS_DELIVERY_RECEIPT:
					{
						TCResult* res2 = processDeliveryReceipt(pdu);
						debug(res2);
						resultHandler->process(res2);
					}
					break;
				case ESM_CLASS_INTERMEDIATE_NOTIFICATION:
					{
						TCResult* res3 = processIntermediateNotification(pdu);
						debug(res3);
						resultHandler->process(res3);
					}
					break;
				default:
					res->addFailure(101);
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	__dumpPdu__("SmppReceiverTestCases::processDeliverySmAfter", systemId, pdu);
	debug(res);
	resultHandler->process(res);
}

TCResult* SmppReceiverTestCases::processNormalSms(PduDeliverySm& pdu)
{
	TCResult* res = new TCResult(TC_PROCESS_NORMAL_SMS);
	try
	{
		PduRegistry* pduReg; //перкрыть pduReg класса
		Address origAlias, origAddr;
		SmppUtil::convert(pdu.get_message().get_source(), &origAlias);
		const AliasHolder* aliasHolder = aliasReg->findAddressByAlias(origAlias);
		if (!aliasHolder)
		{
			res->addFailure(101);
		}
		else if (!aliasHolder->aliasToAddress(origAlias, origAddr))
		{
			ostringstream os;
			os << "origAlias = " << origAlias << ", origAddr = " << origAddr;
			__trace2__("SmppReceiverTestCases::processNormalSms(102): %s",
				os.str().c_str());
			res->addFailure(102);
		}
		else if (!(pduReg = smeReg->getPduRegistry(origAddr)))
		{
			ostringstream os;
			os << "origAlias = " << origAlias << ", origAddr = " << origAddr;
			__trace2__("SmppReceiverTestCases::processNormalSms(103): %s",
				os.str().c_str());
			res->addFailure(103);
		}
		//в полученной pdu нет user_message_reference
		else if (!pdu.get_optional().has_userMessageReference())
		{
			res->addFailure(104);
		}
		else
		{
			ostringstream os;
			os << "origAlias = " << origAlias << ", origAddr = " << origAddr;
			__trace2__("SmppReceiverTestCases::processNormalSms(): %s",
				os.str().c_str());
			//получить оригинальную pdu
			MutexGuard mguard(pduReg->getMutex());
			PduData* pduData =
				pduReg->getPdu(pdu.get_optional().get_userMessageReference());
			//для user_message_reference из полученной pdu
			//нет соответствующего оригинального pdu
			if (!pduData)
			{
				res->addFailure(105);
			}
			else if (pduData->pdu->get_commandId() != SUBMIT_SM)
			{
			   	res->addFailure(106);
			}
			else
			{
				PduSubmitSm* origPdu =
					reinterpret_cast<PduSubmitSm*>(pduData->pdu);
				//данная pdu уже была доставлена или
				//вообще не должна была доставляться
				switch (pduData->deliveryFlag)
				{
					case PDU_REQUIRED_FLAG:
					case PDU_MISSING_ON_TIME_FLAG:
						pduData->deliveryFlag = PDU_RECEIVED_FLAG;
						break;
					case PDU_RECEIVED_FLAG:
						res->addFailure(107);
						break;
					case PDU_NOT_EXPECTED_FLAG:
						res->addFailure(108);
						break;
					default:
						__unreachable__("Unknown pduData->deliveryFlag");
				}
				//неправильное время доставки
				if (pduData->waitTime > time(NULL) ||
					pduData->waitTime < __checkTime__)
				{
					res->addFailure(109);
				}
				//Сравнить правильность маршрута
				vector<int> tmp = routeChecker->checkRouteForNormalSms(*origPdu, pdu);
				for (int i = 0; i < tmp.size(); i++)
				{
					res->addFailure(120 + tmp[i]);
				}
				//сравнить содержимое полученной и оригинальной pdu
				//header
				//__compare__(get_header().get_commandLength());
				//message (mandatory)
				__compareStr__(131, get_message().get_serviceType());
				//__compareAddr__(get_message().get_source());
				//__compareAddr__(get_message().get_dest());
				//__compare__(get_message().get_esmClass());
				if (pdu.get_message().get_esmClass() & ESM_CLASS_MESSAGE_TYPE_BITS !=
					ESM_CLASS_NORMAL_MESSAGE)
				{
					res->addFailure(132);
				}
				__compare__(133, get_message().get_protocolId());
				__compare__(134, get_message().get_priorityFlag());
				__compare__(135, get_message().get_registredDelivery());
				__compare__(136, get_message().get_dataCoding());
				__compare__(137, get_message().get_smLength());
				__compareStr__(138, get_message().get_shortMessage());
				//optional
				vector<int> tmp2 = SmppUtil::compareOptional(
					pdu.get_optional(), origPdu->get_optional());
				for (int i = 0; i < tmp2.size(); i++)
				{
					res->addFailure(150 + tmp2[i]);
				}
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	return res;
}

TCResult* SmppReceiverTestCases::processDeliveryReceipt(PduDeliverySm &pdu)
{
	TCResult* res = new TCResult(TC_PROCESS_DELIVERY_RECEIPT);
	try
	{
		//в полученной pdu нет user_message_reference
		if (!pdu.get_optional().has_receiptedMessageId())
		{
			res->addFailure(201);
		}
		else if (!pdu.get_optional().has_messageState())
		{
			res->addFailure(202);
		}
		else
		{
			__require__(pduReg);
			//получить оригинальную pdu
			MutexGuard mguard(pduReg->getMutex());
			PduData* pduData =
				pduReg->getPdu(SmppUtil::convert(pdu.get_optional().get_receiptedMessageId()));
			//для receipted_message_id из полученной pdu
			//нет соответствующего оригинального pdu
			if (!pduData)
			{
				res->addFailure(203);
			}
			else if (pduData->pdu->get_commandId() != SUBMIT_SM)
			{
				res->addFailure(204);
			}
			else
			{
				PduSubmitSm* origPdu =
					reinterpret_cast<PduSubmitSm*>(pduData->pdu);
				switch (origPdu->get_message().get_registredDelivery() &
					SMSC_DELIVERY_RECEIPT_BITS)
				{
					//delivery receipt получено, но не запрашивался
					case NO_SMSC_DELIVERY_RECEIPT:
						res->addFailure(205);
						break;
					case FAILURE_SMSC_DELIVERY_RECEIPT:
						//Должна быть причина ошибки
						if (!pdu.get_optional().has_networkErrorCode())
						{
							res->addFailure(206);
						}
						else
						{
							uint8_t networkType =
								*pdu.get_optional().get_networkErrorCode();
							uint16_t errCode = ntohs(*(uint16_t*) (
								pdu.get_optional().get_networkErrorCode() + 1));
							if (!errCode)
							{
								res->addFailure(207);
							}
						}
						//Сообщение не может быть успешно доставлено
						switch (pdu.get_optional().get_messageState())
						{
							case SMPP_ENROUTE_STATE:
							case SMPP_DELIVERED_STATE:
							case SMPP_ACCEPTED_STATE:
								res->addFailure(208);
								break;
							case SMPP_EXPIRED_STATE:
							case SMPP_DELETED_STATE:
							case SMPP_UNDELIVERABLE_STATE:
							case SMPP_UNKNOWN_STATE:
							case SMPP_REJECTED_STATE:
								//ok
								break;
						}
						break;
					case FINAL_SMSC_DELIVERY_RECEIPT:
						//Статус должен быть финальным
						if (pdu.get_optional().get_messageState() ==
							SMPP_ENROUTE_STATE)
						{
							res->addFailure(209);
						}
						break;
					default:
						//Некорректное значение
						res->addFailure(210);
				}
				//Должно придти только одно подтверждение доставки
				switch (pduData->deliveryReceiptFlag)
				{
					case PDU_REQUIRED_FLAG:
					case PDU_MISSING_ON_TIME_FLAG:
						pduData->deliveryReceiptFlag = PDU_RECEIVED_FLAG;
						break;
					case PDU_RECEIVED_FLAG:
						res->addFailure(211);
						break;
					case PDU_NOT_EXPECTED_FLAG:
						res->addFailure(212);
						break;
					default:
						__unreachable__("Unknown pduData->deliveryReceiptFlag");
				}
				//время доставки должно быть не на много позже wait_time
				if (pduData->waitTime < __checkTime__)
				{
					res->addFailure(213);
				}
				//Сравнить правильность маршрута
				vector<int> tmp =
					routeChecker->checkRouteForNotification(*origPdu, pdu);
				for (int i = 0; i < tmp.size(); i++)
				{
					res->addFailure(220 + tmp[i]);
				}
				//сравнить содержимое полученной и оригинальной pdu
				//header
				//__compare__(get_header().get_commandLength());
				//message (mandatory)
				//__compareStr__(get_message().get_serviceType());
				//__compareAddr__(get_message().get_source());
				//__compareAddr__(get_message().get_dest());
				if (pdu.get_message().get_esmClass() !=
					ESM_CLASS_DELIVERY_RECEIPT)
				{
					res->addFailure(231);
				}
				__compare__(232, get_message().get_protocolId());
				//__compare__(get_message().get_priorityFlag());
				//__compare__(get_message().get_registredDelivery());
				__compare__(233, get_message().get_dataCoding());
				//__compare__(get_message().get_smLength());
				//__compareStr__(get_message().get_shortMessage());
				//optional
				vector<int> tmp2 = SmppUtil::compareOptional(
					pdu.get_optional(), origPdu->get_optional());
				for (int i = 0; i < tmp2.size(); i++)
				{
					res->addFailure(250 + tmp2[i]);
				}
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(200);
	}
	return res;
}

TCResult* SmppReceiverTestCases::processIntermediateNotification(
	PduDeliverySm &pdu)
{
	TCResult* res = new TCResult(TC_PROCESS_INTERMEDIATE_NOTIFICATION);
	try
	{
		//в полученной pdu нет user_message_reference
		if (!pdu.get_optional().has_receiptedMessageId())
		{
			res->addFailure(301);
		}
		else if (!pdu.get_optional().has_messageState())
		{
			res->addFailure(302);
		}
		else
		{
			__require__(pduReg);
			//получить оригинальную pdu
			MutexGuard mguard(pduReg->getMutex());
			PduData* pduData =
				pduReg->getPdu(SmppUtil::convert(pdu.get_optional().get_receiptedMessageId()));
			//для receipted_message_id из полученной pdu
			//нет соответствующего оригинального pdu
			if (!pduData)
			{
				res->addFailure(302);
			}
			else if (pduData->pdu->get_commandId() != SUBMIT_SM)
			{
				res->addFailure(303);
			}
			else
			{
				PduSubmitSm* origPdu =
					reinterpret_cast<PduSubmitSm*>(pduData->pdu);
				//intermediate notification получено, но не запрашивался
				if (!(origPdu->get_message().get_registredDelivery() &
					INTERMEDIATE_NOTIFICATION_REQUESTED))
				{
					res->addFailure(304);
				}
				//intermediate notification может придти несколько
				//if (data->intermediateNotificationFlag) { ... }
				if (pdu.get_optional().get_messageState() != SMPP_ENROUTE_STATE)
				{
					pduData->intermediateNotificationFlag = PDU_RECEIVED_FLAG;
				}
				//время получения должно быть не на много позже wait_time
				if (pduData->waitTime < __checkTime__)
				{
					res->addFailure(305);
				}
				//Сравнить правильность маршрута
				vector<int> tmp =
					routeChecker->checkRouteForNotification(*origPdu, pdu);
				for (int i = 0; i < tmp.size(); i++)
				{
					res->addFailure(320 + tmp[i]);
				}
				//сравнить содержимое полученной и оригинальной pdu
				//header
				//__compare__(get_header().get_commandLength());
				//message (mandatory)
				//__compareStr__(get_message().get_serviceType());
				//__compareAddr__(get_message().get_source());
				//__compareAddr__(get_message().get_dest());
				if (pdu.get_message().get_esmClass() !=
					ESM_CLASS_INTERMEDIATE_NOTIFICATION)
				{
					res->addFailure(331);
				}
				__compare__(332, get_message().get_protocolId());
				//__compare__(get_message().get_priorityFlag());
				//__compare__(get_message().get_registredDelivery());
				__compare__(333, get_message().get_dataCoding());
				//__compare__(get_message().get_smLength());
				//__compareStr__(get_message().get_shortMessage());
				//optional
				vector<int> tmp2 = SmppUtil::compareOptional(
					pdu.get_optional(), origPdu->get_optional());
				for (int i = 0; i < tmp2.size(); i++)
				{
					res->addFailure(350 + tmp2[i]);
				}
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(300);
	}
	return res;
}

void SmppReceiverTestCases::processGenericNack(PduGenericNack &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processGenericNack", systemId, pdu);
}

void SmppReceiverTestCases::processDataSm(PduDataSm &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processDataSm", systemId, pdu);
}

void SmppReceiverTestCases::processMultiResp(PduMultiSmResp &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processMultiResp", systemId, pdu);
}

void SmppReceiverTestCases::processReplaceSmResp(PduReplaceSmResp &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processReplaceSmResp", systemId, pdu);
}

void SmppReceiverTestCases::processDataSmResp(PduDataSmResp &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processDataSmResp", systemId, pdu);
}

void SmppReceiverTestCases::processQuerySmResp(PduQuerySmResp &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processQuerySmResp", systemId, pdu);
}

void SmppReceiverTestCases::processCancelSmResp(PduCancelSmResp &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processCancelSmResp", systemId, pdu);
}

void SmppReceiverTestCases::processAlertNotification(PduAlertNotification &pdu)
{
	__dumpPdu__("SmppReceiverTestCases::processAlertNotification", systemId, pdu);
}

void SmppReceiverTestCases::handleError(int errorCode)
{
	TCResult* res = new TCResult(TC_HANDLE_ERROR);
	res->addFailure(errorCode);
	resultHandler->process(res);
	abort();
}

}
}
}

