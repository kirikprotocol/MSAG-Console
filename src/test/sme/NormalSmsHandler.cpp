#include "NormalSmsHandler.hpp"
#include "SmppTransmitterTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/core/PduUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "profiler/profiler.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::sms::Address;
using smsc::util::Logger;
using smsc::test::conf::TestConfig;
using namespace smsc::profiler;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::smpp::DataCoding;
using namespace smsc::test::smpp;
using namespace smsc::test::core;
using namespace smsc::test::util;

NormalSmsHandler::NormalSmsHandler(SmppFixture* _fixture)
: fixture(_fixture), chkList(_fixture->chkList) {}

Category& NormalSmsHandler::getLog()
{
	static Category& log = Logger::getCategory("NormalSmsHandler");
	return log;
}

vector<int> NormalSmsHandler::checkRoute(PduSubmitSm& pdu1, PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, destAlias1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAlias1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//правильность destAddr
	const RouteHolder* routeHolder = NULL;
	const Address destAddr = fixture->aliasReg->findAddressByAlias(destAlias1);
	if (destAddr != destAddr2)
	{
		res.push_back(1);
	}
	else
	{
		//правильность маршрута
		routeHolder = fixture->routeReg->lookup(origAddr1, destAddr2);
		if (!routeHolder)
		{
			res.push_back(2);
		}
		else if (fixture->smeInfo.systemId != routeHolder->route.smeSystemId)
		{
			res.push_back(3);
		}
	}
	//правильность origAddr
	if (routeHolder)
	{
		if (fixture->smeInfo.wantAlias)
		{
			const Address origAlias = fixture->aliasReg->findAliasByAddress(origAddr1);
			if (origAlias != origAlias2)
			{
				res.push_back(4);
			}
		}
		else if (origAddr1 != origAlias2)
		{
			res.push_back(5);
		}
	}
	return res;
}

void NormalSmsHandler::compareMsgText(DeliveryMonitor* monitor,
	PduSubmitSm& origPdu, PduDeliverySm& pdu, time_t recvTime)
{
	__decl_tc__;
	__require__(monitor->pduData->objProps.count("sms.msg"));
	SmsMsg* msg = dynamic_cast<SmsMsg*>(monitor->pduData->objProps["sms.msg"]);
	if (!msg->valid)
	{
		return;
	}
	__tc__("deliverySm.normalSms.notMap.checkDataCoding");
	if (pdu.get_message().get_dataCoding() != msg->dataCoding)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	if (pdu.get_message().get_dataCoding() == origPdu.get_message().get_dataCoding())
	{
		__tc__("deliverySm.normalSms.notMap.checkEqualDataCoding");
	}
	else
	{
		__tc__("deliverySm.normalSms.notMap.checkDiffDataCoding");
	}
	if (pdu.get_message().size_shortMessage() &&
		pdu.get_optional().has_messagePayload())
	{
		__tc_fail__(1);
	}
	else if (pdu.get_message().size_shortMessage())
	{
		if (pdu.get_message().size_shortMessage() != msg->len)
		{
			__tc_fail__(2);
		}
		else if (memcmp(pdu.get_message().get_shortMessage(), msg->msg, msg->len))
		{
			__tc_fail__(3);
		}
	}
	else if (pdu.get_optional().has_messagePayload())
	{
		if (pdu.get_optional().size_messagePayload() != msg->len)
		{
			__tc_fail__(4);
		}
		else if (memcmp(pdu.get_optional().get_messagePayload(), msg->msg, msg->len))
		{
			__tc_fail__(5);
		}
	}
	else if (msg->len)
	{
		__tc_fail__(6);
	}
	__tc_ok_cond__;
	//игнорирую опциональный language_indicator
}

