#include "SmscSmeTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/core/PduUtil.hpp"
#include "test/util/DateFormatter.hpp"
#include "test/util/TextUtil.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::test::conf::TestConfig;
using smsc::test::sms::SmsUtil;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::smpp::DataCoding;
using namespace smsc::profiler;
using namespace smsc::test::core;
using namespace smsc::test::smpp;
using namespace smsc::test::util;

SmscSmeTestCases::SmscSmeTestCases(SmppFixture* fixture)
: DeliveryReportHandler(fixture,
	TestConfig::getAddrParam("smscAddr"),
	TestConfig::getAddrParam("smscAlias"),
	TestConfig::getStrParam("smscServiceType"),
	TestConfig::getIntParam("smscProtocolId"))
{}

Category& SmscSmeTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmscSmeTestCases");
	return log;
}

void SmscSmeTestCases::submitSm(bool sync)
{
	__decl_tc__;
	__tc__("submitPduToSmscSme");
	try
	{
		PduSubmitSm* pdu = new PduSubmitSm();
		__cfg_addr__(smscAlias);
		fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, smscAlias, rand0(5));
		fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, NULL, NULL, NULL, PDU_NULL_ERR);
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmscSmeTestCases::dataSm(bool sync)
{
	__decl_tc__;
	__tc__("dataPduToSmscSme");
	try
	{
		PduDataSm* pdu = new PduDataSm();
		__cfg_addr__(smscAlias);
		fixture->transmitter->setupRandomCorrectDataSmPdu(pdu, smscAlias, rand0(5));
		fixture->transmitter->sendDataSmPdu(pdu, NULL, sync, NULL, NULL, NULL, PDU_NULL_ERR);
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

AckText* SmscSmeTestCases::getExpectedResponse(DeliveryReceiptMonitor* monitor,
	const string& text, time_t recvTime)
{
	__require__(monitor);
	__cfg_int__(timeCheckAccuracy);
	SmsPduWrapper origPdu(monitor->pduData->pdu, 0);
	//профиль
	Address srcAddr;
	SmppUtil::convert(origPdu.getSource(), &srcAddr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(srcAddr, t);
	bool valid = t + timeCheckAccuracy <= time(NULL);
	//destAlias
	Address destAlias;
	SmppUtil::convert(origPdu.getDest(), &destAlias);
	switch (monitor->state)
	{
		case SMPP_DELIVERED_STATE:
			for (time_t t = recvTime; t > recvTime - timeCheckAccuracy; t--)
			{
				static const DateFormatter df("dd MMMM yyyy, HH:mm:ss");
				ostringstream s;
				s << "*Your message sent to ";
				s << SmsUtil::configString(destAlias);
				s << " was successfully delivered on ";
				s << df.format(t);
				const pair<string, uint8_t> p = convert(s.str(), profile.codepage);
				__trace2__("getExpectedResponse(): %s", p.first.c_str());
				if (p.first.find(text) != string::npos)
				{
					return new AckText(p.first, p.second, valid);
				}
			}
			break;
		case SMPP_UNDELIVERABLE_STATE:
		case SMPP_EXPIRED_STATE:
			{
				time_t sendTime = monitor->pduData->sendTime;
				for (PduData* replacePduData = monitor->pduData->replacePdu; replacePduData; )
				{
					sendTime = replacePduData->sendTime;
				}
				for (time_t t = sendTime; t <= sendTime + timeCheckAccuracy; t++)
				{
					static const DateFormatter df("ddMMyyHHmmss");
					ostringstream s;
					s << "*ѕодтв ";
					s << SmsUtil::configString(destAlias) << " ";
					s << df.format(t) << ": ";
					if (monitor->state == SMPP_UNDELIVERABLE_STATE)
					{
						s << "permanent error"; //захардкожено
					}
					else if (monitor->state == SMPP_EXPIRED_STATE)
					{
						s << "expired"; //захардкожено
					}
					//s << monitor->deliveryStatus;
					const pair<string, uint8_t> p = convert(s.str(), profile.codepage);
					__trace2__("getExpectedResponse(): %s", p.first.c_str());
					if (p.first.find(text) != string::npos)
					{
						return new AckText(p.first, p.second, valid);
					}
				}
			}
			break;
	}
	return new AckText("Unknown", DEFAULT, valid);
}

AckText* SmscSmeTestCases::getExpectedResponse(
	IntermediateNotificationMonitor* monitor, const string& text, time_t recvTime)
{
	__require__(monitor);
	__cfg_int__(timeCheckAccuracy);
	SmsPduWrapper origPdu(monitor->pduData->pdu, 0);
	//профиль
	Address srcAddr;
	SmppUtil::convert(origPdu.getSource(), &srcAddr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(srcAddr, t);
	bool valid = t + timeCheckAccuracy <= time(NULL);
	//destAlias
	Address destAlias;
	SmppUtil::convert(origPdu.getDest(), &destAlias);
	time_t sendTime = monitor->pduData->sendTime;
	for (PduData* replacePduData = monitor->pduData->replacePdu; replacePduData; )
	{
		sendTime = replacePduData->sendTime;
	}
	for (time_t t = sendTime; t <= sendTime + timeCheckAccuracy; t++)
	{
		static const DateFormatter df("ddMMyyHHmmss");
		ostringstream s;
		s << "$Notif ";
		s << SmsUtil::configString(destAlias) << " ";
		s << df.format(t) << ": ";
		switch (monitor->deliveryStatus)
		{
			case ESME_ROK:
				__unreachable__("Invalid delivery status");
				//break;
			case ESME_RX_T_APPN:
			case ESME_RMSGQFUL:
				s << "subscriber busy";
				break;
			case ESME_RX_P_APPN:
				__unreachable__("Not supported");
			case DELIVERY_STATUS_NO_RESPONSE: //ошибка отправки deliver_sm_resp
				s << "delivery attempt timed out";
				break;
			case DELIVERY_STATUS_DEST_TRANSMITTER:
				s << "facility not supported";
				break; 
			default: //все остальные коды ошибок
				s << "destination unavailable";
		}
		const pair<string, uint8_t> p = convert(s.str(), profile.codepage);
		__trace2__("getExpectedResponse(): %s", p.first.c_str());
		if (p.first.find(text) != string::npos)
		{
			return new AckText(p.first, p.second, valid);
		}
	}
	return new AckText("Unknown", DEFAULT, valid);
}

#define __compare__(errCode, field, value) \
	if (value != field) { __tc_fail__(errCode); }

void SmscSmeTestCases::processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
	SmppHeader* header, time_t recvTime)
{
	__decl_tc__;
	//дл€ ext sme не может быть PDU_COND_REQUIRED_FLAG
	if (monitor->getFlag() == PDU_COND_REQUIRED_FLAG)
	{
		monitor->setNotExpected();
		return;
	}
	SmsPduWrapper pdu(header, 0);
	SmsPduWrapper origPdu(monitor->pduData->pdu, 0);
	__require__(pdu.isDeliverSm());
	//декодировать
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().size_shortMessage(), pdu.getDataCoding(), false);
	if (!monitor->pduData->objProps.count("smscSmeTc.receiptOutput"))
	{
		AckText* ack = getExpectedResponse(monitor, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["smscSmeTc.receiptOutput"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["smscSmeTc.receiptOutput"]);
	__require__(ack);
	if (!ack->valid)
	{
		monitor->setNotExpected();
		return;
	}
	__tc__("sms.reports.deliveryReceipt.checkStatus");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.getMsgRef());
	//если submit_sm_resp уже получен
	if (monitor->pduData->strProps.count("smsId"))
	{
		opt.set_receiptedMessageId(monitor->pduData->strProps["smsId"].c_str());
	}
	//иначе не провер€ю receiptedMessageId
	else if (pdu.get_optional().has_receiptedMessageId())
	{
		opt.set_receiptedMessageId(pdu.get_optional().get_receiptedMessageId());
	}
	else
	{
		opt.set_receiptedMessageId("");
	}
	uint8_t errCode[3];
	*errCode = (uint8_t) 3; //GSM
	uint16_t tmp = rand0(65535);
	memcpy(errCode + 1, &tmp, 2);
	uint8_t regDelivery =
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData);
	switch(monitor->state)
	{
		case SMPP_ENROUTE_STATE:
		case SMPP_DELETED_STATE:
			__tc_fail__(1);
			break;
		case SMPP_DELIVERED_STATE:
			if (regDelivery == FINAL_SMSC_DELIVERY_RECEIPT &&
				monitor->deliveryStatus == ESME_ROK)
			{
				opt.set_messageState(SMPP_DELIVERED_STATE);
			}
			else if (regDelivery == FAILURE_SMSC_DELIVERY_RECEIPT)
			{
				__tc_fail__(2);
			}
			else
			{
				__tc_fail__(3);
			}
			break;
		case SMPP_EXPIRED_STATE:
			opt.set_messageState(SMPP_EXPIRED_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		case SMPP_UNDELIVERABLE_STATE:
			opt.set_messageState(SMPP_UNDELIVERABLE_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		default:
			__unreachable__("Invalid flag");
	}
	__trace2__("expected optional part:\n%s", str(opt).c_str());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 10);
	__tc_ok_cond__;
	__tc__("sms.reports.deliveryReceipt.checkText");
	__compare__(1, pdu.getDataCoding(), ack->dataCoding);
	if (text != ack->text)
	{
		__trace2__("delivery receipt text mismatch: received:\n%s\nexpected:\n%s",
			text.c_str(), ack->text.c_str());
		__tc_fail__(2);
	}
	__tc_ok_cond__;
	monitor->setNotExpected();
}

void SmscSmeTestCases::processIntermediateNotification(
	IntermediateNotificationMonitor* monitor, SmppHeader* header, time_t recvTime)
{
	__decl_tc__;
	//дл€ ext sme не может быть PDU_COND_REQUIRED_FLAG
	if (monitor->getFlag() == PDU_COND_REQUIRED_FLAG)
	{
		monitor->setNotExpected();
		return;
	}
	SmsPduWrapper pdu(header, 0);
	SmsPduWrapper origPdu(monitor->pduData->pdu, 0);
	__require__(pdu.isDeliverSm());
	//декодировать
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().size_shortMessage(), pdu.getDataCoding(), false);
	if (!monitor->pduData->objProps.count("smscSmeTc.notificationOutput"))
	{
		AckText* ack = getExpectedResponse(monitor, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["smscSmeTc.notificationOutput"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["smscSmeTc.notificationOutput"]);
	__require__(ack);
	if (!ack->valid)
	{
		monitor->setNotExpected();
		return;
	}
	//проверить содержимое полученной pdu
	__tc__("sms.reports.intermediateNotification.checkStatus");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.getMsgRef());
	//если submit_sm_resp уже получен
	if (monitor->pduData->strProps.count("smsId"))
	{
		opt.set_receiptedMessageId(monitor->pduData->strProps["smsId"].c_str());
	}
	//иначе не провер€ю receiptedMessageId
	else if (pdu.get_optional().has_receiptedMessageId())
	{
		opt.set_receiptedMessageId(pdu.get_optional().get_receiptedMessageId());
	}
	else
	{
		opt.set_receiptedMessageId("");
	}
	uint8_t errCode[3];
	*errCode = (uint8_t) 3; //GSM
	uint16_t tmp = rand0(65535);
	memcpy(errCode + 1, &tmp, 2);
	switch(monitor->state)
	{
		case SMPP_ENROUTE_STATE: //темпоральна€ ошибка
		case SMPP_UNDELIVERABLE_STATE: //есть маршрут, но sme не забиндена
		case SMPP_EXPIRED_STATE: //pdu еще находитс€ в ENROUTE, но уже не будет доставл€тьс€
			opt.set_messageState(SMPP_ENROUTE_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		case SMPP_DELIVERED_STATE:
			__tc_fail__(1);
			break;
		case SMPP_DELETED_STATE:
			__tc_fail__(2);
			break;
		default:
			__unreachable__("Invalid flag");
	}
	__trace2__("expected optional part:\n%s", str(opt).c_str());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 10);
	__tc_ok_cond__;
	__tc__("sms.reports.intermediateNotification.checkText");
	__compare__(1, pdu.getDataCoding(), ack->dataCoding);
	if (text != ack->text)
	{
		__trace2__("intermediate notification text mismatch: received:\n%s\nexpected:\n%s",
			text.c_str(), ack->text.c_str());
		__tc_fail__(2);
	}
	__tc_ok_cond__;
	monitor->setNotExpected();
}

}
}
}

