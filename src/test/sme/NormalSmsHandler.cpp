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

vector<int> NormalSmsHandler::checkRoute(SmppHeader* header1,
	SmppHeader* header2) const
{
	__require__(header1 && header2);
	SmsPduWrapper pdu1(header1, 0);
	SmsPduWrapper pdu2(header2, 0);
	vector<int> res;
	Address origAddr1, destAlias1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.getSource(), &origAddr1);
	SmppUtil::convert(pdu1.getDest(), &destAlias1);
	SmppUtil::convert(pdu2.getSource(), &origAlias2);
	SmppUtil::convert(pdu2.getDest(), &destAddr2);
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
	/*
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
	*/
	return res;
}

#define __check__(errCode, cond) \
	if (!(cond)) { __tc_fail__(errCode); }

void NormalSmsHandler::checkSourceAddr(DeliveryMonitor* monitor, SmppHeader* header)
{
	__require__(monitor->pduData->objProps.count("senderData"));
	__decl_tc__;
	SmsPduWrapper pdu(header, 0);
	SmsPduWrapper origPdu(monitor->pduData);
	SenderData* senderData =
		dynamic_cast<SenderData*>(monitor->pduData->objProps["senderData"]);
	//алиасенный адрес
	PduAddress aliasedAddr;
	Address addr;
	SmppUtil::convert(origPdu.getSource(), &addr);
	addr = fixture->aliasReg->findAliasByAddress(addr);
	SmppUtil::convert(addr, &aliasedAddr);
	if (fixture->smeInfo.wantAlias)
	{
		if (monitor->pduData->intProps.count("directive.hide"))
		{
			if (monitor->pduData->intProps["directive.hide"])
			{
				__tc__("sms.normalSms.checkSourceAddr.hideDirective");
				__check__(1, pdu.getSource() == aliasedAddr);
			}
			else
			{
				__tc__("sms.normalSms.checkSourceAddr.unhideDirective");
				__check__(1, pdu.getSource() == origPdu.getSource());
			}
		}
		else if (senderData->validProfile)
		{
			__tc__("sms.normalSms.checkSourceAddr.nohideDirective");
			if (senderData->profile.hide)
			{
				__check__(1, pdu.getSource() == aliasedAddr);
			}
			else
			{
				__check__(1, pdu.getSource() == origPdu.getSource());
			}
		}
	}
	else
	{
		__tc__("sms.normalSms.checkSourceAddr.notWantAlias");
		__check__(1, pdu.getSource() == origPdu.getSource());
	}
	__tc_ok_cond__;
}

//проверка: тип pdu, data coding, text
void NormalSmsHandler::checkNotMapMsgText(DeliveryMonitor* monitor,
	SmppHeader* header)
{
	__decl_tc__;
	__require__(monitor && header);
	__require__(monitor->pduData->objProps.count("senderData"));
	__require__(monitor->pduData->objProps.count("sms.msg"));
	__require__(monitor->pduData->intProps.count("dataCoding"));
	SmsMsg* msg = dynamic_cast<SmsMsg*>(monitor->pduData->objProps["sms.msg"]);
	if (!msg->valid)
	{
		return;
	}
	//при отправке с нормальной sme на больную sme результат непредсказуемый
	SenderData* senderData =
		dynamic_cast<SenderData*>(monitor->pduData->objProps["senderData"]);
	if (!senderData->smeInfo->forceDC && fixture->smeInfo.forceDC)
	{
		return;
	}
	SmsPduWrapper pdu(header, 0);
	SmsPduWrapper origPdu(monitor->pduData);
	//тип pdu
	if (pdu.isDeliverSm())
	{
		__tc__("sms.normalSms.notMap.checkDeliverSm");
		__check__(1, origPdu.isSubmitSm());
		__tc_ok_cond__;
	}
	else if (pdu.isDataSm())
	{
		__tc__("sms.normalSms.notMap.checkDataSm");
		__check__(1, origPdu.isDataSm());
		__tc_ok_cond__;
	}
	//data coding только для больная sme -> нормальную и нормальная -> нормальную
	if (!fixture->smeInfo.forceDC)
	{
		__tc__("sms.normalSms.notMap.checkDataCoding");
		__check__(1, pdu.getDataCoding() == msg->dataCoding);
		__tc_ok_cond__;
	}
	if (monitor->pduData->intProps["dataCoding"] == msg->dataCoding)
	{
		__tc__("sms.normalSms.notMap.checkEqualDataCoding");
	}
	else
	{
		__tc__("sms.normalSms.notMap.checkDiffDataCoding");
	}
	if (pdu.isDeliverSm())
	{
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
	}
	else if (pdu.isDataSm())
	{
		if (pdu.get_optional().has_messagePayload())
		{
			if (pdu.get_optional().size_messagePayload() != msg->len)
			{
				__tc_fail__(11);
			}
			else if (memcmp(pdu.get_optional().get_messagePayload(), msg->msg, msg->len))
			{
				__tc_fail__(12);
			}
		}
		else if (msg->len)
		{
			__tc_fail__(13);
		}
	}
	__tc_ok_cond__;
	//игнорирую опциональный language_indicator
}

