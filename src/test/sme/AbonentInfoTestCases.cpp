#include "AbonentInfoTestCases.hpp"
#include "AbonentInfoMessages.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/core/PduUtil.hpp"
#include "test/core/ProfileUtil.hpp"
#include "test/util/TextUtil.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::test::conf::TestConfig;
using smsc::test::sms::SmsUtil;
using smsc::test::core::operator==;
using smsc::test::core::operator!=;
using smsc::test::sms::operator==;
using smsc::test::sms::operator!=;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::smpp::DataCoding;
using namespace smsc::profiler;
using namespace smsc::test::core;
using namespace smsc::test::smpp;
using namespace smsc::test::util;

AbonentInfoTestCases::AbonentInfoTestCases(SmppFixture* fixture)
: SmeAcknowledgementHandler(fixture,
	TestConfig::getStrParam("abonentInfoServiceType"),
	TestConfig::getIntParam("abonentInfoProtocolId"))
{
	__cfg_addr__(abonentInfoAddrSme);
	__cfg_addr__(abonentInfoAddrMobile);
	__cfg_addr__(abonentInfoAliasSme);
	__cfg_addr__(abonentInfoAliasMobile);
	addSmeAddr(abonentInfoAddrSme);
	addSmeAddr(abonentInfoAddrMobile);
	addSmeAlias(abonentInfoAliasSme);
	addSmeAlias(abonentInfoAliasMobile);
}

Category& AbonentInfoTestCases::getLog()
{
	static Category& log = Logger::getCategory("AbonentInfoTestCases");
	return log;
}

AbonentData* AbonentInfoTestCases::getAbonentData(const string& input)
{
	__cfg_int__(timeCheckAccuracy);
	try
	{
		Address abonentAlias(input.c_str());
		const Address abonentAddr = fixture->aliasReg->findAddressByAlias(abonentAlias);
		//profile
		time_t t;
		const Profile& abonentProfile =
			fixture->profileReg->getProfile(abonentAddr, t);
		//status
		SmeType smeType = fixture->routeChecker->isDestReachable(
			fixture->smeAddr, abonentAlias);
		return new AbonentData(input, abonentAddr, smeType, abonentProfile,
			time(NULL) > t + timeCheckAccuracy);
	}
	catch (...)
	{
		return NULL;
	}
}

void AbonentInfoTestCases::sendAbonentInfoPdu(const string& input,
	bool sync, uint8_t dataCoding, bool correct)
{
	__decl_tc__;
	try
	{
		//��������� ������ ���������
		switch (dataCoding)
		{
			case DEFAULT:
				__tc__("queryAbonentInfo.cmdTextDefault"); __tc_ok__;
				break;
			case SMSC7BIT:
				__tc__("queryAbonentInfo.cmdText7bit"); __tc_ok__;
				break;
			case UCS2:
				__tc__("queryAbonentInfo.cmdTextUcs2"); __tc_ok__;
				break;
			default:
				__unreachable__("Invalid data coding"); __tc_ok__;
		}
		uint8_t dcs = dataCoding;
		if (fixture->smeInfo.forceDC)
		{
			if (dataCoding == DEFAULT)
			{
				dataCoding = SMSC7BIT;
			}
			uint8_t dc;
			bool simMsg;
			do
			{
				dcs = rand0(255);
				SmppUtil::extractDataCoding(dcs, dc, simMsg);
			}
			while (dc != dataCoding);
		}
		//���������������
		int msgLen;
		auto_ptr<char> msg = encode(input, dataCoding, msgLen, false);
		//�����
		__cfg_addr__(abonentInfoAliasSme);
		__cfg_addr__(abonentInfoAliasMobile);
		Address abonentInfoAlias;
		switch (rand1(2))
		{
			case 1:
				__tc__("queryAbonentInfo.smppAddr"); __tc_ok__;
				abonentInfoAlias = abonentInfoAliasSme;
				break;
			case 2:
				__tc__("queryAbonentInfo.mapAddr"); __tc_ok__;
				abonentInfoAlias = abonentInfoAliasMobile;
				break;
			default:
				__unreachable__("Invalid address");
		}
		//���������� props
		PduData::ObjProps objProps;
        AbonentData* abonentData = getAbonentData(input);
		if (abonentData)
		{
			abonentData->ref();
			objProps["abonentInfoTc.abonentData"] = abonentData;
		}
		//��� ����������� �������� ������ abonent info �� ������� �����
		PduType pduType = correct ? PDU_EXT_SME : PDU_NULL_OK;
		if (rand0(1))
		{
			__tc__("queryAbonentInfo.submitSm");
			PduSubmitSm* pdu = new PduSubmitSm();
			//��������� short_message & message_payload
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu,
				abonentInfoAlias, false, OPT_ALL & ~OPT_MSG_PAYLOAD);
			//���������� ����������� ��������
			pdu->get_message().set_esmClass(0x0); //����� abonent info �������
			pdu->get_message().set_scheduleDeliveryTime("");
			pdu->get_message().set_dataCoding(dcs);
			if (rand0(1))
			{
				pdu->get_message().set_shortMessage(msg.get(), msgLen);
			}
			else
			{
				pdu->get_optional().set_messagePayload(msg.get(), msgLen);
			}
			fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync,
				NULL, NULL, &objProps, pduType);
		}
		else
		{
			__tc__("queryAbonentInfo.dataSm");
			PduDataSm* pdu = new PduDataSm();
			//��������� short_message & message_payload
			fixture->transmitter->setupRandomCorrectDataSmPdu(pdu,
				abonentInfoAlias, OPT_ALL & ~OPT_MSG_PAYLOAD);
			//���������� ����������� ��������
			pdu->get_data().set_esmClass(0x0); //����� abonent info �������
			pdu->get_data().set_dataCoding(dcs);
			pdu->get_optional().set_messagePayload(msg.get(), msgLen);
			fixture->transmitter->sendDataSmPdu(pdu, NULL, sync,
				NULL, NULL, &objProps, pduType);
		}
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
		//throw;
	}
}

