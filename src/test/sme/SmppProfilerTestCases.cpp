#include "SmppProfilerTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::test::conf::TestConfig;
using namespace smsc::profiler; //Profile, constants
using namespace smsc::test::core; //constants
using namespace smsc::test::util;
using namespace smsc::test::smpp; //constants, SmppUtil

SmppProfilerTestCases::SmppProfilerTestCases(SmppFixture* fixture)
: SmeAcknowledgementHandler(fixture,
	TestConfig::getStrParam("profilerServiceType"),
	TestConfig::getIntParam("profilerProtocolId"))
{
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	addSmeAddr(profilerAddr);
	addSmeAlias(profilerAlias);
}

Category& SmppProfilerTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppProfilerReceiverTestCases");
	return log;
}

void SmppProfilerTestCases::sendUpdateProfilePdu(const string& text,
	PduData::IntProps* intProps, PduData::StrProps* strProps,
	PduData::ObjProps* objProps, bool sync, uint8_t dataCoding)
{
	__decl_tc__;
	try
	{
		//создать pdu
		PduSubmitSm* pdu = new PduSubmitSm();
		__cfg_addr__(profilerAlias);
		fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, profilerAlias,
			OPT_ALL & ~OPT_MSG_PAYLOAD); //отключить messagePayload
		//установить немедленную доставку
		pdu->get_message().set_esmClass(0x0); //иначе профайлер отлупит
		pdu->get_message().set_scheduleDeliveryTime("");
		//текст сообщения
		switch (dataCoding)
		{
			case DATA_CODING_SMSC_DEFAULT:
				__tc__("updateProfile.cmdTextDefault");
				break;
			case DATA_CODING_UCS2:
				__tc__("updateProfile.cmdTextUcs2");
				break;
			default:
				__unreachable__("Invalid data coding");
		}
		int msgLen;
		auto_ptr<char> msg = encode(text, dataCoding, msgLen);
		pdu->get_message().set_shortMessage(msg.get(), msgLen);
		pdu->get_message().set_dataCoding(dataCoding);
		//отправить pdu
		fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, intProps, NULL, NULL, PDU_EXT_SME);
		__tc_ok__;
		//обновить профиль, ответные сообщения от профайлера и
		//подтверждения доставки  уже по новым настройкам
		if (fixture->profileReg && intProps)
		{
			time_t t;
			Profile profile = fixture->profileReg->getProfile(fixture->smeAddr, t);
			if (intProps->count("reportOptions"))
			{
				profile.reportoptions = intProps->find("reportOptions")->second;
			}
			if (intProps->count("codePage"))
			{
				profile.codepage = intProps->find("codePage")->second;
			}
			fixture->profileReg->putProfile(fixture->smeAddr, profile);
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

void SmppProfilerTestCases::updateReportOptionsCorrect(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		try
		{
			string text;
			int cmdType = UPDATE_REPORT_OPTIONS;
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //report none
					__tc__("updateProfile.reportOptions.reportNoneMixedCase");
					text = "RePoRT NoNe";
					intProps["reportOptions"] = ProfileReportOptions::ReportNone;
					break;
				case 2: //report none
					__tc__("updateProfile.reportOptions.reportNoneSpaces");
					text = "  rEpOrt  nOnE  ";
					intProps["reportOptions"] = ProfileReportOptions::ReportNone;
					break;
				case 3: //report full
					__tc__("updateProfile.reportOptions.reportFullMixedCase");
					text = "RePoRT FuLL";
					intProps["reportOptions"] = ProfileReportOptions::ReportFull;
					break;
				case 4: //report full
					__tc__("updateProfile.reportOptions.reportFullSpaces");
					text = "  rEpOrt  fUll  ";
					intProps["reportOptions"] = ProfileReportOptions::ReportFull;
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendUpdateProfilePdu(text, &intProps, NULL, NULL, sync, dataCoding);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProfilerTestCases::updateCodePageCorrect(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		try
		{
			string text;
			int cmdType = UPDATE_CODE_PAGE;
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //ucs2 codepage
					__tc__("updateProfile.dataCoding.ucs2CodepageMixedCase");
					text = "uCS2";
					intProps["codePage"] = ProfileCharsetOptions::Ucs2;
					break;
				case 2: //usc2 codepage
					__tc__("updateProfile.dataCoding.ucs2CodepageSpaces");
					text = "  Ucs2  ";
					intProps["codePage"] = ProfileCharsetOptions::Ucs2;
					break;
				case 3: //default codepage
					__tc__("updateProfile.dataCoding.defaultCodepageMixedCase");
					text = "DeFauLT";
					intProps["codePage"] = ProfileCharsetOptions::Default;
					break;
				case 4: //default codepage
					__tc__("updateProfile.dataCoding.defaultCodepageSpaces");
					text = "  dEfAUlt  ";
					intProps["codePage"] = ProfileCharsetOptions::Default;
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendUpdateProfilePdu(text, &intProps, NULL, NULL, sync, dataCoding);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppProfilerTestCases::updateProfileIncorrect(bool sync, uint8_t dataCoding)
{
	__decl_tc__;
	__tc__("updateProfile.incorrectCmdText");
	try
	{
		PduData::IntProps intProps;
		intProps["incorrectCmdText"] = 1;
		sendUpdateProfilePdu("Cmd Text", &intProps, NULL, NULL, sync, dataCoding);
		__tc_ok__;

	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

#define __get_resp__(param, codepage, valid) \
	switch (codepage) { \
		case ProfileCharsetOptions::Default: { \
			static const pair<string, uint8_t> p = convert(param, codepage); \
			return new AckText(p.first, p.second, valid); \
		} case ProfileCharsetOptions::Ucs2: { \
			static const pair<string, uint8_t> p = convert(param, codepage); \
			return new AckText(p.first, p.second, valid); \
		} default: \
			__unreachable__("Invalid codepage"); \
	}

AckText* SmppProfilerTestCases::getExpectedResponse(SmeAckMonitor* monitor,
	PduDeliverySm &pdu, time_t recvTime)
{
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	Address addr;
	SmppUtil::convert(pdu.get_message().get_dest(), &addr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(addr, t);
	bool valid = t + timeCheckAccuracy <= recvTime;
	//проверка profiler reportOptions
	if (monitor->pduData->intProps.count("reportOptions"))
	{
		__tc__("processUpdateProfile.reportOptions.dataCoding"); __tc_ok__;
		__cfg_str__(profilerRespReportNone);
		__cfg_str__(profilerRespReportFull);
		switch (monitor->pduData->intProps.find("reportOptions")->second)
		{
			case ProfileReportOptions::ReportNone:
				__get_resp__(profilerRespReportNone, profile.codepage, valid);
				//break;
			case ProfileReportOptions::ReportFull:
				__get_resp__(profilerRespReportFull, profile.codepage, valid);
				//break;
			default:
				__unreachable__("Invalid reportoptions");
		}
	}
	//проверка profiler codePage
	else if (monitor->pduData->intProps.count("codePage"))
	{
		__tc__("processUpdateProfile.codePage.dataCoding"); __tc_ok__;
		__cfg_str__(profilerRespDataCodingDefault);
		__cfg_str__(profilerRespDataCodingUcs2);
		switch (monitor->pduData->intProps.find("codePage")->second)
		{
			case ProfileCharsetOptions::Default:
				__get_resp__(profilerRespDataCodingDefault,
					ProfileCharsetOptions::Default, true);
				//break;
			case ProfileCharsetOptions::Ucs2:
				__get_resp__(profilerRespDataCodingUcs2,
					ProfileCharsetOptions::Ucs2, true);
				//break;
			default:
				__unreachable__("Invalid codepage");
		}
	}
	//неправильный текст команды
	else if (monitor->pduData->intProps.count("incorrectCmdText"))
	{
		__tc__("processUpdateProfile.incorrectCmdText.dataCoding"); __tc_ok__;
		__cfg_str__(profilerRespInvalidCmdText);
		__get_resp__(profilerRespInvalidCmdText, profile.codepage, valid);
		//return ...;
	}
	__unreachable__("Invalid sms was sent to profiler");
}

#define __check__(errCode, field, value) \
	if (value != pdu.get_message().get_##field()) { \
		__tc_fail__(errCode); \
	}

void SmppProfilerTestCases::processSmeAcknowledgement(SmeAckMonitor* monitor,
	PduDeliverySm& pdu, time_t recvTime)
{
	__require__(monitor);
	__decl_tc__;
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().get_smLength(), pdu.get_message().get_dataCoding());
	if (!monitor->pduData->objProps.count("profilerOutput"))
	{
		AckText* ack = getExpectedResponse(monitor, pdu, recvTime);
		ack->ref();
		monitor->pduData->objProps["profilerOutput"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["profilerOutput"]);
	__require__(ack);
	if (!ack->valid)
	{
		monitor->setReceived();
		return;
	}
	//проверить и обновить профиль
	__tc__("processUpdateProfile.checkFields");
	__check__(1, dataCoding, ack->dataCoding);
	if (text.length() > getMaxChars(ack->dataCoding))
	{
		__tc_fail__(2);
	}
    SmppOptional opt;
	opt.set_userMessageReference(pdu.get_optional().get_userMessageReference());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 10);
	__tc_ok_cond__;
	__tc__("processUpdateProfile.checkText");
	int pos = ack->text.find(text);
	__trace2__("profiler ack: pos = %d, received:\n%s\nexpected:\n%s\n",
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
			__tc__("processUpdateProfile.multipleMessages");
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