//проверка: тип pdu, text
PduFlag NormalSmsHandler::checkSegmentedMapMsgText(DeliveryMonitor* monitor,
	SmppHeader* header, RespPduFlag respFlag, uint8_t dataCoding,
	SmsMsg* msg, int concatRefNum, int concatMaxNum, int concatSeqNum)
{
	__decl_tc__;
	SmsPduWrapper pdu(header, 0);
	__require__(pdu.getEsmClass() & ESM_CLASS_UDHI_INDICATOR);
	__require__(pdu.isDeliverSm() && pdu.get_message().size_shortMessage() >= 6);
	__trace2__("segmented message received: refNum = %d, maxNum = %d, seqNum = %d",
		concatRefNum, concatMaxNum, concatSeqNum);

	__tc__("sms.normalSms.map.longSms.checkDeliverSm");
	__check__(1, pdu.isDeliverSm());
	__tc_ok_cond__;

	int udhLen = 1 + *(unsigned char*) pdu.get_message().get_shortMessage();
	const char* sm = pdu.get_message().get_shortMessage() + udhLen;
	int smLen = pdu.get_message().size_shortMessage() - udhLen;

	if (dataCoding == msg->dataCoding)
	{
		__tc__("sms.normalSms.map.longSms.checkEqualDataCoding");
	}
	else
	{
		__tc__("sms.normalSms.map.longSms.checkDiffDataCoding");
	}
	__check__(1, !pdu.get_optional().has_messagePayload());
	//numSegments
	if (!monitor->pduData->intProps.count("map.maxNum"))
	{
		monitor->pduData->intProps["map.maxNum"] = concatMaxNum;
	}
	__check__(2, monitor->pduData->intProps["map.maxNum"] == concatMaxNum);
	//refNum
	if (!monitor->pduData->intProps.count("map.refNum"))
	{
		monitor->pduData->intProps["map.refNum"] = concatRefNum;
	}
	__check__(3, monitor->pduData->intProps["map.refNum"] == concatRefNum);
	//seqNum
	if (!monitor->pduData->intProps.count("map.seqNum"))
	{
		monitor->pduData->intProps["map.seqNum"] = 1;
	}
	__check__(4, monitor->pduData->intProps["map.seqNum"] == concatSeqNum);
	//length
	int sz = msg->len - msg->offset;
	if (msg->dataCoding == DEFAULT || msg->dataCoding == SMSC7BIT)
	{
		int len1 = 0, len2 = 0, prevLen1 = 0, prevLen2 = 0, ext = 0;
		bool prevWhiteSpace = true;
		for (int i = 0; i < sz; i++)
		{
			char ch = msg->msg[i + msg->offset];
			if (msg->dataCoding == DEFAULT)
			{
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
			}
			//текст без пробелов
			if (!len1 && !len2 && i + ext >= 153)
			{
				len1 = len2 = i;
				break;
			}
			switch (ch)
			{
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					if (!prevWhiteSpace)
					{
						prevLen1 = len1;
						len1 = i;
					}
					len2 = i + 1;
					prevWhiteSpace = true;
					break;
				default:
					if (prevWhiteSpace)
					{
						prevLen2 = len2;
						len2 = i;
					}
					prevWhiteSpace = false;
			}
			if (len2 + ext >= 153)
			{
				break;
			}
		}
		if (sz + ext <= 153)
		{
			len1 = len2 = sz;
		}
		else if (len1 + ext <= 153 && len2 + ext > 153)
		{
			len2 = 153 - ext;
		}
		else if (len1 + ext > 153)
		{
			len1 = prevLen1;
			len2 = prevLen2;
		}
		__require__(len1 <= len2 && len2 + ext <= 153);
		if (smLen < len1 || smLen > len2)
		{
			__trace2__("check segment len: seqNum = %d, maxNum = %d, len1 = %d, len2 = %d, ext = %d, msg offset = %d, msg len = %d",
				concatSeqNum, concatMaxNum, len1, len2, ext, msg->offset, msg->len);
			__tc_fail__(5);
		}
	}
	else if (msg->dataCoding == UCS2)
	{
		int len1 = 0, len2 = 0, prevLen1 = 0, prevLen2 = 0, ext = 0;
		bool prevWhiteSpace = true;
		for (int i = 0; i < sz; i += 2)
		{
			short ch;
			memcpy(&ch, msg->msg + msg->offset + i, 2);
			ch = ntohs(ch);
			//текст без пробелов
			if (!len1 && !len2 && i >= 134)
			{
				len1 = len2 = i;
				break;
			}
			switch (ch)
			{
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					if (!prevWhiteSpace)
					{
						prevLen1 = len1;
						len1 = i;
					}
					len2 = i + 2;
					prevWhiteSpace = true;
					break;
				default:
					if (prevWhiteSpace)
					{
						prevLen2 = len2;
						len2 = i;
					}
					prevWhiteSpace = false;
			}
			if (len2 >= 134)
			{
				break;
			}
		}
		if (sz <= 134)
		{
			len1 = len2 = sz;
		}
		else if (len1 <= 134 && len2 > 134)
		{
			len2 = 134;
		}
		else if (len1 > 134)
		{
			len1 = prevLen1;
			len2 = prevLen2;
		}
		__require__(len1 <= len2 && len2 <= 134);
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

//проверка: тип pdu, text
PduFlag NormalSmsHandler::checkSimpleMapMsgText(DeliveryMonitor* monitor,
	SmppHeader* header, RespPduFlag respFlag, uint8_t dataCoding, SmsMsg* msg)
{
	__decl_tc__;
	SmsPduWrapper pdu(header, 0);
	SmsPduWrapper origPdu(monitor->pduData);
	//тип pdu
	if (pdu.isDeliverSm())
	{
		__tc__("sms.normalSms.map.shortSms.checkDeliverSm");
		__check__(1, origPdu.isSubmitSm());
		__tc_ok_cond__;
	}
	else if (pdu.isDataSm())
	{
		__tc__("sms.normalSms.map.shortSms.checkDataSm");
		__check__(1, origPdu.isDataSm());
		__tc_ok_cond__;
	}
	if (dataCoding == msg->dataCoding)
	{
		__tc__("sms.normalSms.map.shortSms.checkEqualDataCoding");
	}
	else
	{
		__tc__("sms.normalSms.map.shortSms.checkDiffDataCoding");
	}
	//проверка на длину сообщения <= MAX_MAP_SM_LENGTH делается в SmppPduChecker
	if (pdu.isDeliverSm())
	{
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
	}
	else if (pdu.isDataSm())
	{
		if (pdu.get_optional().has_messagePayload())
		{
			if (pdu.get_optional().size_messagePayload() != msg->len)
			{
				__tc_fail__(11);
			}
			else if (memcmp(pdu.get_optional().get_messagePayload(), msg->msg, msg->len))
			{
				__tc_fail__(12);
			}
		}
		else if (msg->len)
		{
			__tc_fail__(13);
		}
	}
	__tc_ok_cond__;
	//игнорирую опциональный language_indicator
	return PDU_NOT_EXPECTED_FLAG;
}

PduFlag NormalSmsHandler::checkMapMsgText(DeliveryMonitor* monitor,
	SmppHeader* header, RespPduFlag respFlag)
{
	__require__(!fixture->smeInfo.forceDC);
	__require__(monitor && header);
	__require__(monitor->pduData->objProps.count("map.msg"));
	__require__(monitor->pduData->intProps.count("dataCoding"));
	__decl_tc__;
	SmsMsg* msg = dynamic_cast<SmsMsg*>(monitor->pduData->objProps["map.msg"]);
	if (!msg->valid)
	{
		return PDU_COND_REQUIRED_FLAG;
	}
	//data coding только для больная sme -> нормальную и нормальная -> нормальную
	SmsPduWrapper pdu(header, 0);
	if (!fixture->smeInfo.forceDC)
	{
		__tc__("sms.normalSms.map.checkDataCoding");
		__check__(1, pdu.getDataCoding() == msg->dataCoding);
		__tc_ok_cond__;
	}
	int concatRefNum = 0, concatMaxNum = 0, concatSeqNum = 0;
	bool udhi = pdu.getEsmClass() & ESM_CLASS_UDHI_INDICATOR;
	uint8_t dataCoding = monitor->pduData->intProps["dataCoding"];
	bool segmentedMsg = false;
	if (udhi && pdu.isDeliverSm() && pdu.get_message().size_shortMessage())
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
		return checkSegmentedMapMsgText(monitor, header, respFlag, dataCoding,
			msg, concatRefNum, concatMaxNum, concatSeqNum);
	}
	else
	{
		return checkSimpleMapMsgText(monitor, header, respFlag, dataCoding, msg);
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
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData, chkList);
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
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData, chkList);
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
				__tc__("sms.reports.deliveryReceipt.failureDeliveryReceipt");
				__tc_ok__;
				return;
			}
			break;
		default:
			__unreachable__("Invalid regDelivery");
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
			__tc__("sms.reports.deliveryReceipt.expiredDeliveryReceipt");
			__tc_ok__;
			startTime = monitor->getValidTime();
			state = SMPP_EXPIRED_STATE;
			break;
		case RESP_PDU_MISSING:
			__tc__("sms.reports.deliveryReceipt.expiredDeliveryReceipt");
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
	if (pdu.field != origPdu.field) { \
		ostringstream s1, s2; \
		s1 << (pdu.field); \
		s2 << (origPdu.field); \
		__trace2__("%s: %s != %s", #field, s1.str().c_str(), s2.str().c_str()); \
		__tc_fail__(failureCode); \
	}

#define __compareCStr__(failureCode, field) \
	if (strcmp(nvl(pdu.field), nvl(origPdu.field))) { \
		__trace2__("%s: %s != %s", #field, nvl(pdu.field), nvl(origPdu.field)); \
		__tc_fail__(failureCode); \
	}

void NormalSmsHandler::processPdu(SmppHeader* header, const Address& origAddr,
	time_t recvTime)
{
	__require__(header);
	__trace__("processNormalSms()");
	__decl_tc__;
	SmsPduWrapper pdu(header, recvTime);
	try
	{
		__tc__("sms.normalSms");
		//перкрыть pduReg класса
		PduRegistry* pduReg = fixture->smeReg->getPduRegistry(origAddr);
		__require__(pduReg);
		//получить оригинальную pdu
		MutexGuard mguard(pduReg->getMutex());
		DeliveryMonitor* monitor = pduReg->getDeliveryMonitor(pdu.getMsgRef());
		//для user_message_reference из полученной pdu
		//нет соответствующего оригинального pdu
		if (!monitor)
		{
			__tc_fail__(2);
			__trace2__("getDeliveryMonitor(): pduReg = %p, userMessageReference = %d, monitor = NULL",
				pduReg, pdu.getMsgRef());
			throw TCException();
		}
		__tc_ok_cond__;
		__require__(monitor->pduData->objProps.count("senderData"));
		SmsPduWrapper pdu(header, 0);
		SmsPduWrapper origPdu(monitor->pduData);
		SenderData* senderData =
			dynamic_cast<SenderData*>(monitor->pduData->objProps["senderData"]);
		//проверить правильность маршрута
		__tc__("sms.normalSms.checkRoute");
		__tc_fail2__(checkRoute(monitor->pduData->pdu, header), 0);
		__tc_ok_cond__;
		//проверка адреса отправителя
		checkSourceAddr(monitor, header);
		//сравнить поля полученной и оригинальной pdu
		__tc__("sms.normalSms.checkMandatoryFields");
		//поля хедера проверяются в processDeliverySm()
		//message
		__compareCStr__(1, getServiceType());
		//правильность адресов проверяется в fixture->routeChecker->checkRouteForNormalSms()
		//__compareAddr__(get_message().get_source());
		//__compareAddr__(get_message().get_dest());
		uint8_t esmClassMask = 0xfc; //без 2-ух младших битов
		//при наличии темплейта udhi не проверяю
		if (monitor->pduData->strProps.count("directive.template"))
		{
			esmClassMask &= ~0x40;
		}
		__compare__(2, getEsmClass() & esmClassMask);
		if ((pdu.getEsmClass() & ESM_CLASS_MESSAGE_TYPE_BITS) !=
			ESM_CLASS_NORMAL_MESSAGE)
		{
			__tc_fail__(3);
		}
		if (origPdu.isSubmitSm() && pdu.isDeliverSm())
		{
			__compare__(4, get_message().get_protocolId());
			//в действительности, priority задается маршрутом и
			//влияет на порядок доставки сообщений
			__compare__(5, get_message().get_priorityFlag());
		}
		__compare__(6, getRegistredDelivery());
		__tc_ok_cond__;
		//optional
		__tc__("sms.normalSms.checkOptionalFields");
		//отключить message_payload, который проверяется в compareMsgText()
		uint64_t excludeMask = OPT_MSG_PAYLOAD + OPT_RCPT_MSG_ID;
		if (senderData->smeInfo->forceDC || fixture->smeInfo.forceDC)
		{
			excludeMask += OPT_DEST_ADDR_SUBUNIT +
				OPT_MS_MSG_WAIT_FACILITIES + OPT_MS_VALIDITY;
		}
		__tc_fail2__(SmppUtil::compareOptional(pdu.get_optional(),
			origPdu.get_optional(), excludeMask), 0);
		__tc_ok_cond__;
		//проверка механизма повторной доставки
		__tc__("sms.normalSms.scheduleChecks");
		__tc_fail2__(monitor->checkSchedule(recvTime), 0);
		__tc_ok_cond__;
		//отправить респонс
		pair<uint32_t, time_t> deliveryResp = fixture->respSender->sendSmsResp(header);
		RespPduFlag respFlag = isAccepted(deliveryResp.first);
		//сравнить текст
		PduFlag textFlag = PDU_NOT_EXPECTED_FLAG; //получены все сегменты
		if (fixture->smeInfo.systemId == "MAP_PROXY")
		{
			textFlag = checkMapMsgText(monitor, header, respFlag);
		}
		else if (fixture->profileReg)
		{
			checkNotMapMsgText(monitor, header);
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
		__tc__("sms.normalSms.checkAllowed");
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
			__tc__("sms.normalSms.ussdServiceOp");
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
		fixture->respSender->sendSmsResp(header);
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

