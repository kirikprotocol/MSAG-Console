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

Category& SmscSmeTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmscSmeTestCases");
	return log;
}

AckText* SmscSmeTestCases::getExpectedResponse(DeliveryReceiptMonitor* monitor,
	PduSubmitSm* origPdu, const string& text, time_t recvTime)
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
		s << "¬аше сообщение отправленное по адресу ";
		s << SmsUtil::configString(destAlias);
		s << " wasn't delivered, reason: failed"; //failed захардкожено
		//s << monitor->deliveryStatus;
		expected = s.str();
	}
	//профиль
	Address srcAddr;
	SmppUtil::convert(origPdu->get_message().get_source(), &srcAddr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(srcAddr, t);
	bool valid = t + timeCheckAccuracy <= time(NULL);
	const pair<string, uint8_t> p = convert(expected, profile.codepage);
	return new AckText(p.first, p.second, valid);
}

#define __compare__(errCode, field, value) \
	if (value != pdu.field) { __tc_fail__(errCode); }

void SmscSmeTestCases::processDeliveryReceipt(DeliveryReceiptMonitor* monitor,
	PduDeliverySm& pdu, time_t recvTime)
{
	__decl_tc__;
	__cfg_addr__(smscAddr);
	__cfg_addr__(smscAlias);
	__cfg_str__(smscServiceType);
	__cfg_int__(smscProtocolId);

	//оригинальна€ pdu
	__require__(monitor->pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* origPdu =
		reinterpret_cast<PduSubmitSm*>(monitor->pduData->pdu);
	__require__(origPdu);
	//sme
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
		monitor->pduData->intProps["skipReceivedCheck"] = 1;
		monitor->setReceived();
		return;
	}
	//проверить содержимое полученной pdu
	__tc__("processDeliverySm.deliveryReceipt.checkFields");
	//пол€ хедера провер€ютс€ в processDeliverySm()
	//message
	__compare__(1, get_message().get_serviceType(), smscServiceType);
	//правильность адресов частично провер€етс€ в fixture->routeChecker->checkRouteForAcknowledgementSms()
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
	//__compare__(4, get_message().get_dest(), origPdu->get_message().get_source());
	switch (origPdu->get_message().get_registredDelivery() & SMSC_DELIVERY_RECEIPT_BITS)
	{
		case NO_SMSC_DELIVERY_RECEIPT:
		case SMSC_DELIVERY_RECEIPT_RESERVED:
			if (monitor->regDelivery == ProfileReportOptions::ReportFull)
			{
				__compare__(4, get_message().get_esmClass(), ESM_CLASS_NORMAL_MESSAGE);
			}
			else
			{
				__tc_fail__(5);
			}
			break;
		case FINAL_SMSC_DELIVERY_RECEIPT:
		case FAILURE_SMSC_DELIVERY_RECEIPT:
			__compare__(6, get_message().get_esmClass(), ESM_CLASS_DELIVERY_RECEIPT);
			break;
		default:
			__unreachable__("Invalid reg dilivery");
	}
	__compare__(7, get_message().get_protocolId(), smscProtocolId);
	__compare__(8, get_message().get_priorityFlag(), 0);
	__compare__(9, get_message().get_registredDelivery(), 0);
	__compare__(10, get_message().get_dataCoding(), ack->dataCoding);
	if (text.length() > getMaxChars(ack->dataCoding))
	{
		__tc_fail__(11);
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
	__trace2__("expected optional part:\n%s", str(opt).c_str());
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



}
}
}

