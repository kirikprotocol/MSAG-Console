#include "SmppProfilerTestCases.hpp"
#include "test/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::test::TestConfig;
using namespace smsc::profiler; //Profile, constants
using namespace smsc::test::core; //constants
using namespace smsc::test::util;
using namespace smsc::test::smpp; //constants, SmppUtil

Category& SmppProfilerTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppProfilerReceiverTestCases");
	return log;
}

void SmppProfilerTestCases::sendUpdateProfilePdu(PduSubmitSm* pdu,
	const string& text, bool sync, uint8_t dataCoding, PduData::IntProps& intProps)
{
	__require__(pdu);
	__decl_tc__;
	try
	{
		switch (dataCoding)
		{
			case DATA_CODING_SMSC_DEFAULT:
				__tc__("updateProfileCorrect.cmdTextDefault");
				break;
			case DATA_CODING_UCS2:
				__tc__("updateProfileCorrect.cmdTextUcs2");
				break;
			default:
				__unreachable__("Invalid data coding");
		}
		string encText = encode(text.c_str(), dataCoding);
		pdu->get_message().set_shortMessage(encText.c_str(), encText.length());
		pdu->get_message().set_dataCoding(dataCoding);
		//��������� � ���������������� pdu
		transmitter->sendSubmitSmPdu(pdu, NULL, sync, &intProps, NULL, false);
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

void SmppProfilerTestCases::updateProfileCorrect(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 8);
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			__cfg_addr__(profilerAlias);
			transmitter->setupRandomCorrectSubmitSmPdu(pdu, profilerAlias);
			string text;
			int cmdType;
			PduData::IntProps intProps;
			switch (s.value())
			{
				case 1: //report none
					__tc__("updateProfile.reportOptions.reportNoneMixedCase");
					text = "RePoRT NoNe";
					intProps["reportOptions"] = ProfileReportOptions::ReportNone;
					cmdType = UPDATE_REPORT_OPTIONS;
					break;
				case 2: //report none
					__tc__("updateProfile.reportOptions.reportNoneSpaces");
					text = "  rEpOrt  nOnE  ";
					intProps["reportOptions"] = ProfileReportOptions::ReportNone;
					cmdType = UPDATE_REPORT_OPTIONS;
					break;
				case 3: //report full
					__tc__("updateProfile.reportOptions.reportFullMixedCase");
					text = "RePoRT FuLL";
					intProps["reportOptions"] = ProfileReportOptions::ReportFull;
					cmdType = UPDATE_REPORT_OPTIONS;
					break;
				case 4: //report full
					__tc__("updateProfile.reportOptions.reportFullSpaces");
					text = "  rEpOrt  fUll  ";
					intProps["reportOptions"] = ProfileReportOptions::ReportFull;
					cmdType = UPDATE_REPORT_OPTIONS;
					break;
				case 5: //ucs2 codepage
					__tc__("updateProfile.dataCoding.ucs2CodepageMixedCase");
					text = "uCS2";
					intProps["codePage"] = ProfileCharsetOptions::Ucs2;
					cmdType = UPDATE_CODE_PAGE;
					break;
				case 6: //usc2 codepage
					__tc__("updateProfile.dataCoding.ucs2CodepageSpaces");
					text = "  Ucs2  ";
					intProps["codePage"] = ProfileCharsetOptions::Ucs2;
					cmdType = UPDATE_CODE_PAGE;
					break;
				case 7: //default codepage
					__tc__("updateProfile.dataCoding.defaultCodepageMixedCase");
					text = "DeFauLT";
					intProps["codePage"] = ProfileCharsetOptions::Default;
					cmdType = UPDATE_CODE_PAGE;
					break;
				case 8: //default codepage
					__tc__("updateProfile.dataCoding.defaultCodepageSpaces");
					text = "  dEfAUlt  ";
					intProps["codePage"] = ProfileCharsetOptions::Default;
					cmdType = UPDATE_CODE_PAGE;
					break;
				default:
					__unreachable__("Invalid num");
			}
			//������ ���������, ��������� � ���������������� pdu
			if (fixture->profileReg)
			{
				time_t t;
				Profile profile = fixture->profileReg->getProfile(fixture->smeAddr, t);
				if (intProps.count("reportOptions"))
				{
					profile.reportoptions = intProps.find("reportOptions")->second;
				}
				if (intProps.count("codePage"))
				{
					profile.codepage = intProps.find("codePage")->second;
				}
				//���������� ����������� �������� � �������� profileReg
				pdu->get_message().set_scheduleDeliveryTime("");
				fixture->profileReg->putProfile(fixture->smeAddr, profile);
			}
			sendUpdateProfilePdu(pdu, text, sync, dataCoding, intProps);
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
		PduSubmitSm* pdu = new PduSubmitSm();
		__cfg_addr__(profilerAlias);
		transmitter->setupRandomCorrectSubmitSmPdu(pdu, profilerAlias);
		PduData::IntProps intProps;
		intProps["incorrectCmdText"] = 1;
		//������ ���������, ��������� � ���������������� pdu
		sendUpdateProfilePdu(pdu, "Cmd Text", sync, dataCoding, intProps);
		__tc_ok__;

	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmppProfilerTestCases::processSmeAcknowledgement(PduData* pduData,
	PduDeliverySm &pdu)
{
	__require__(pduData);
	__decl_tc__;
	__tc__("processUpdateProfile");
	__cfg_addr__(profilerAlias);
	Address srcAlias;
	SmppUtil::convert(pdu.get_message().get_source(), &srcAlias);
	if (srcAlias != profilerAlias)
	{
		__tc_fail__(1);
	}
	if (pdu.get_message().get_dataCoding() != DATA_CODING_SMSC_DEFAULT)
	{
		__tc_fail__(2);
	}
	//��������� � �������� �������
	Address addr;
	SmppUtil::convert(pdu.get_message().get_dest(), &addr);
	string text;
	//�������� profiler reportOptions
	if (pduData->intProps.count("reportOptions"))
	{
		__tc__("processUpdateProfile.reportOptions");
		__cfg_str__(cmdRespReportNone);
		__cfg_str__(cmdRespReportFull);
		switch (pduData->intProps.find("reportOptions")->second)
		{
			case ProfileReportOptions::ReportNone:
				text = cmdRespReportNone;
				break;
			case ProfileReportOptions::ReportFull:
				text = cmdRespReportFull;
				break;
			default:
				__unreachable__("Invalid reportoptions");
		}
	}
	//�������� profiler codePage
	else if (pduData->intProps.count("codePage"))
	{
		__tc__("processUpdateProfile.codePage");
		__cfg_str__(cmdRespDataCodingDefault);
		__cfg_str__(cmdRespDataCodingUcs2);
		switch (pduData->intProps.find("codePage")->second)
		{
			case ProfileCharsetOptions::Default:
				text = cmdRespDataCodingDefault;
				break;
			case ProfileCharsetOptions::Ucs2:
				text = cmdRespDataCodingUcs2;
				break;
			default:
				__unreachable__("Invalid codepage");
		}
	}
	//������������ ����� �������
	else if (pduData->intProps.count("incorrectCmdText"))
	{
		__tc__("processUpdateProfile.incorrectCmdText");
		__cfg_str__(cmdRespInvalidCmdText);
		text = cmdRespInvalidCmdText;
	}
	else
	{
		__tc_fail__(2);
	}
	//�������� �������
	if (text.length())
	{
		string pduText = decode(pdu.get_message().get_shortMessage(),
			pdu.get_message().get_smLength(), pdu.get_message().get_dataCoding());
		if (text != pduText)
		{
			__tc_fail__(1);
		}
	}
	__tc_ok_cond__;
}

}
}
}