void AbonentInfoTestCases::queryAbonentInfoCorrect(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	int numAddr = 2;
	int numAddrFormat = 2;
	TCSelector s(num, numAddr * numAddrFormat);
	for (; s.check(); s++)
	{
		Address addr;
		switch (s.value1(numAddr))
		{
			case 1: //������������ �����
				{
					addr = *fixture->smeReg->getRandomAddress();
					const RouteInfo* routeInfo =
						fixture->routeChecker->getRouteInfoForNormalSms(fixture->smeAddr, addr);
					if (routeInfo && routeInfo->smeSystemId == "MAP_PROXY")
					{
						__tc__("queryAbonentInfo.correct.nonExistentAddr"); __tc_ok__;
						SmsUtil::setupRandomCorrectAddress(&addr);
					}
					else
					{
						__tc__("queryAbonentInfo.correct.existentAddr"); __tc_ok__;
					}
				}
				break;
			case 2: //�������������� �����
				__tc__("queryAbonentInfo.correct.nonExistentAddr"); __tc_ok__;
				SmsUtil::setupRandomCorrectAddress(&addr);
				break;
			default:
				__unreachable__("Invalid numAddr");
		}
		string input;
		switch (s.value2(numAddr))
		{
			case 1:
				__tc__("queryAbonentInfo.correct.unifiedAddrFormat"); __tc_ok__;
				input = str(addr);
				break;
			case 2:
				__tc__("queryAbonentInfo.correct.humanFormat"); __tc_ok__;
				input = SmsUtil::configString(addr);
				break;
			default:
				__unreachable__("Invalid numAddrFormat");
		}
		sendAbonentInfoPdu(input, sync, dataCoding, true);
	}
}

void AbonentInfoTestCases::queryAbonentInfoIncorrect(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 6);
	for (; s.check(); s++)
	{
		string input;
		bool correct = false;
		switch (s.value())
		{
			case 1: //������������ ������ ������
				__tc__("queryAbonentInfo.incorrect.addrFormat"); __tc_ok__;
				input = "+abc";
				break;
			case 2: //������������ ����� ������
				__tc__("queryAbonentInfo.incorrect.addrLength"); __tc_ok__;
				input = "+123456789012345678901234567890";
				correct = true; //����� ������ 20 ��������
				break;
			case 3: //������������ ������� � ������
				__tc__("queryAbonentInfo.incorrect.invalidSymbols"); __tc_ok__;
				input = "???";
				correct = true; //�����������
				break;
			case 4: //������ ����� � �������
				__tc__("queryAbonentInfo.incorrect.extraWords"); __tc_ok__;
				input = "+123 abc";
				correct = true; //����� ������ +123
				break;
			case 5: //�������� ������������ ton
				__tc__("queryAbonentInfo.incorrect.tonNpi"); __tc_ok__;
				input = ".1000.1.123";
				correct = true; //����� 1000 & 0xff
				break;
			case 6: //�������� ������������ npi
				__tc__("queryAbonentInfo.incorrect.tonNpi"); __tc_ok__;
				input = ".1.1000.123";
				correct = true; //����� 1000 & 0xff
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendAbonentInfoPdu(input, sync, dataCoding, correct);
	}
}

