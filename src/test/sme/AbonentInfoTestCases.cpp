#include "AbonentInfoTestCases.hpp"
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
	__cfg_addr__(abonentInfoAddr);
	__cfg_addr__(abonentInfoAlias);
	addSmeAddr(abonentInfoAddr);
	addSmeAlias(abonentInfoAlias);
}

Category& AbonentInfoTestCases::getLog()
{
	static Category& log = Logger::getCategory("AbonentInfoTestCases");
	return log;
}

AckText* AbonentInfoTestCases::getExpectedResponse(const string& input,
	time_t submitTime)
{
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	try
	{
		Address alias(input.c_str());
		const Address addr = fixture->aliasReg->findAddressByAlias(alias);
		//status
		bool bound = fixture->smeReg->isSmeBound(addr);
		//profile
		time_t t;
		const Profile& profile = fixture->profileReg->getProfile(addr, t);
		bool valid = t + timeCheckAccuracy <= submitTime;
		ostringstream s;
		__tc__("processAbonentInfo.checkText");
		__tc_fail__(-1); //������ �������� ������ 1
		s << input << ":1," << profile.codepage;
		return new AckText(s.str(), DEFAULT, valid);
	}
	catch (...)
	{
		return NULL;
	}
}

void AbonentInfoTestCases::sendAbonentInfoPdu(const string& input,
	bool sync, uint8_t dataCoding)
{
	__decl_tc__;
	try
	{
		//������� pdu
		PduSubmitSm* pdu = new PduSubmitSm();
		__cfg_addr__(abonentInfoAlias);
		fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, abonentInfoAlias,
			OPT_ALL & ~OPT_MSG_PAYLOAD); //��������� messagePayload
		//���������� ����������� ��������
		pdu->get_message().set_esmClass(0x0); //����� abonent info �������
		pdu->get_message().set_scheduleDeliveryTime("");
		//����� ���������
		switch (dataCoding)
		{
			case DEFAULT:
				__tc__("queryAbonentInfo.cmdTextDefault");
				break;
			case UCS2:
				__tc__("queryAbonentInfo.cmdTextUcs2");
				break;
			default:
				__unreachable__("Invalid data coding");
		}
		int msgLen;
		auto_ptr<char> msg = encode(input, dataCoding, msgLen);
		pdu->get_message().set_shortMessage(msg.get(), msgLen);
		pdu->get_message().set_dataCoding(dataCoding);
		//��������� pdu
		PduData::StrProps strProps;
		strProps["abonentInfoInput"] = input;
		PduData::ObjProps objProps;
		AckText* ack = getExpectedResponse(input, time(NULL));
		if (ack)
		{
			ack->ref();
			objProps["abonentInfoOutput"] = ack;
		}
		//��� ����������� �������� ������ abonent info �� ������� �����
		PduType pduType = ack ? PDU_EXT_SME : PDU_NULL_OK;
		fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync,
			NULL, &strProps, &objProps, pduType);
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
				__tc__("queryAbonentInfo.correct.existentAddr"); __tc_ok__;
				addr = *fixture->smeReg->getRandomAddress();
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
		sendAbonentInfoPdu(input, sync, dataCoding);
	}
}

void AbonentInfoTestCases::queryAbonentInfoIncorrect(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		string input;
		switch (s.value())
		{
			case 1: //������������ ������ ������
				__tc__("queryAbonentInfo.incorrect.addrFormat"); __tc_ok__;
				input = "+abc";
				break;
			case 2: //������������ ����� ������
				__tc__("queryAbonentInfo.incorrect.addrLength"); __tc_ok__;
				input = "+123456789012345678901234567890";
				break;
			case 3: //������������ ������� � ������
				__tc__("queryAbonentInfo.incorrect.invalidSymbols"); __tc_ok__;
				input = "???";
				break;
			case 4: //������ ����� � �������
				__tc__("queryAbonentInfo.incorrect.extraWords"); __tc_ok__;
				input = "+123 abc";
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendAbonentInfoPdu(input, sync, dataCoding);
	}
}

//�� ������������ ��-�� ������ ������ ������ abonent info
AckText* AbonentInfoTestCases::getExpectedResponse(SmeAckMonitor* monitor,
	const string& text, time_t recvTime)
{
	__require__(monitor);
	__cfg_int__(timeCheckAccuracy);
	const string input = monitor->pduData->strProps["abonentInfoInput"];
	Address alias(input.c_str());
	const Address addr = fixture->aliasReg->findAddressByAlias(alias);
	//status
	bool bound = fixture->smeReg->isSmeBound(addr);
	//profile
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(addr, t);
	bool valid = t + timeCheckAccuracy <= recvTime;
	ostringstream s;
	s << input << ":" << (bound ? 1 : 0) << "," << profile.codepage;
	return new AckText(s.str(), DEFAULT, valid);
}

#define __compare__(errCode, field, value) \
	if (value != pdu.field) { __tc_fail__(errCode); }

void AbonentInfoTestCases::processSmeAcknowledgement(SmeAckMonitor* monitor,
	PduDeliverySm& pdu, time_t recvTime)
{
	__decl_tc__;
	//������������
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().get_smLength(), pdu.get_message().get_dataCoding());
	if (!monitor->pduData->objProps.count("abonentInfoOutput"))
	{
		__unreachable__("specific to abonent info internals");
		AckText* ack = getExpectedResponse(monitor, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["abonentInfoOutput"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["abonentInfoOutput"]);
	__require__(ack);
	if (!ack->valid)
	{
		__trace__("monitor is not valid");
		monitor->setReceived();
		return;
	}
	//��������� ���������� ���������� pdu
	__tc__("processAbonentInfo.checkFields");
	__compare__(1, get_message().get_dataCoding(), ack->dataCoding);
	if (text.length() > getMaxChars(ack->dataCoding))
	{
		__tc_fail__(2);
	}
	__tc_ok_cond__;
	__tc__("processAbonentInfo.checkText");
	__trace2__("abonent info cmd: input:\n%s\noutput:\n%s\nexpected:\n%s\n",
		monitor->pduData->strProps["abonentInfoInput"].c_str(), text.c_str(), ack->text.c_str());
	if (text != ack->text)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
}

}
}
}