PduFlag NormalSmsHandler::compareSegmentedMapMsgText(DeliveryMonitor* monitor,
	PduSubmitSm& origPdu, PduDeliverySm& pdu, RespPduFlag respFlag,
	SmsMsg* msg, int concatRefNum, int concatMaxNum, int concatSeqNum)
{
	__decl_tc__;
	__require__(pdu.get_message().get_esmClass() & ESM_CLASS_UDHI_INDICATOR);
	__require__(pdu.get_message().size_shortMessage() >= 6);
	__trace2__("segmented messge received: refNum = %d, maxNum = %d, seqNum = %d",
		concatRefNum, concatMaxNum, concatSeqNum);

	int udhLen = 1 + *(unsigned char*) pdu.get_message().get_shortMessage();
	const char* sm = pdu.get_message().get_shortMessage() + udhLen;
	int smLen = pdu.get_message().size_shortMessage() - udhLen;

	if (pdu.get_message().get_dataCoding() == origPdu.get_message().get_dataCoding())
	{
		__tc__("deliverySm.normalSms.map.checkLongSmsEqualDataCoding");
	}
	else
	{
		__tc__("deliverySm.normalSms.map.checkLongSmsDiffDataCoding");
	}
	if (pdu.get_optional().has_messagePayload())
	{
		__tc_fail__(1);
	}
	//numSegments
	if (!monitor->pduData->intProps.count("map.maxNum"))
	{
		monitor->pduData->intProps["map.maxNum"] = concatMaxNum;
	}
	else if (monitor->pduData->intProps["map.maxNum"] != concatMaxNum)
	{
		__tc_fail__(2);
	}
	//refNum
	if (!monitor->pduData->intProps.count("map.refNum"))
	{
		monitor->pduData->intProps["map.refNum"] = concatRefNum;
	}
	else if (monitor->pduData->intProps["map.refNum"] != concatRefNum)
	{
		__tc_fail__(3);
	}
	//seqNum
	if (!monitor->pduData->intProps.count("map.seqNum"))
	{
		monitor->pduData->intProps["map.seqNum"] = 1;
	}
	if (monitor->pduData->intProps["map.seqNum"] != concatSeqNum)
	{
		__tc_fail__(4);
	}
	//length
	int sz = msg->len - msg->offset;
	if (msg->dataCoding == DEFAULT || msg->dataCoding == SMSC7BIT)
	{
		int len1 = 0, len2 = 0, ext = 0;
		bool prevWhiteSpace = true;
		for (int i = 0; i < sz; i++)
		{
			char ch = msg->msg[i + msg->offset];
			switch (ch)
			{
				case '|':
				case '^':
				case '{':
				case '}':
				case '[':
				case ']':
				case '~':
				case '\\':
					ext++;
					break;
			}
			if (i + ext >= 153)
			{
				if (!len1 && !len2)
				{
					len1 = len2 = i;
				}
				break;
			}
			switch (ch)
			{
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					if (!prevWhiteSpace) { len1 = i; }
					len2 = i + 1;
					prevWhiteSpace = true;
					break;
				default:
					if (prevWhiteSpace) { len2 = i; }
					prevWhiteSpace = false;
			}
		}
		if (sz + ext <= 153)
		{
			len1 = len2 = sz;
		}
		if (smLen < len1 || smLen > len2)
		{
			__trace2__("check segment len: seqNum = %d, maxNum = %d, len1 = %d, len2 = %d, msg offset = %d, msg len = %d",
				concatSeqNum, concatMaxNum, len1, len2, msg->offset, msg->len);
			__tc_fail__(5);
		}
	}
	else if (msg->dataCoding == UCS2)
	{
		int len1 = 0, len2 = 0, ext = 0;
		bool prevWhiteSpace = true;
		for (int i = 0; i < min(sz, 134); i += 2)
		{
			short ch;
			memcpy(&ch, msg->msg + msg->offset + i, 2);
			ch = ntohs(ch);
			switch (ch)
			{
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					if (!prevWhiteSpace) { len1 = i; }
					len2 = i + 2;
					prevWhiteSpace = true;
					break;
				default:
					if (prevWhiteSpace) { len2 = i; }
					prevWhiteSpace = false;
			}
		}
		if (sz <= 134)
		{
			len1 = len2 = sz;
		}
		if (smLen % 2)
		{
			__tc_fail__(6);
		}
		if (smLen < len1 || smLen > len2)
		{
			__trace2__("check segment len: seqNum = %d, maxNum = %d, len1 = %d, len2 = %d, msg offset = %d, msg len = %d",
				concatSeqNum, concatMaxNum, len1, len2, msg->offset, msg->len);
			__tc_fail__(7);
		}
	}
	else
	{
		if (smLen != 134 && smLen != sz)
		{
			__tc_fail__(8);
		}
	}
	//sm
	if (smLen > sz)
	{
		__tc_fail__(9);
	}
	else if (memcmp(sm, msg->msg + msg->offset, smLen))
	{
		__tc_fail__(10);
	}
	//последний сегмент
	if (smLen == sz && concatSeqNum != concatMaxNum)
	{
		__tc_fail__(11);
	}
	if (concatSeqNum == concatMaxNum && smLen != sz)
	{
		__tc_fail__(12);
	}
	__tc_ok_cond__;
	//отправлен ESME_ROK респонс на SC
	if (respFlag == RESP_PDU_OK)
	{
		monitor->pduData->intProps["map.seqNum"]++;
		msg->offset += smLen;
	}
	return (msg->offset < msg->len ? PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG);
}

