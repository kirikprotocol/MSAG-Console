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

AckText* SmscSmeTestCases::getExpectedResponse(DeliveryReceiptMonitor* monitor,
	PduSubmitSm* origPdu, const string& text, time_t recvTime)
{
	__require__(monitor);
	__require__(origPdu);
	__cfg_int__(timeCheckAccuracy);
	//профиль
	Address srcAddr;
	SmppUtil::convert(origPdu->get_message().get_source(), &srcAddr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(srcAddr, t);
	bool valid = t + timeCheckAccuracy <= time(NULL);
	//destAlias
	Address destAlias;
	SmppUtil::convert(origPdu->get_message().get_dest(), &destAlias);
	switch (monitor->state)
	{
		case DELIVERED:
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
		case UNDELIVERABLE:
		case EXPIRED:
			for (time_t t = monitor->pduData->sendTime;
				  t <= monitor->pduData->sendTime + timeCheckAccuracy; t++)
			{
				static const DateFormatter df("ddMMyyHHmmss");
				ostringstream s;
				s << "*ѕодтв ";
				s << SmsUtil::configString(destAlias) << " ";
				s << df.format(t) << ": ";
				if (monitor->state == UNDELIVERABLE)
				{
					s << "permanent error"; //захардкожено
				}
				else if (monitor->state == EXPIRED)
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
			break;
	}
	return new AckText("Unknown", DEFAULT, valid);
}

AckText* SmscSmeTestCases::getExpectedResponse(
	IntermediateNotificationMonitor* monitor,
	PduSubmitSm* origPdu, const string& text, time_t recvTime)
{
	__require__(monitor);
	__require__(origPdu);
	__cfg_int__(timeCheckAccuracy);
	//профиль
	Address srcAddr;
	SmppUtil::convert(origPdu->get_message().get_source(), &srcAddr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(srcAddr, t);
	bool valid = t + timeCheckAccuracy <= time(NULL);
	//destAlias
	Address destAlias;
	SmppUtil::convert(origPdu->get_message().get_dest(), &destAlias);
	for (time_t t = monitor->pduData->sendTime;
		  t <= monitor->pduData->sendTime + timeCheckAccuracy; t++)
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
	PduDeliverySm& pdu, time_t recvTime)
{
	__decl_tc__;
	//дл€ ext sme не может быть PDU_COND_REQUIRED_FLAG
	if (monitor->getFlag() == PDU_COND_REQUIRED_FLAG)
	{
		return;
	}
	//оригинальна€ pdu
	__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* origPdu =
		reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
	__require__(origPdu);
	//декодировать
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().get_smLength(), pdu.get_message().get_dataCoding(), false);
	if (!monitor->pduData->objProps.count("smscSmeTc.receiptOutput"))
	{
		AckText* ack = getExpectedResponse(monitor, origPdu, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["smscSmeTc.receiptOutput"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["smscSmeTc.receiptOutput"]);
	__require__(ack);
	if (!ack->valid)
	{
		monitor->setCondRequired();
		return;
	}
	__tc__("deliverySm.reports.deliveryReceipt.checkStatus");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.get_optional().get_userMessageReference());
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
		case ENROUTE:
		case DELETED:
			__tc_fail__(1);
			break;
		case DELIVERED:
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
		case EXPIRED:
			opt.set_messageState(SMPP_EXPIRED_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		case UNDELIVERABLE:
			opt.set_messageState(SMPP_UNDELIVERABLE_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		default:
			__unreachable__("Invalid flag");
	}
	__trace2__("expected optional part:\n%s", str(opt).c_str());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 10);
	__tc_ok_cond__;
	__tc__("deliverySm.reports.deliveryReceipt.checkText");
	__compare__(1, pdu.get_message().get_dataCoding(), ack->dataCoding);
	if (text.length() > getMaxChars(ack->dataCoding))
	{
		__tc_fail__(2);
	}
	int pos = ack->text.find(text);
	__trace2__("delivery receipt: pos = %d, received:\n%s\nexpected:\n%s",
		pos, text.c_str(), ack->text.c_str());
	if (pos == string::npos)
	{
		__tc_fail__(3);
		monitor->setNotExpected();
	}
	else
	{
		__tc_ok__;
		ack->text.erase(pos, text.length());
		if (!ack->text.length())
		{
			monitor->setNotExpected();
		}
		else
		{
			__tc__("deliverySm.reports.multipleMessages");
			if (text.length() != getMaxChars(ack->dataCoding) &&
				ack->text.length() % getMaxChars(ack->dataCoding) != 0)
			{
				__tc_fail__(1);
			}
			__tc_ok_cond__;
		}
	}
}

void SmscSmeTestCases::processIntermediateNotification(
	IntermediateNotificationMonitor* monitor, PduDeliverySm& pdu, time_t recvTime)
{
	__decl_tc__;
	//дл€ ext sme не может быть PDU_COND_REQUIRED_FLAG
	if (monitor->getFlag() == PDU_COND_REQUIRED_FLAG)
	{
		return;
	}
	//оригинальна€ pdu
	__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* origPdu =
		reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
	__require__(origPdu);
	//декодировать
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().get_smLength(), pdu.get_message().get_dataCoding(), false);
	if (!monitor->pduData->objProps.count("smscSmeTc.notificationOutput"))
	{
		AckText* ack = getExpectedResponse(monitor, origPdu, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["smscSmeTc.notificationOutput"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["smscSmeTc.notificationOutput"]);
	__require__(ack);
	if (!ack->valid)
	{
		monitor->setCondRequired();
		return;
	}
	//проверить содержимое полученной pdu
	__tc__("deliverySm.reports.intermediateNotification.checkStatus");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.get_optional().get_userMessageReference());
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
		case ENROUTE: //темпоральна€ ошибка
		case UNDELIVERABLE: //есть маршрут, но sme не забиндена
		case EXPIRED: //pdu еще находитс€ в ENROUTE, но уже не будет доставл€тьс€
			opt.set_messageState(SMPP_ENROUTE_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		case DELIVERED:
			__tc_fail__(1);
			break;
		case DELETED:
			__tc_fail__(2);
			break;
		default:
			__unreachable__("Invalid flag");
	}
	__trace2__("expected optional part:\n%s", str(opt).c_str());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 10);
	__tc_ok_cond__;
	__tc__("deliverySm.reports.intermediateNotification.checkText");
	__compare__(1, pdu.get_message().get_dataCoding(), ack->dataCoding);
	if (text.length() > getMaxChars(ack->dataCoding))
	{
		__tc_fail__(2);
	}
	int pos = ack->text.find(text);
	__trace2__("intermediate notification: pos = %d, received:\n%s\nexpected:\n%s",
		pos, text.c_str(), ack->text.c_str());
	if (pos == string::npos)
	{
		__tc_fail__(3);
		monitor->setNotExpected();
	}
	else
	{
		__tc_ok__;
		ack->text.erase(pos, text.length());
		if (!ack->text.length())
		{
			monitor->setNotExpected();
		}
		else
		{
			__tc__("deliverySm.reports.multipleMessages");
			if (text.length() != getMaxChars(ack->dataCoding) &&
				ack->text.length() % getMaxChars(ack->dataCoding) != 0)
			{
				__tc_fail__(1);
			}
			__tc_ok_cond__;
		}
	}
}

}
}
}

