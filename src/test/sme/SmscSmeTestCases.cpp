#include "SmscSmeTestCases.hpp"
#include "SmscSmeMessages.hpp"
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
	__require__(monitor->pduData->objProps.count("senderData"));
	__cfg_int__(timeCheckAccuracy);
	SmsPduWrapper origPdu(monitor->pduData);
	SenderData* senderData =
		dynamic_cast<SenderData*>(monitor->pduData->objProps["senderData"]);
	Address destAlias;
	SmppUtil::convert(origPdu.getDest(), &destAlias);
	//взять текущий профиль отправителя
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(senderData->srcAddr, t);
	bool validProfile = t + timeCheckAccuracy < recvTime;
	switch (monitor->state)
	{
		case SMPP_DELIVERED_STATE:
			for (time_t t = recvTime; t > recvTime - timeCheckAccuracy; t--)
			{
				const pair<string, uint8_t> p = SmscSmeDeliveredReceipt::format(
					senderData->smeInfo->receiptSchemeName, profile, destAlias, t);
				__trace2__("getExpectedResponse(): %s", p.first.c_str());
				if (p.first.find(text) != string::npos)
				{
					return new AckText(p.first, p.second, validProfile);
				}
			}
			break;
		case SMPP_UNDELIVERABLE_STATE:
		case SMPP_EXPIRED_STATE:
			{
				time_t sendTime = monitor->pduData->sendTime;
				PduData* pduData = monitor->pduData;
				//для replace_sm не меняется время submitTime
				while (pduData && pduData->intProps.count("replaceSm"))
				{
					__require__(pduData->replacePdu);
					sendTime = pduData->replacePdu->sendTime;
					pduData = pduData->replacePdu;
				}
				int status = monitor->state == SMPP_EXPIRED_STATE ?
					Status::EXPIRED : monitor->deliveryStatus;
				for (time_t t = sendTime; t <= sendTime + timeCheckAccuracy; t++)
				{
					const pair<string, uint8_t> p = SmscSmeFailedReceipt::format(
						senderData->smeInfo->receiptSchemeName, profile,
						destAlias, t, status);
					__trace2__("getExpectedResponse(): %s", p.first.c_str());
					if (p.first.find(text) != string::npos)
					{
						return new AckText(p.first, p.second, validProfile);
					}
				}
			}
			break;
	}
	return new AckText("Unknown", DEFAULT, validProfile);
}

AckText* SmscSmeTestCases::getExpectedResponse(
	IntermediateNotificationMonitor* monitor, const string& text, time_t recvTime)
{
	__require__(monitor);
	__cfg_int__(timeCheckAccuracy);
	SmsPduWrapper origPdu(monitor->pduData);
	SenderData* senderData =
		dynamic_cast<SenderData*>(monitor->pduData->objProps["senderData"]);
	Address destAlias;
	SmppUtil::convert(origPdu.getDest(), &destAlias);
	//взять текущий профиль отправителя
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(senderData->srcAddr, t);
	bool validProfile = t + timeCheckAccuracy < recvTime;
	time_t sendTime = monitor->pduData->sendTime;
	PduData* pduData = monitor->pduData;
	//для replace_sm не меняется время submitTime
	while (pduData && pduData->intProps.count("replaceSm"))
	{
		__require__(pduData->replacePdu);
		sendTime = pduData->replacePdu->sendTime;
		pduData = pduData->replacePdu;
	}
	for (time_t t = sendTime; t <= sendTime + timeCheckAccuracy; t++)
	{
		const pair<string, uint8_t> p = SmscSmeNotification::format(
			senderData->smeInfo->receiptSchemeName, profile,
			destAlias, t, monitor->deliveryStatus);
		__trace2__("getExpectedResponse(): %s", p.first.c_str());
		if (p.first.find(text) != string::npos)
		{
			return new AckText(p.first, p.second, validProfile);
		}
	}
	return new AckText("Unknown", DEFAULT, validProfile);
}

#define __compare__(errCode, field, value) \
	if (value != field) { __tc_fail__(errCode); }

void SmscSmeTestCases::processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
	SmppHeader* header, time_t recvTime)
{
	__decl_tc__;
	//для ext sme не может быть PDU_COND_REQUIRED_FLAG
	if (monitor->getFlag() == PDU_COND_REQUIRED_FLAG)
	{
		monitor->setNotExpected();
		return;
	}
	SmsPduWrapper pdu(header, 0);
	SmsPduWrapper origPdu(monitor->pduData);
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
	//иначе не проверяю receiptedMessageId
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
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData, chkList);
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
	//для ext sme не может быть PDU_COND_REQUIRED_FLAG
	if (monitor->getFlag() == PDU_COND_REQUIRED_FLAG)
	{
		monitor->setNotExpected();
		return;
	}
	SmsPduWrapper pdu(header, 0);
	SmsPduWrapper origPdu(monitor->pduData);
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
	//иначе не проверяю receiptedMessageId
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
		case SMPP_ENROUTE_STATE: //темпоральная ошибка
		case SMPP_UNDELIVERABLE_STATE: //есть маршрут, но sme не забиндена
		case SMPP_EXPIRED_STATE: //pdu еще находится в ENROUTE, но уже не будет доставляться
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

