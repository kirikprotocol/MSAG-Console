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
using namespace smsc::test::smpp::DataCoding; //constants

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
		//отключить short_message & message_payload
		fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, profilerAlias,
			false, OPT_ALL & ~OPT_MSG_PAYLOAD);
		//установить немедленную доставку
		pdu->get_message().set_esmClass(0x0); //иначе профайлер отлупит
		pdu->get_message().set_scheduleDeliveryTime("");
		//текст сообщения
		switch (dataCoding)
		{
			case DEFAULT:
				__tc__("updateProfile.cmdTextDefault");
				break;
			case UCS2:
				__tc__("updateProfile.cmdTextUcs2");
				break;
			default:
				__unreachable__("Invalid data coding");
		}
		int msgLen;
		auto_ptr<char> msg = encode(text, dataCoding, msgLen, false);
		pdu->get_message().set_shortMessage(msg.get(), msgLen);
		pdu->get_message().set_dataCoding(dataCoding);
		//отправить pdu
		fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, intProps,
			strProps, objProps, PDU_EXT_SME);
		__tc_ok__;
		//обновить профиль, ответные сообщения от профайлера и
		//подтверждения доставки  уже по новым настройкам
		if (fixture->profileReg && intProps)
		{
			time_t t;
			Profile profile = fixture->profileReg->getProfile(fixture->smeAddr, t);
			if (intProps->count("profilerTc.reportOptions"))
			{
				__require__(!intProps->count("profilerTc.codePage"));
				profile.reportoptions = (*intProps)["profilerTc.reportOptions"];
			}
			if (intProps->count("profilerTc.codePage"))
			{
				__require__(!intProps->count("profilerTc.reportOptions"));
				profile.codepage = (*intProps)["profilerTc.codePage"];
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
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //report none
					__tc__("updateProfile.reportOptions.reportNoneMixedCase");
					text = "RePoRT NoNe";
					intProps["profilerTc.reportOptions"] =
						ProfileReportOptions::ReportNone;
					break;
				case 2: //report none
					__tc__("updateProfile.reportOptions.reportNoneSpaces");
					text = "  rEpOrt  nOnE  ";
					intProps["profilerTc.reportOptions"] =
						ProfileReportOptions::ReportNone;
					break;
				case 3: //report full
					__tc__("updateProfile.reportOptions.reportFullMixedCase");
					text = "RePoRT FuLL";
					intProps["profilerTc.reportOptions"] =
						ProfileReportOptions::ReportFull;
					break;
				case 4: //report full
					__tc__("updateProfile.reportOptions.reportFullSpaces");
					text = "  rEpOrt  fUll  ";
					intProps["profilerTc.reportOptions"] =
						ProfileReportOptions::ReportFull;
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
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //ucs2 codepage
					__tc__("updateProfile.dataCoding.ucs2CodepageMixedCase");
					text = "uCS2";
					intProps["profilerTc.codePage"] = ProfileCharsetOptions::Ucs2;
					break;
				case 2: //usc2 codepage
					__tc__("updateProfile.dataCoding.ucs2CodepageSpaces");
					text = "  Ucs2  ";
					intProps["profilerTc.codePage"] = ProfileCharsetOptions::Ucs2;
					break;
				case 3: //default codepage
					__tc__("updateProfile.dataCoding.defaultCodepageMixedCase");
					text = "DeFauLT";
					intProps["profilerTc.codePage"] = ProfileCharsetOptions::Default;
					break;
				case 4: //default codepage
					__tc__("updateProfile.dataCoding.defaultCodepageSpaces");
					text = "  dEfAUlt  ";
					intProps["profilerTc.codePage"] = ProfileCharsetOptions::Default;
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
		intProps["profilerTc.incorrectCmdText"] = 1;
		auto_ptr<char> tmp = rand_char(rand0(5));
		sendUpdateProfilePdu(tmp.get(), &intProps, NULL, NULL, sync, dataCoding);
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
	if (monitor->pduData->intProps.count("profilerTc.reportOptions"))
	{
		__require__(!monitor->pduData->intProps.count("profilerTc.codePage"));
		__tc__("updateProfile.ack.reportOptions.dataCoding"); __tc_ok__;
		__cfg_str__(profilerRespReportNone);
		__cfg_str__(profilerRespReportFull);
		switch (monitor->pduData->intProps["profilerTc.reportOptions"])
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
	if (monitor->pduData->intProps.count("profilerTc.codePage"))
	{
		__require__(!monitor->pduData->intProps.count("profilerTc.reportOptions"));
		__tc__("updateProfile.ack.codePage.dataCoding"); __tc_ok__;
		__cfg_str__(profilerRespDataCodingDefault);
		__cfg_str__(profilerRespDataCodingUcs2);
		switch (monitor->pduData->intProps["profilerTc.codePage"])
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
	if (monitor->pduData->intProps.count("profilerTc.incorrectCmdText"))
	{
		__tc__("updateProfile.ack.incorrectCmdText.dataCoding"); __tc_ok__;
		__cfg_str__(profilerRespInvalidCmdText);
		__get_resp__(profilerRespInvalidCmdText, profile.codepage, valid);
		//return ...;
	}
	__unreachable__("Invalid sms was sent to profiler");
}

void SmppProfilerTestCases::processSmeAcknowledgement(SmeAckMonitor* monitor,
	PduDeliverySm& pdu, time_t recvTime)
{
	__require__(monitor);
	__decl_tc__;
	if (monitor->getFlag() == PDU_COND_REQUIRED_FLAG)
	{
		return;
	}
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().get_smLength(), pdu.get_message().get_dataCoding(), false);
	if (!monitor->pduData->objProps.count("profilerTc.output"))
	{
		AckText* ack = getExpectedResponse(monitor, pdu, recvTime);
		ack->ref();
		monitor->pduData->objProps["profilerTc.output"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["profilerTc.output"]);
	__require__(ack);
	if (ack->valid)
	{
		//зафиксировать время изменения профиля
		fixture->profileReg->setProfileUpdateTime(fixture->smeAddr, 0);
		//проверить и обновить профиль
		__tc__("updateProfile.ack.checkFields");
		SmppOptional opt;
		opt.set_userMessageReference(pdu.get_optional().get_userMessageReference());
		__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 0);
		__tc_ok_cond__;
		__tc__("updateProfile.ack.checkText");
		if (pdu.get_message().get_dataCoding() != ack->dataCoding)
		{
			__tc_fail__(1);
		}
		if (text != ack->text)
		{
			__trace2__("profiler ack text mismatch: received:\n%s\nexpected:\n%s\n",
				text.c_str(), ack->text.c_str());
			__tc_fail__(2);
		}
	}
	monitor->setNotExpected();
}

}
}
}

