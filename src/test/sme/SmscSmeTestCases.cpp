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
		fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, smscAlias);
		fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, NULL, NULL, NULL, PDU_NULL);
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
	//�������
	Address srcAddr;
	SmppUtil::convert(origPdu->get_message().get_source(), &srcAddr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(srcAddr, t);
	bool valid = t + timeCheckAccuracy <= time(NULL);
	//destAlias
	Address destAlias;
	SmppUtil::convert(origPdu->get_message().get_dest(), &destAlias);
	switch (monitor->deliveryFlag)
	{
		case PDU_RECEIVED_FLAG:
			for (time_t t = recvTime; t > recvTime - timeCheckAccuracy; t--)
			{
				static const DateFormatter df("dd MMMM yyyy, HH:mm:ss");
				ostringstream s;
				s << "*Your message sent to ";
				s << SmsUtil::configString(destAlias);
				s << " was successfully delivered on ";
				s << df.format(t);
				const pair<string, uint8_t> p = convert(s.str(), profile.codepage);
				if (p.first.find(text) != string::npos)
				{
					return new AckText(p.first, p.second, valid);
				}
			}
			break;
		case PDU_ERROR_FLAG:
		case PDU_EXPIRED_FLAG:
			for (time_t t = monitor->pduData->submitTime;
				  t <= monitor->pduData->submitTime + timeCheckAccuracy; t++)
			{
				static const DateFormatter df("ddMMyyHHmmss");
				ostringstream s;
				s << "*����� ";
				s << SmsUtil::configString(destAlias) << " ";
				s << df.format(t) << ": ";
				if (monitor->deliveryFlag == PDU_ERROR_FLAG)
				{
					s << "permanent error"; //������������
				}
				else if (monitor->deliveryFlag == PDU_EXPIRED_FLAG)
				{
					s << "expired"; //������������
				}
				//s << monitor->deliveryStatus;
				const pair<string, uint8_t> p = convert(s.str(), profile.codepage);
				if (p.first.find(text) != string::npos)
				{
					return new AckText(p.first, p.second, valid);
				}
			}
			break;
	}
	return new AckText("Unknown", DATA_CODING_SMSC_DEFAULT, valid);
}

