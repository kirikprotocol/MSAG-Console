#include "SmppReceiverTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "test/util/DateFormatter.hpp"
#include "core/synchronization/Mutex.hpp"

#define __check__(errCode, field, value) \
	if (value != pdu.field) { __tc_fail__(errCode); }

#define __compare__(failureCode, field) \
	if (pdu.field != origPdu->field) { \
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
	pduReg->removeMonitor(rcptMonitor);
	switch (rcptMonitor->regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			__require__(rcptMonitor->getFlag() == PDU_NOT_EXPECTED_FLAG);
			break;
		case FINAL_SMSC_DELIVERY_RECEIPT:
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
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			switch (respFlag)
			{
				case RESP_PDU_OK:
					__tc__("processDeliverySm.deliveryReceipt.failureDeliveryReceipt");
					__tc_ok__;
					rcptMonitor->setNotExpected();
					break;
				case RESP_PDU_ERROR:
					rcptMonitor->reschedule(recvTime);
					break;
				case RESP_PDU_RESCHED:
				case RESP_PDU_MISSING:
					if (nextTime)
					{
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
			break;
		default:
			__unreachable__("Invalid registered delivery flag");
	}
	rcptMonitor->deliveryFlag = monitor->getFlag();
	rcptMonitor->deliveryStatus = deliveryStatus;
	pduReg->registerMonitor(rcptMonitor);
}

void SmppReceiverTestCases::processNormalSms(PduDeliverySm& pdu, time_t recvTime)
{
	__trace__("processNormalSms()");
	__decl_tc__;
	try
	{
		Address origAlias;
		SmppUtil::convert(pdu.get_message().get_source(), &origAlias);
		const Address origAddr = fixture->aliasReg->findAddressByAlias(origAlias);
		//проверить тип sme
		if (fixture->smeReg->isExternalSme(origAddr))
		{
			processSmeAcknowledgement(pdu, recvTime);
			return;
		}
		__tc__("processDeliverySm.normalSms");
		//в полученной pdu нет user_message_reference
		if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(1);
			return;
		}
		//перкрыть pduReg класса
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(origAddr);
		__require__(pduReg);
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		DeliveryMonitor* monitor = pduReg->getDeliveryMonitor(
			pdu.get_optional().get_userMessageReference(),
			pdu.get_message().get_serviceType());
		//для user_message_reference из полученной pdu
		//нет соответствующего оригинального pdu
		if (!monitor)
		{
			__tc_fail__(2);
			return;
		}
		if (!monitor->pduData->valid)
		{
			__tc_fail__(3);
			return;
		}
		__tc_ok_cond__;
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
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppReceiverTestCases::updateDeliveryReceiptMonitor(SmeAckMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime)
{
	__require__(monitor && pduReg);
	__require__(deliveryStatus == ESME_ROK);
	__decl_tc__;
	DeliveryReceiptMonitor* rcptMonitor = pduReg->getDeliveryReceiptMonitor(
		monitor->pduData->msgRef, monitor->pduData);
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

void SmppReceiverTestCases::processSmeAcknowledgement(PduDeliverySm &pdu,
	time_t recvTime)
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
			return;
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
			return;
		}
		if (!monitor->pduData->valid)
		{
			__tc_fail__(3);
			return;
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
				return;
			case PDU_NOT_EXPECTED_FLAG:
				__tc_fail__(2);
				return;
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
		__tc_fail2__(fixture->routeChecker->checkRouteForAcknowledgementSms(
			*origPdu, pdu), 0);
		__tc_ok_cond__;
		//проверить содержимое полученной pdu
		__tc__("processDeliverySm.smeAck.checkFields");
		//поля header проверяются в processDeliverySm()
		//поля message проверяются в fixture->ackHandler->processSmeAcknowledgement()
		//правильность адресов проверяется в fixture->routeChecker->checkRouteForAcknowledgementSms()
		__check__(1, get_message().get_esmClass(), ESM_CLASS_NORMAL_MESSAGE);
		__tc_ok_cond__;
		if (fixture->ackHandler)
		{
			pduReg->removeMonitor(monitor);
			fixture->ackHandler->processSmeAcknowledgement(monitor, pdu, recvTime);
			pduReg->registerMonitor(monitor);
		}
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
		RespPduFlag respFlag = isAccepted(deliveryStatus);
		__require__(respFlag == RESP_PDU_OK);
		//обновить статус delivery receipt монитора
		updateDeliveryReceiptMonitor(monitor, pduReg, deliveryStatus, recvTime);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

AckText* SmppReceiverTestCases::getExpectedResponse(
	DeliveryReceiptMonitor* monitor, PduSubmitSm* origPdu, const string& text,
	time_t recvTime)
{
	__require__(monitor);
	__require__(origPdu);
	__cfg_int__(timeCheckAccuracy);
	Address destAlias;
	SmppUtil::convert(origPdu->get_message().get_dest(), &destAlias);
	string expected;
	if (monitor->deliveryFlag == PDU_RECEIVED_FLAG)
	{
		static const DateFormatter df("dd MMMM yyyy, HH:mm:ss");
		for (time_t t = recvTime; t > recvTime - timeCheckAccuracy; t--)
		{
			ostringstream s;
			s << "Your message sent to address ";
			s << SmsUtil::configString(destAlias);
			s << " was successfully delivered on ";
			s << df.format(t);
			expected = s.str();
			if (expected == text)
			{
				break;
			}
		}
	}
	else if (monitor->deliveryFlag == PDU_NOT_EXPECTED_FLAG)
	{
		ostringstream s;
		s << "Ваше сообщение отправленное по адресу ";
		s << SmsUtil::configString(destAlias);
		s << " wasn't delivered, reason: failed"; //failed захардкожено
		//s << monitor->deliveryStatus;
		expected = s.str();
	}
	Address srcAddr;
	SmppUtil::convert(origPdu->get_message().get_source(), &srcAddr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(srcAddr, t);
	const pair<string, uint8_t> p = convert(expected, profile.codepage);
	bool valid = t + timeCheckAccuracy <= time(NULL);
	return new AckText(p.first, p.second, valid);
}

void SmppReceiverTestCases::processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
	PduDeliverySm& pdu, PduSubmitSm* origPdu, time_t recvTime)
{
	__decl_tc__;
	__cfg_addr__(smscAddr);
	__cfg_addr__(smscAlias);
	__cfg_str__(smscServiceType);
	__cfg_int__(smscProtocolId);

	Address destAddr;
	SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
	const SmeInfo* sme = fixture->smeReg->getSme(destAddr);
	__require__(sme);
	//декодировать
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().get_smLength(), pdu.get_message().get_dataCoding());
	if (!monitor->pduData->objProps.count("output"))
	{
		AckText* ack = getExpectedResponse(monitor, origPdu, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["output"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["output"]);
	__require__(ack);
	if (!ack->valid)
	{
		__trace__("monitor is not valid");
		monitor->pduData->intProps.count("skipReceivedCheck")
		monitor->setReceived();
		return;
	}
	//проверить содержимое полученной pdu
	__tc__("processDeliverySm.deliveryReceipt.checkFields");
	//поля хедера проверяются в processDeliverySm()
	//message
	__check__(1, get_message().get_serviceType(), smscServiceType);
	//правильность адресов частично проверяется в fixture->routeChecker->checkRouteForAcknowledgementSms()
	Address srcAlias;
	SmppUtil::convert(pdu.get_message().get_source(), &srcAlias);
	if (sme->wantAlias && srcAlias != smscAlias)
	{
		__tc_fail__(2);
	}
	else if (!sme->wantAlias && srcAlias != smscAddr)
	{
		__tc_fail__(3);
	}
	//__check__(4, get_message().get_dest(), origPdu->get_message().get_source());
	__check__(4, get_message().get_esmClass(), ESM_CLASS_DELIVERY_RECEIPT);
	__check__(5, get_message().get_protocolId(), smscProtocolId);
	__check__(6, get_message().get_priorityFlag(), 0);
	__check__(7, get_message().get_registredDelivery(), 0);
	__check__(8, get_message().get_dataCoding(), ack->dataCoding);
	if (text.length() > getMaxChars(ack->dataCoding))
	{
		__tc_fail__(9);
	}
	__tc_ok_cond__;
	__tc__("processDeliverySm.deliveryReceipt.checkStatus");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.get_optional().get_userMessageReference());
	opt.set_receiptedMessageId(monitor->pduData->smsId.c_str());
	switch(monitor->deliveryFlag)
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			__tc_fail__(1);
			break;
		case PDU_RECEIVED_FLAG:
			if (monitor->regDelivery == FINAL_SMSC_DELIVERY_RECEIPT &&
				monitor->deliveryStatus == ESME_ROK)
			{
				opt.set_messageState(SMPP_DELIVERED_STATE);
			}
			else if (monitor->regDelivery == FAILURE_SMSC_DELIVERY_RECEIPT)
			{
				__tc_fail__(2);
			}
			else
			{
				__tc_fail__(3);
			}
			break;
		case PDU_NOT_EXPECTED_FLAG:
			switch (monitor->deliveryStatus)
			{
				case ESME_ROK:
					__tc_fail__(4);
					break;
				case ESME_RX_T_APPN:
				case ESME_RMSGQFUL:
				case 0xffffffff: //ошибка отправки deliver_sm_resp
					opt.set_messageState(SMPP_EXPIRED_STATE);
					break;
				case ESME_RX_P_APPN:
					__unreachable__("Not supported");
				default:
					{
						opt.set_messageState(SMPP_REJECTED_STATE);
						uint8_t errCode[3];
						*errCode = 3; //GSM
						*((uint16_t*) (errCode + 1)) = rand0(65535);
						opt.set_networkErrorCode(errCode);
					}
			}
			break;
		default:
			__unreachable__("Invalid flag");
	}
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 10);
	__tc_ok_cond__;
	__tc__("processDeliverySm.deliveryReceipt.checkText");
	int pos = ack->text.find(text);
	__trace2__("delivery receipt: pos = %d, received:\n%s\nexpected:\n%s\n",
		pos, text.c_str(), ack->text.c_str());
	if (pos == string::npos)
	{
		__tc_fail__(1);
		monitor->setReceived();
	}
	else
	{
		__tc_ok__;
		ack->text.erase(pos, text.length());
		if (!ack->text.length())
		{
			monitor->setReceived();
		}
		else
		{
			__tc__("processDeliverySm.deliveryReceipt.multipleMessages");
			if (text.length() != getMaxChars(ack->dataCoding) &&
				ack->text.length() % getMaxChars(ack->dataCoding) != 0)
			{
				__tc_fail__(1);
			}
			__tc_ok_cond__;
		}
	}
}

void SmppReceiverTestCases::processDeliveryReceipt(PduDeliverySm& pdu,
	time_t recvTime)
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
			return;
		}
		//перекрыть pduReg класса
		Address destAddr;
		SmppUtil::convert(pdu.get_message().get_dest(), &destAddr);
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(destAddr);
		__require__(pduReg);
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		DeliveryReceiptMonitor* monitor = pduReg->getDeliveryReceiptMonitor(
			pdu.get_optional().get_userMessageReference(),
			pdu.get_optional().get_receiptedMessageId());
		//для user_message_reference из полученной pdu
		//нет соответствующего оригинального pdu
		if (!monitor)
		{
			__tc_fail__(2);
			return;
		}
		if (!monitor->pduData->valid)
		{
			__tc_fail__(3);
			return;
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
				return;
			case PDU_NOT_EXPECTED_FLAG:
				__tc_fail__(2);
				return;
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
		__tc_fail2__(fixture->routeChecker->checkRouteForNotificationSms(*origPdu, pdu), 0);
		__tc_ok_cond__;
		//проверка полей pdu
		pduReg->removeMonitor(monitor);
		processDeliveryReceipt(monitor, pdu, origPdu, recvTime);
		pduReg->registerMonitor(monitor);
		//для delivery receipt не проверяю повторную доставку
		__tc__("processDeliverySm.deliveryReceipt.recvTimeChecks");
		__cfg_int__(timeCheckAccuracy);
		if (recvTime < monitor->getStartTime())
		{
			__tc_fail__(1);
		}
		else if (recvTime > monitor->getStartTime() + timeCheckAccuracy)
		{
			__tc_fail__(2);
		}
		__tc_ok_cond__;
		//отправить респонс, только ESME_ROK разрешено
		uint32_t deliveryStatus = fixture->respSender->sendDeliverySmResp(pdu);
		RespPduFlag respFlag = isAccepted(deliveryStatus);
		__require__(respFlag == RESP_PDU_OK);
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