PduFlag NormalSmsHandler::compareSimpleMapMsgText(DeliveryMonitor* monitor,
	PduSubmitSm& origPdu, PduDeliverySm& pdu, RespPduFlag respFlag,
	SmsMsg* msg)
{
	__decl_tc__;
	if (pdu.get_message().get_dataCoding() == origPdu.get_message().get_dataCoding())
	{
		__tc__("deliverySm.normalSms.map.checkShortSmsEqualDataCoding");
	}
	else
	{
		__tc__("deliverySm.normalSms.map.checkShortSmsDiffDataCoding");
	}
	//проверка на длину сообщения <= MAX_MAP_SM_LENGTH делается в SmppPduChecker
	if (pdu.get_message().size_shortMessage() &&
		pdu.get_optional().has_messagePayload())
	{
		__tc_fail__(1);
	}
	else if (pdu.get_message().size_shortMessage())
	{
		if (pdu.get_message().size_shortMessage() != msg->len)
		{
			__tc_fail__(2);
		}
		else if (memcmp(pdu.get_message().get_shortMessage(), msg->msg, msg->len))
		{
			__tc_fail__(3);
		}
	}
	else if (pdu.get_optional().has_messagePayload())
	{
		if (pdu.get_optional().size_messagePayload() != msg->len)
		{
			__tc_fail__(4);
		}
		else if (memcmp(pdu.get_optional().get_messagePayload(), msg->msg, msg->len))
		{
			__tc_fail__(5);
		}
	}
	else if (msg->len)
	{
		__tc_fail__(6);
	}
	__tc_ok_cond__;
	return PDU_NOT_EXPECTED_FLAG;
}

PduFlag NormalSmsHandler::compareMapMsgText(DeliveryMonitor* monitor,
	PduSubmitSm& origPdu, PduDeliverySm& pdu, RespPduFlag respFlag)
{
	__require__(monitor);
	__decl_tc__;
	__require__(monitor->pduData->objProps.count("map.msg"));
	SmsMsg* msg = dynamic_cast<SmsMsg*>(monitor->pduData->objProps["map.msg"]);
	if (!msg->valid)
	{
		return PDU_COND_REQUIRED_FLAG;
	}
	__tc__("deliverySm.normalSms.map.checkDataCoding");
	if (pdu.get_message().get_dataCoding() != msg->dataCoding)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	int concatRefNum = 0, concatMaxNum = 0, concatSeqNum = 0;
	bool udhi = pdu.get_message().get_esmClass() & ESM_CLASS_UDHI_INDICATOR;
	bool segmentedMsg = false;
	if (udhi && pdu.get_message().size_shortMessage())
	{
		unsigned char* sm = (unsigned char*) pdu.get_message().get_shortMessage();
		int udhLen = 1 + *sm;
		for (int i = 1; i < udhLen; )
		{
			//ie - informational element
			int ieId = sm[i++];
			int ieLen = sm[i++];
			if (ieId == 0) //Concatenated short messages, 8-bit reference number
			{
				__require__(ieLen == 3);
				segmentedMsg = true;
				concatRefNum = sm[i++];
				concatMaxNum = sm[i++];
				concatSeqNum = sm[i++];
				break;
			}
			i += ieLen;
		}
	}
	if (segmentedMsg)
	{
		return compareSegmentedMapMsgText(monitor, origPdu, pdu, respFlag,
			msg, concatRefNum, concatMaxNum, concatSeqNum);
	}
	else
	{
		return compareSimpleMapMsgText(monitor, origPdu, pdu, respFlag, msg);
	}
}