AckText* SmscSmeTestCases::getExpectedResponse(
	IntermediateNotificationMonitor* monitor,
	PduSubmitSm* origPdu, const string& text, time_t recvTime)
{
	__require__(monitor);
	__require__(origPdu);
	__cfg_int__(timeCheckAccuracy);
	//�������
	Address srcAddr;
	SmppUtil::convert(origPdu->get_message().get_source(), &srcAddr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(srcAddr, t);
	bool valid = t + timeCheckAccuracy <= time(NULL);
	//destAlias
	Address destAlias;
	SmppUtil::convert(origPdu->get_message().get_dest(), &destAlias);
	switch (monitor->deliveryFlag)
	{
		case PDU_REQUIRED_FLAG:
		case PDU_EXPIRED_FLAG:
			for (time_t t = monitor->pduData->submitTime;
				  t <= monitor->pduData->submitTime + timeCheckAccuracy; t++)
			{
				static const DateFormatter df("ddMMyyHHmmss");
				ostringstream s;
				s << "$����� ";
				s << SmsUtil::configString(destAlias) << " ";
				s << df.format(t) << ": ";
				s << "temp error"; //������������
				//s << monitor->deliveryStatus;
				const pair<string, uint8_t> p = convert(s.str(), profile.codepage);
				if (p.first.find(text) != string::npos)
				{
					return new AckText(p.first, p.second, valid);
				}
			}
			break;
	}
	return new AckText("Unknown", DATA_CODING_SMSC_DEFAULT, valid);
}

#define __compare__(errCode, field, value) \
	if (value != field) { __tc_fail__(errCode); }

void SmscSmeTestCases::processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
	PduDeliverySm& pdu, time_t recvTime)
{
	__decl_tc__;
	//������������ pdu
	__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* origPdu =
		reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
	__require__(origPdu);
	//������������
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().get_smLength(), pdu.get_message().get_dataCoding());
	if (!monitor->pduData->objProps.count("deliveryReceiptOutput"))
	{
		AckText* ack = getExpectedResponse(monitor, origPdu, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["deliveryReceiptOutput"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["deliveryReceiptOutput"]);
	__require__(ack);
	if (!ack->valid)
	{
		__trace__("monitor is not valid");
		monitor->setReceived();
		return;
	}
	__tc__("processDeliverySm.deliveryReport.deliveryReceipt.checkStatus");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.get_optional().get_userMessageReference());
	opt.set_receiptedMessageId(monitor->pduData->smsId.c_str());
	uint8_t errCode[3];
	*errCode = 3; //GSM
	*((uint16_t*) (errCode + 1)) = rand0(65535);
	uint8_t regDelivery =
		SmppTransmitterTestCases::getRegisteredDelivery(monitor->pduData);
	switch(monitor->deliveryFlag)
	{
		case PDU_REQUIRED_FLAG:
		case PDU_MISSING_ON_TIME_FLAG:
			__tc_fail__(1);
			break;
		case PDU_RECEIVED_FLAG:
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
		case PDU_NOT_EXPECTED_FLAG:
			__tc_fail__(4);
			break;
		case PDU_EXPIRED_FLAG:
			opt.set_messageState(SMPP_EXPIRED_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		case PDU_ERROR_FLAG:
			opt.set_messageState(SMPP_UNDELIVERABLE_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		default:
			__unreachable__("Invalid flag");
	}
	__trace2__("expected optional part:\n%s", str(opt).c_str());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 0);
	__tc_ok_cond__;
	__tc__("processDeliverySm.deliveryReport.deliveryReceipt.checkText");
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
			__tc__("processDeliverySm.deliveryReport.multipleMessages");
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
	//������������ pdu
	__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* origPdu =
		reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
	__require__(origPdu);
	//������������
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().get_smLength(), pdu.get_message().get_dataCoding());
	if (!monitor->pduData->objProps.count("notificationOutput"))
	{
		AckText* ack = getExpectedResponse(monitor, origPdu, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["notificationOutput"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["notificationOutput"]);
	__require__(ack);
	if (!ack->valid)
	{
		__trace__("monitor is not valid");
		monitor->setReceived();
		return;
	}
	//��������� ���������� ���������� pdu
	__tc__("processDeliverySm.deliveryReport.intermediateNotification.checkStatus");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.get_optional().get_userMessageReference());
	opt.set_receiptedMessageId(monitor->pduData->smsId.c_str());
	uint8_t errCode[3];
	*errCode = 3; //GSM
	*((uint16_t*) (errCode + 1)) = rand0(65535);
	switch(monitor->deliveryFlag)
	{
		case PDU_REQUIRED_FLAG:
			opt.set_messageState(SMPP_ENROUTE_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		case PDU_MISSING_ON_TIME_FLAG:
			__tc_fail__(1);
			break;
		case PDU_RECEIVED_FLAG:
			__tc_fail__(2);
			break;
		case PDU_NOT_EXPECTED_FLAG:
			__tc_fail__(3);
			break;
		case PDU_EXPIRED_FLAG:
			//pdu ��� ��������� � ENROUTE, �� ��� �� ����� ������������
			opt.set_messageState(SMPP_ENROUTE_STATE);
			opt.set_networkErrorCode(errCode);
			break;
		case PDU_ERROR_FLAG:
			__tc_fail__(4);
			break;
		default:
			__unreachable__("Invalid flag");
	}
	__trace2__("expected optional part:\n%s", str(opt).c_str());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 0);
	__tc_ok_cond__;
	__tc__("processDeliverySm.deliveryReport.intermediateNotification.checkText");
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
			__tc__("processDeliverySm.deliveryReport.multipleMessages");
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

