#include "SmppProfilerTestCases.hpp"
#include "ProfilerMessages.hpp"
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

void SmppProfilerTestCases::sendUpdateProfilePdu(const string& _text,
	PduData::IntProps* intProps, PduData::StrProps* strProps,
	PduData::ObjProps* objProps, bool sync, uint8_t dataCoding)
{
	__decl_tc__;
	__cfg_addr__(profilerAlias);
	string text(_text);
	try
	{
		//кодировка текста сообщения
		switch (dataCoding)
		{
			case DEFAULT:
				__tc__("updateProfile.cmdTextDefault"); __tc_ok__;
				break;
			case SMSC7BIT:
				__tc__("updateProfile.cmdText7bit"); __tc_ok__;
				break;
			case UCS2:
				__tc__("updateProfile.cmdTextUcs2"); __tc_ok__;
				break;
			default:
				__unreachable__("Invalid data coding");
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
		//модификации текста
		__tc__("updateProfile.cmdTextMixedCase"); __tc_ok__;
		mixedCase(text);
		__tc__("updateProfile.cmdTextExtraWhiteSpaces"); __tc_ok__;
		//static const char whiteSpace[] = {' ', '\n'};
		//static const int whiteSpaceSize = sizeof(whiteSpace);
		text.insert(0, string(rand0(2), ' '));
		int pos = text.find(' '); //первый пробел
		if (pos != string::npos)
		{
			text.replace(pos, 1, string(rand1(2), ' '));
		}
		text.append(string(rand0(2), ' '));
		//перекодирование
		int msgLen;
		auto_ptr<char> msg = encode(text, dataCoding, msgLen, false);
		//submit_sm
		if (rand0(1))
		{
			__tc__("updateProfile.submitSm");
			PduSubmitSm* pdu = new PduSubmitSm();
			//отключить short_message & message_payload
			fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, profilerAlias,
				false, OPT_ALL & ~OPT_MSG_PAYLOAD);
			//установить немедленную доставку
			pdu->get_message().set_esmClass(0x0); //иначе профайлер отлупит
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
			fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, intProps,
				strProps, objProps, PDU_EXT_SME);
		}
		else
		{
			__tc__("updateProfile.dataSm");
			PduDataSm* pdu = new PduDataSm();
			//отключить short_message & message_payload
			fixture->transmitter->setupRandomCorrectDataSmPdu(pdu, profilerAlias,
				OPT_ALL & ~OPT_MSG_PAYLOAD);
			//установить немедленную доставку
			pdu->get_data().set_esmClass(0x0); //иначе профайлер отлупит
			pdu->get_data().set_dataCoding(dcs);
			pdu->get_optional().set_messagePayload(msg.get(), msgLen);
			fixture->transmitter->sendDataSmPdu(pdu, NULL, sync, intProps,
				strProps, objProps, PDU_EXT_SME);
		}
		__tc_ok__;
		//обновить профиль, ответные сообщения от профайлера и
		//подтверждения доставки  уже по новым настройкам
		if (fixture->profileReg)
		{
			time_t t;
			Profile profile = fixture->profileReg->getProfile(fixture->smeAddr, t);
			if (intProps && intProps->count("profilerTc.reportOptions"))
			{
				profile.reportoptions = (*intProps)["profilerTc.reportOptions"];
			}
			if (intProps && intProps->count("profilerTc.codePage"))
			{
				profile.codepage = (*intProps)["profilerTc.codePage"];
			}
			if (strProps && strProps->count("profilerTc.locale") &&
				(*strProps)["profilerTc.locale"].length())
			{
				profile.locale = (*strProps)["profilerTc.locale"];
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
	//убрать нарезку длинных ответных сообщений от профайлера для map proxy
	if (fixture->smeInfo.systemId == "MAP_PROXY")
	{
		return;
	}
	TCSelector s(num, 2);
	for (; s.check(); s++)
	{
		try
		{
			string text;
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //report none
					__tc__("updateProfile.reportOptions.reportNone");
					text = "report none";
					intProps["profilerTc.reportOptions"] =
						ProfileReportOptions::ReportNone;
					break;
				case 2: //report full
					__tc__("updateProfile.reportOptions.reportFull");
					text = "report full";
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
	//убрать нарезку длинных ответных сообщений от профайлера для map proxy
	if (fixture->smeInfo.systemId == "MAP_PROXY")
	{
		return;
	}
	TCSelector s(num, 2);
	for (; s.check(); s++)
	{
		try
		{
			string text;
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //ucs2 codepage
					__tc__("updateProfile.dataCoding.ucs2");
					text = "ucs2";
					intProps["profilerTc.codePage"] = ProfileCharsetOptions::Ucs2;
					break;
				case 2: //default codepage
					__tc__("updateProfile.dataCoding.default");
					text = "default";
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

void SmppProfilerTestCases::updateLocaleCorrect(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	//убрать нарезку длинных ответных сообщений от профайлера для map proxy
	if (fixture->smeInfo.systemId == "MAP_PROXY")
	{
		return;
	}
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		try
		{
			string text;
			PduData::StrProps strProps;
			switch (s.value())
			{
				case 1: //ru_ru
					__tc__("updateProfile.locale.existentLocale");
					text = "locale ru_ru";
					strProps["profilerTc.locale"] = "ru_ru";
					break;
				case 2: //en_us
					__tc__("updateProfile.locale.existentLocale");
					text = "locale en_us";
					strProps["profilerTc.locale"] = "en_us";
					break;
				case 3: //en_gb
					__tc__("updateProfile.locale.existentLocale");
					text = "locale en_gb";
					strProps["profilerTc.locale"] = "en_gb";
					break;
				case 4: //не существующий
					__tc__("updateProfile.locale.nonExistentLocale");
					text = "locale qu_qu";
					strProps["profilerTc.locale"] = "";
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendUpdateProfilePdu(text, NULL, &strProps, NULL, sync, dataCoding);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

template<typename T, int size>
inline int sz(T (&)[size]) { return size; }

void SmppProfilerTestCases::updateProfileIncorrect(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	//убрать нарезку длинных ответных сообщений от профайлера для map proxy
	if (fixture->smeInfo.systemId == "MAP_PROXY")
	{
		return;
	}
	__tc__("updateProfile.incorrectCmdText");
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		try
		{
			static const string invalidReportCmd[] =
			{
				"report", "report non", "report ful", "repor none"
				//"report none2", "report full2", "report2 none"
			};
			static const string invalidDcCmd[] =
			{
				"ucs", "ucs1", "defaul"
				//"ucs22", "default2"
			};
			static const string invalidLocaleCmd[] =
			{
				"locale", "local en_us"
				//"locale2 en_us"
			};
			string text;
			switch (s.value())
			{
				case 1: //случайный текст
					{
						auto_ptr<char> tmp = rand_char(rand0(5));
						text = tmp.get();
					}
					break;
				case 2: //report options
					text = invalidReportCmd[rand0(sz(invalidReportCmd) - 1)];
					break;
				case 3: //data coding
					text = invalidDcCmd[rand0(sz(invalidDcCmd) - 1)];
					break;
				case 4: //locale
					text = invalidLocaleCmd[rand0(sz(invalidLocaleCmd) - 1)];
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendUpdateProfilePdu(text, NULL, NULL, NULL, sync, dataCoding);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

AckText* SmppProfilerTestCases::getExpectedResponse(SmeAckMonitor* monitor,
	SmppHeader* header, time_t recvTime)
{
	__require__(monitor->pduData->objProps.count("senderData"));
	__decl_tc__;
	__cfg_int__(timeCheckAccuracy);
	SmsPduWrapper pdu(header, 0);
	//данные отправителя со старым профилем
	SenderData* senderData =
		dynamic_cast<SenderData*>(monitor->pduData->objProps["senderData"]);
	Profile profile = senderData->profile;
	bool valid = senderData->validProfile;
	//проапдейтить профиль получателя
	if (monitor->pduData->intProps.count("profilerTc.reportOptions"))
	{
		profile.reportoptions = monitor->pduData->intProps["profilerTc.reportOptions"];
	}
	if (monitor->pduData->intProps.count("profilerTc.codePage"))
	{
		profile.codepage = monitor->pduData->intProps["profilerTc.codePage"];
	}
	if (monitor->pduData->strProps.count("profilerTc.locale") &&
		monitor->pduData->strProps["profilerTc.locale"].length())
	{
		profile.locale = monitor->pduData->strProps["profilerTc.locale"];
	}
	//проверка profiler reportOptions
	if (monitor->pduData->intProps.count("profilerTc.reportOptions"))
	{
		__tc__("updateProfile.ack.reportOptions.dataCoding"); __tc_ok__;
		const pair<string, uint8_t> p = ProfilerReportMessage::format(profile);
		return new AckText(p.first, p.second, true /*valid*/);
	}
	//проверка profiler codePage
	if (monitor->pduData->intProps.count("profilerTc.codePage"))
	{
		__tc__("updateProfile.ack.codePage.dataCoding"); __tc_ok__;
		const pair<string, uint8_t> p = ProfilerCodepageMessage::format(profile);
		return new AckText(p.first, p.second, true /*valid*/);
	}
	//проверка profiler locale
	if (monitor->pduData->strProps.count("profilerTc.locale"))
	{
		__tc__("updateProfile.ack.locale.dataCoding"); __tc_ok__;
		const pair<string, uint8_t> p = ProfilerLocaleMessage::format(profile,
			monitor->pduData->strProps["profilerTc.locale"]);
		return new AckText(p.first, p.second, true /*valid*/);
	}
	//неправильный текст команды
	__tc__("updateProfile.ack.incorrectCmdText.dataCoding"); __tc_ok__;
	const pair<string, uint8_t> p = ProfilerErrorMessage::format(profile);
	return new AckText(p.first, p.second, true /*valid*/);
}

#define __check__(errCode, cond) \
	if (!(cond)) { __tc_fail__(errCode); }

void SmppProfilerTestCases::processSmeAcknowledgement(SmeAckMonitor* monitor,
	SmppHeader* header, time_t recvTime)
{
	__require__(monitor);
	__decl_tc__;
	if (monitor->getFlag() == PDU_COND_REQUIRED_FLAG)
	{
		return;
	}
	SmsPduWrapper pdu(header, 0);
	__require__(pdu.isDeliverSm());
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
	if (!monitor->pduData->objProps.count("profilerTc.output"))
	{
		AckText* ack = getExpectedResponse(monitor, header, recvTime);
		ack->ref();
		monitor->pduData->objProps["profilerTc.output"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["profilerTc.output"]);
	__require__(ack);
	if (!ack->valid)
	{
		__trace__("monitor is not valid");
		monitor->setNotExpected();
		return;
	}
	//зафиксировать время изменения профиля
	fixture->profileReg->setProfileUpdateTime(fixture->smeAddr, 0);
	//проверить и обновить профиль
	__tc__("updateProfile.ack.checkFields");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.getMsgRef());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional(), OPT_MSG_PAYLOAD), 0);
	__tc_ok_cond__;

	__trace2__("profiler ack text: output:\n%s\nexpected:\n%s\n",
		text.c_str(), ack->text.c_str());

	__tc__("updateProfile.ack.checkText");
	__check__(1, pdu.getDataCoding() == ack->dataCoding);
	__check__(2, text == ack->text);
	__tc_ok_cond__;
	monitor->setNotExpected();
}

}
}
}