void NormalSmsHandler::registerIntermediateNotificationMonitor(
	const DeliveryMonitor* monitor, PduRegistry* pduReg, uint32_t deliveryStatus,
	time_t recvTime, time_t respTime)
{
	__require__(monitor && pduReg);
	//intermediate notification monitor отправляется только после первой
	//неудачной попытка доставки с rescheduling
	//lastAttempt считается по времени попытки, поэтому в случае конкатенации для
	//map proxy попытка применяется к серии кусочков
	if (monitor->getLastAttempt())
	{
		return;
	}
	IntermediateNotificationMonitor* m =
		pduReg->getIntermediateNotificationMonitor(monitor->msgRef);
	//__require__(!m);
	if (m)
	{
		__warning2__("registerIntermediateNotificationMonitor(): monitor = %p already registered", m);
		return;
	}
	__decl_tc__;
	uint8_t regDelivery =
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData);
	//flag
	PduFlag flag;
	switch (regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			return;
		case FINAL_SMSC_DELIVERY_RECEIPT:
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			flag = PDU_REQUIRED_FLAG;
			break;
		default:
			__unreachable__("Invalid regDelivery");
	}
	if (monitor->pduData->intProps.count("ussdServiceOp"))
	{
		__tc__("deliverySm.reports.intermediateNotification.ussdServiceOp");
		__tc_ok__;
		return;
	}
	//startTime
	time_t startTime;
	RespPduFlag respFlag = isAccepted(deliveryStatus);
	switch (respFlag)
	{
		case RESP_PDU_OK:
		case RESP_PDU_ERROR:
			return; //повторных доставок не будет
		case RESP_PDU_CONTINUE:
			return; //продолжается доставка пачки
		case RESP_PDU_RESCHED:
			startTime = respTime;
			break;
		case RESP_PDU_MISSING:
			startTime = recvTime + fixture->smeInfo.timeout - 1;
			break;
		default:
			__unreachable__("Invalid respFlag");
	}
	//register
	IntermediateNotificationMonitor* notifMonitor =
		new IntermediateNotificationMonitor(monitor->msgRef, startTime,
			monitor->pduData, PDU_REQUIRED_FLAG);
	notifMonitor->state = SMPP_ENROUTE_STATE;
	notifMonitor->deliveryStatus = deliveryStatus;
	pduReg->registerMonitor(notifMonitor);
}

