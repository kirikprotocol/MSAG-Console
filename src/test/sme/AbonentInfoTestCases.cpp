#include "AbonentInfoTestCases.hpp"
#include "AbonentInfoMessages.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/core/PduUtil.hpp"
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

AckText* AbonentInfoTestCases::getExpectedResponse(const string& input,
	const Address& smeAddr, time_t submitTime)
{
	__decl_tc__;
	__cfg_addr__(abonentInfoAliasSme);
	__cfg_addr__(abonentInfoAliasMobile);
	__cfg_int__(timeCheckAccuracy);
	try
	{
		Address destAlias(input.c_str());
		const Address destAddr = fixture->aliasReg->findAddressByAlias(destAlias);
		//status
		int status = 0;
		SmeType smeType = fixture->routeChecker->isDestReachable(
			fixture->smeAddr, destAlias);
		switch (smeType)
		{
			case SME_RECEIVER:
			case SME_TRANSMITTER:
			case SME_TRANSCEIVER:
				status = 1;
				break;
		}
		//profile
		time_t t;
		const Profile& profile = fixture->profileReg->getProfile(destAddr, t);
		bool valid = t + timeCheckAccuracy <= submitTime;
		if (smeAddr == abonentInfoAliasSme)
		{
			const pair<string, uint8_t> p =
				AbonentInfoSmeMessage::format(profile, input, status, "");
			__trace2__("getExpectedResponse(): input = %s, output = %s", input.c_str(), p.first.c_str());
			return new AckText(p.first, p.second, valid);
		}
		else if (smeAddr == abonentInfoAliasMobile)
		{
			const pair<string, uint8_t> p =
				AbonentInfoMobileMessage::format(profile, input, status, "");
			__trace2__("getExpectedResponse(): input = %s, output = %s", input.c_str(), p.first.c_str());
			return new AckText(p.first, p.second, valid);
		}
		__unreachable__("Invalid address");
	}
	catch (...)
	{
		__trace2__("getExpectedResponse(): input = %s, ack = NULL", input.c_str());
		return NULL;
	}
}

void AbonentInfoTestCases::sendAbonentInfoPdu(const string& input,
	bool sync, uint8_t dataCoding, bool correct)
{
	__decl_tc__;
	try
	{
		//����� ���������
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
		PduData::StrProps strProps;
		strProps["abonentInfoTc.input"] = input;
		PduData::ObjProps objProps;
		//�� ������ ������ ������ ���������
		AckText* ack = getExpectedResponse(input, abonentInfoAlias, time(NULL));
		if (correct)
		{
			__require__(ack);
		}
		if (ack)
		{
			ack->ref();
			objProps["abonentInfoTc.output"] = ack;
		}
		//��� ����������� �������� ������ abonent info �� ������� �����
		PduType pduType = ack ? PDU_EXT_SME : PDU_NULL_OK;
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
			pdu->get_message().set_dataCoding(dataCoding);
			if (rand0(1))
			{
				pdu->get_message().set_shortMessage(msg.get(), msgLen);
			}
			else
			{
				pdu->get_optional().set_messagePayload(msg.get(), msgLen);
			}
			fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync,
				NULL, &strProps, &objProps, pduType);
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
			pdu->get_data().set_dataCoding(dataCoding);
			pdu->get_optional().set_messagePayload(msg.get(), msgLen);
			fixture->transmitter->sendDataSmPdu(pdu, NULL, sync,
				NULL, &strProps, &objProps, pduType);
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

//�� ������������ ��-�� ������ ������ ������ abonent info
AckText* AbonentInfoTestCases::getExpectedResponse(SmeAckMonitor* monitor,
	const string& text, time_t recvTime)
{
	__require__(monitor);
	__cfg_int__(timeCheckAccuracy);
	const string input = monitor->pduData->strProps["abonentInfoTc.input"];
	Address alias(input.c_str());
	const Address addr = fixture->aliasReg->findAddressByAlias(alias);
	//profile
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(addr, t);
	bool valid = t + timeCheckAccuracy <= recvTime;
	ostringstream s;
	//������ �������� ������ 1
	s << input << ":1," << profile.codepage;
	return new AckText(s.str(), DEFAULT, valid);
}

#define __check__(errCode, cond) \
	if (!(cond)) { __tc_fail__(errCode); }

void AbonentInfoTestCases::processSmeAcknowledgement(SmeAckMonitor* monitor,
	SmppHeader* header, time_t recvTime)
{
	__decl_tc__;
	SmsPduWrapper pdu(header, 0);
	__require__(pdu.isDeliverSm() && pdu.get_message().size_shortMessage());
	//������������
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().size_shortMessage(), pdu.getDataCoding(), false);
	if (!monitor->pduData->objProps.count("abonentInfoTc.output"))
	{
		__unreachable__("specific to abonent info internals");
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