AckText* AbonentInfoTestCases::getExpectedResponse(SmeAckMonitor* monitor,
	const string& text, time_t recvTime)
{
	__require__(monitor);
	__require__(monitor->pduData->objProps.count("recipientData"));

	__decl_tc__;
	__cfg_addr__(abonentInfoAddrSme);
	__cfg_addr__(abonentInfoAddrMobile);
	__cfg_int__(timeCheckAccuracy);

	if (!monitor->pduData->objProps.count("abonentInfoTc.abonentData"))
	{
		return new AckText("", DEFAULT, false);
	}
	SenderData* senderData =
		dynamic_cast<SenderData*>(monitor->pduData->objProps["senderData"]);
	RecipientData* recipientData =
		dynamic_cast<RecipientData*>(monitor->pduData->objProps["recipientData"]);
	AbonentData* abonentData =
		dynamic_cast<AbonentData*>(monitor->pduData->objProps["abonentInfoTc.abonentData"]);
	//����� ������� ������� �����������
	time_t t1;
	const Profile& profile = fixture->profileReg->getProfile(senderData->srcAddr, t1);
	bool valid = abonentData->validProfile && (t1 + timeCheckAccuracy < recvTime);
	if (!valid)
	{
		return new AckText("", DEFAULT, false);
	}
	//��������� ��������� �������
	time_t t2;
	const Profile& abonentProfile = fixture->profileReg->getProfile(abonentData->addr, t2);
	if (abonentProfile != abonentData->profile || recvTime <= t2 + timeCheckAccuracy)
	{
		return new AckText("", DEFAULT, false);
	}
	if (recipientData->destAddr == abonentInfoAddrSme)
	{
		const pair<string, uint8_t> p = AbonentInfoSmeMessage::format(
			profile, abonentData->input, abonentProfile, abonentData->status, "");
		__trace2__("getExpectedResponse(): input = %s, output = %s", abonentData->input.c_str(), p.first.c_str());
		return new AckText(p.first, p.second, valid);
	}
	if (recipientData->destAddr == abonentInfoAddrMobile)
	{
		const pair<string, uint8_t> p = AbonentInfoMobileMessage::format(
				profile, abonentData->input, abonentProfile, abonentData->status, "");
		__trace2__("getExpectedResponse(): input = %s, output = %s", abonentData->input.c_str(), p.first.c_str());
		return new AckText(p.first, p.second, valid);
	}
	__unreachable__("Invalid address");
}

#define __check__(errCode, cond) \
	if (!(cond)) { __tc_fail__(errCode); }

void AbonentInfoTestCases::processSmeAcknowledgement(SmeAckMonitor* monitor,
	SmppHeader* header, time_t recvTime)
{
	__decl_tc__;
	SmsPduWrapper pdu(header, 0);
	__require__(pdu.isDeliverSm());
	//������������
	const char* msg;
	int msgLen;
	if (pdu.get_optional().has_messagePayload())
	{
		msg = pdu.get_optional().get_messagePayload();
		msgLen = pdu.get_optional().size_messagePayload();
	}
	else
	{
		msg = pdu.get_message().get_shortMessage();
		msgLen = pdu.get_message().size_shortMessage();
	}
	const string text = decode(msg, msgLen, pdu.getDataCoding(), false);
	if (!monitor->pduData->objProps.count("abonentInfoTc.output"))
	{
		AckText* ack = getExpectedResponse(monitor, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["abonentInfoTc.output"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["abonentInfoTc.output"]);
	__require__(ack);
	if (!ack->valid)
	{
		__trace__("monitor is not valid");
		monitor->setNotExpected();
		return;
	}
	//��������� ���������� ���������� pdu
	__tc__("queryAbonentInfo.ack.checkFields");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.getMsgRef());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 0);
	__tc_ok_cond__;

	__trace2__("abonent info cmd: input:\n%s\noutput:\n%s\nexpected:\n%s\n",
		monitor->pduData->strProps["abonentInfoTc.input"].c_str(), text.c_str(), ack->text.c_str());

	__tc__("queryAbonentInfo.ack.checkText");
	__check__(1, pdu.getDataCoding() == ack->dataCoding);
	__check__(2, text == ack->text);
	__tc_ok_cond__;
	monitor->setNotExpected();
}

}
}
}