void NormalSmsHandler::registerDeliveryReceiptMonitor(const DeliveryMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime, time_t respTime)
{
	__require__(monitor && pduReg);
	__decl_tc__;
	DeliveryReceiptMonitor* m = pduReg->getDeliveryReceiptMonitor(monitor->msgRef);
	if (m)
	{
		//если следующая попытка доставки сообщения приходится около validity_period,
		//то попытка может как состояться, так и нет и delivery report нужно
		//регистрировать/перерегистрировать в обоих случаях
		//__require__(m->getFlag() == PDU_COND_REQUIRED_FLAG);
		if (m->getFlag() != PDU_COND_REQUIRED_FLAG)
		{
			__warning2__("registerDeliveryReceiptMonitor(): monitor = %p status is not cond required", m);
			return;
		}
		pduReg->removeMonitor(m);
		delete m;
	}
	//flag
	PduFlag flag;
	switch (monitor->getFlag())
	{
		case PDU_REQUIRED_FLAG:
			return;
		case PDU_MISSING_ON_TIME_FLAG:
			__unreachable__("not expected");
			break;
		case PDU_COND_REQUIRED_FLAG:
			flag = PDU_COND_REQUIRED_FLAG;
			break;
		case PDU_NOT_EXPECTED_FLAG:
			flag = PDU_REQUIRED_FLAG;
			break;
		default:
			__unreachable__("Invalid flag");
	}
	//regDelivery
	uint8_t regDelivery =
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData);
	switch (regDelivery)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			return;
		case FINAL_SMSC_DELIVERY_RECEIPT:
			break;
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			if (deliveryStatus == ESME_ROK)
			{
				__tc__("deliverySm.reports.deliveryReceipt.failureDeliveryReceipt");
				__tc_ok__;
				return;
			}
			break;
		default:
			__unreachable__("Invalid regDelivery");
	}
	if (monitor->pduData->intProps.count("ussdServiceOp"))
	{
		__tc__("deliverySm.reports.deliveryReceipt.ussdServiceOp");
		__tc_ok__;
		return;
	}
	//startTime & state
	time_t startTime;
	SmppState state;
	RespPduFlag respFlag = isAccepted(deliveryStatus);
	switch (respFlag)
	{
		case RESP_PDU_OK:
			startTime = respTime;
			state = SMPP_DELIVERED_STATE;
			break;
		case RESP_PDU_ERROR:
			startTime = respTime;
			state = SMPP_UNDELIVERABLE_STATE;
			break;
		case RESP_PDU_RESCHED:
			__tc__("deliverySm.reports.deliveryReceipt.expiredDeliveryReceipt");
			__tc_ok__;
			startTime = monitor->getValidTime();
			state = SMPP_EXPIRED_STATE;
			break;
		case RESP_PDU_MISSING:
			__tc__("deliverySm.reports.deliveryReceipt.expiredDeliveryReceipt");
			__tc_ok__;
			startTime = max(recvTime + (time_t) (fixture->smeInfo.timeout - 1),
				monitor->getValidTime());
			state = SMPP_EXPIRED_STATE;
			break;
		default:
			__unreachable__("Invalid respFlag");
	}
	//register
	DeliveryReceiptMonitor* rcptMonitor = new DeliveryReceiptMonitor(
		monitor->msgRef, startTime, monitor->pduData,flag);
	rcptMonitor->state = state;
	rcptMonitor->deliveryStatus = deliveryStatus;
	pduReg->registerMonitor(rcptMonitor);
}

void NormalSmsHandler::registerDeliveryReportMonitors(const DeliveryMonitor* monitor,
	PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime, time_t respTime)
{
	registerIntermediateNotificationMonitor(monitor, pduReg, deliveryStatus,
		recvTime, respTime);
	registerDeliveryReceiptMonitor(monitor, pduReg, deliveryStatus,
		recvTime, respTime);
}

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

void NormalSmsHandler::processPdu(PduDeliverySm& pdu, const Address origAddr,
	time_t recvTime)
{
	__trace__("processNormalSms()");
	__decl_tc__;
	try
	{
		__tc__("deliverySm.normalSms");
		//в полученной pdu нет user_message_reference
		if (!pdu.get_optional().has_userMessageReference())
		{
			__tc_fail__(1);
			throw TCException();
		}
		//перкрыть pduReg класса
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(origAddr);
		__require__(pduReg);
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		DeliveryMonitor* monitor = pduReg->getDeliveryMonitor(
			pdu.get_optional().get_userMessageReference());
		//для user_message_reference из полученной pdu
		//нет соответствующего оригинального pdu
		if (!monitor)
		{
			__tc_fail__(2);
			__trace2__("getDeliveryMonitor(): pduReg = %p, userMessageReference = %d, monitor = NULL",
				pduReg, pdu.get_optional().get_userMessageReference());
			throw TCException();
		}
		__tc_ok_cond__;
		__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* origPdu =
			reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
		//проверить правильность маршрута
		__tc__("deliverySm.normalSms.checkRoute");
		__tc_fail2__(checkRoute(*origPdu, pdu), 0);
		__tc_ok_cond__;
		//сравнить поля полученной и оригинальной pdu
		__tc__("deliverySm.normalSms.checkMandatoryFields");
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
		//optional
		__tc__("deliverySm.normalSms.checkOptionalFields");
		//отключить message_payload, который проверяется в compareMsgText()
		__tc_fail2__(SmppUtil::compareOptional(pdu.get_optional(),
			origPdu->get_optional(), OPT_MSG_PAYLOAD + OPT_RCPT_MSG_ID), 0);
		__tc_ok_cond__;
		//проверка механизма повторной доставки
		__tc__("deliverySm.normalSms.scheduleChecks");
		__tc_fail2__(monitor->checkSchedule(recvTime), 0);
		__tc_ok_cond__;
		//отправить респонс
		pair<uint32_t, time_t> deliveryResp =
			fixture->respSender->sendDeliverySmResp(pdu);
		RespPduFlag respFlag = isAccepted(deliveryResp.first);
		//сравнить текст
		PduFlag textFlag = PDU_NOT_EXPECTED_FLAG; //получены все сегменты
		if (fixture->smeInfo.systemId == "MAP_PROXY")
		{
			textFlag = compareMapMsgText(monitor, *origPdu, pdu, respFlag);
		}
		else if (fixture->profileReg)
		{
			compareMsgText(monitor, *origPdu, pdu, recvTime);
		}
		//обновить статус delivery монитора
		pduReg->removeMonitor(monitor);
		switch (textFlag)
		{
			case PDU_REQUIRED_FLAG:
				if (respFlag == RESP_PDU_OK)
				{
					respFlag = RESP_PDU_CONTINUE;
				}
				break;
			case PDU_COND_REQUIRED_FLAG:
				monitor->setCondRequired();
				if (respFlag == RESP_PDU_OK)
				{
					respFlag = RESP_PDU_CONTINUE;
				}
				break;
			case PDU_NOT_EXPECTED_FLAG:
				break;
			default:
				__unreachable__("Invalid text flag");
		}
		__tc__("deliverySm.normalSms.checkAllowed");
		__tc_fail2__(monitor->update(recvTime, respFlag), 0);
		switch (respFlag)
		{
			case RESP_PDU_OK:
				monitor->state = SMPP_DELIVERED_STATE;
				break;
			case RESP_PDU_ERROR:
				monitor->state = SMPP_UNDELIVERABLE_STATE;
				break;
			case RESP_PDU_CONTINUE:
				monitor->state = SMPP_ENROUTE_STATE;
				break;
			case RESP_PDU_RESCHED:
			case RESP_PDU_MISSING:
				switch (monitor->getFlag())
				{
					case PDU_REQUIRED_FLAG:
					case PDU_COND_REQUIRED_FLAG:
						monitor->state = SMPP_ENROUTE_STATE;
						break;
					case PDU_NOT_EXPECTED_FLAG:
						monitor->state = SMPP_EXPIRED_STATE;
						break;
					default: //PDU_MISSING_ON_TIME_FLAG
						__unreachable__("Invalid flag");
				}
				break;
			default:
				__unreachable__("Invalid respFlag");
		}
		monitor->respStatus = deliveryResp.first;
		monitor->respTime = deliveryResp.second;
		//для ussd единственная попытка доставки
		if (monitor->pduData->intProps.count("ussdServiceOp"))
		{
			__tc__("deliverySm.normalSms.ussdServiceOp");
			__tc_ok__;
			monitor->setNotExpected();
		}
		pduReg->registerMonitor(monitor); //тест кейсы на финализированные pdu
		__tc_ok_cond__;
		//зарегистрировать delivery report мониторы
		SmeType smeType = fixture->smeReg->getSmeBindType(origAddr);
		if (smeType != SME_TRANSMITTER)
		{
			__require__(smeType == SME_RECEIVER || smeType == SME_TRANSCEIVER);
			registerDeliveryReportMonitors(monitor, pduReg, deliveryResp.first,
				recvTime, deliveryResp.second);
		}
	}
	catch (TCException&)
	{
		//отправить респонс
		fixture->respSender->sendDeliverySmResp(pdu);
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

