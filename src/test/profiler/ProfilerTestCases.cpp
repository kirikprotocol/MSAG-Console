#include "ProfilerTestCases.hpp"
#include "test/core/ProfileUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include "util/debug.h"
#include <algorithm>

namespace smsc {
namespace test {
namespace profiler {

using smsc::util::Logger;
using namespace std;
using namespace smsc::sms; //constants
using namespace smsc::profiler; //constants
using namespace smsc::test::sms;
using namespace smsc::test::core; //ProfileUtil, str()
using namespace smsc::test::util;

ProfilerTestCases::ProfilerTestCases(Profiler* _profiler,
	ProfileRegistry* _profileReg, CheckList* _chkList)
	: profiler(_profiler), profileReg(_profileReg), chkList(_chkList)
{
	__require__(profiler);
	//__require__(profileReg);
	//__require__(chkList);
}

Category& ProfilerTestCases::getLog()
{
	static Category& log = Logger::getCategory("ProfilerTestCases");
	return log;
}

void ProfilerTestCases::fillAddressWithQuestionMarks(Address& addr, int len)
{
	__require__(len > 0 && len <= addr.getLength());
	AddressValue addrVal;
	uint8_t addrLen = addr.getValue(addrVal);
	memset(addrVal + addrLen - len, '?', len);
	addr.setValue(addrLen, addrVal);
}

void ProfilerTestCases::createProfileMatch(Address& addr, int num)
{
	TCSelector s(num, 3);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			Profile profile;
			ProfileUtil::setupRandomCorrectProfile(profile);
			switch (s.value())
			{
				case 1: //адрес без знаков подстановки
					__tc__("createProfileMatch.noSubstSymbols");
					break;
				case 2: //адрес с одним или несколькими '?'
					__tc__("createProfileMatch.withQuestionMarks");
					fillAddressWithQuestionMarks(addr, rand1(addr.getLength()));
					break;
				case 3: //адрес целиком из '?'
					__tc__("createProfileMatch.entirelyQuestionMarks");
					fillAddressWithQuestionMarks(addr, addr.getLength());
					break;
				default:
					__unreachable__("Invalid num");
			}
			if (profileReg)
			{
				if (!profileReg->checkExists(addr))
				{
					profiler->update(addr, profile);
					profileReg->putProfile(addr, profile);
					__tc_ok__;
				}
			}
			else
			{
				profiler->update(addr, profile);
				__tc_ok__;
			}
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void ProfilerTestCases::createProfileNotMatch(Address& addr, int num)
{
	TCSelector s(num, 5);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			__tc__("createProfileMatch.noSubstSymbols");
			Profile profile;
			ProfileUtil::setupRandomCorrectProfile(profile);
			AddressValue addrVal;
			uint8_t addrLen = addr.getValue(addrVal);
			switch (s.value())
			{
				case 1: //отличается typeOfNumber
					__tc__("createProfileNotMatch.diffType");
					addr.setTypeOfNumber(addr.getTypeOfNumber() + 1);
					break;
				case 2: //отличается numberingPlan
					__tc__("createProfileNotMatch.diffPlan");
					addr.setNumberingPlan(addr.getNumberingPlan() + 1);
					break;
				case 3: //отличается addressValue
					if (addrLen > 1 && next_permutation(addrVal, addrVal + addrLen))
					{
						__tc__("createProfileNotMatch.diffValue");
						addr.setValue(addrLen, addrVal);
					}
					break;
				case 4: //в адресе '?' меньше, чем нужно
					if (addrLen > 1)
					{
						__tc__("createProfileNotMatch.diffValueLength");
						int len1 = rand1(addrLen - 1);
						int len2 = rand1(addrLen - len1);
						__require__(len1 + len2 <= addrLen);
						memset(addrVal + addrLen - len1 - len2, '?', len1);
						addr.setValue(addrLen - len2, addrVal);
					}
					break;
				case 5: //в адресе '?' больше, чем нужно
					if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
					{
						__tc__("createProfileNotMatch.diffValueLength");
						int len1 = rand0(addrLen);
						int len2 = rand1(MAX_ADDRESS_VALUE_LENGTH - addrLen);
						__require__(addrLen + len2 <= MAX_ADDRESS_VALUE_LENGTH);
						memset(addrVal + addrLen - len1, '?', len1 + len2);
						addr.setValue(addrLen + len2, addrVal);
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			if (profileReg)
			{
				if (!profileReg->checkExists(addr))
				{
					profiler->update(addr, profile);
					profileReg->putProfile(addr, profile);
					__tc_ok__;
				}
			}
			else
			{
				profiler->update(addr, profile);
				__tc_ok__;
			}
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void ProfilerTestCases::updateProfile(const Address& addr)
{
	__decl_tc__;
	__tc__("updateProfile");
	try
	{
		Profile profile;
		ProfileUtil::setupRandomCorrectProfile(profile);
		if (profileReg)
		{
			if (profileReg->checkExists(addr))
			{
				profiler->update(addr, profile);
				profileReg->putProfile(addr, profile);
				__tc_ok__;
			}
		}
		else
		{
			profiler->update(addr, profile);
			__tc_ok__;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void ProfilerTestCases::lookup(const Address& addr)
{
	__decl_tc__;
	__tc__("lookup");
	try
	{
		const Profile& p1 = profiler->lookup(addr);
		const Profile& p2 = profileReg->getProfile(addr);
		__trace2__("ProfilerTestCases::lookup(): profiler.lookup() = {%s}, profileReg.getProfile() = {%s}",
			str(p1).c_str(), str(p2).c_str());
		__tc_fail2__(ProfileUtil::compareProfiles(p1, p2), 0);
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
	}
}

void ProfilerTestCases::putCommand(const Address& addr, int num)
{
	TCSelector s(num, 9);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			SMS sms;
			SmsUtil::setupRandomCorrectSms(&sms);
			sms.setOriginatingAddress(addr);
			string text;
			int codepage = -1;
			int reportoptions = -1;
			switch (s.value())
			{
				case 1: //report none
					__tc__("putCommand.reportNoneMixedCase");
					text = "RePoRT NoNe";
					reportoptions = ProfileReportOptions::ReportNone;
					break;
				case 2: //report none
					__tc__("putCommand.reportNoneSpaces");
					text = "  rEpOrt  nOnE  ";
					reportoptions = ProfileReportOptions::ReportNone;
					break;
				case 3: //report full
					__tc__("putCommand.reportFullMixedCase");
					text = "RePoRT FuLL";
					reportoptions = ProfileReportOptions::ReportFull;
					break;
				case 4: //report full
					__tc__("putCommand.reportFullSpaces");
					text = "  rEpOrt  fUll  ";
					reportoptions = ProfileReportOptions::ReportFull;
					break;
				case 5: //ucs2 codepage
					__tc__("putCommand.ucs2CodepageMixedCase");
					text = "uCS2";
					codepage = ProfileCharsetOptions::Ucs2;
					break;
				case 6: //usc2 codepage
					__tc__("putCommand.ucs2CodepageSpaces");
					text = "  Ucs2  ";
					codepage = ProfileCharsetOptions::Ucs2;
					break;
				case 7: //default codepage
					__tc__("putCommand.defaultCodepageMixedCase");
					text = "DeFauLT";
					codepage = ProfileCharsetOptions::Default;
					break;
				case 8: //default codepage
					__tc__("putCommand.defaultCodepageSpaces");
					text = "  dEfAUlt  ";
					codepage = ProfileCharsetOptions::Default;
					break;
				case 9: //неправильный текст
					__tc__("putCommand.incorrectText");
					break;
				default:
					__unreachable__("Invalid num");
			}
			sms.getMessageBody().setIntProperty(Tag::SMPP_SM_LENGTH, text.length());
			sms.getMessageBody().setStrProperty(Tag::SMPP_SHORT_MESSAGE, text);
			SmscCommand cmd = SmscCommand::makeSumbmitSm(sms, rand0(INT_MAX));
			__trace2__("ProfilerTestCases::putCommand(): sms = %s", str(sms).c_str());
			if (profileReg)
			{
				Profile profile = profileReg->getProfile(addr);
				if (reportoptions >= 0)
				{
					profile.reportoptions = reportoptions;
				}
				if (codepage >= 0)
				{
					profile.codepage = codepage;
				}
				profileReg->putProfile(addr, profile);
			}
			profiler->putCommand(cmd);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

}
}
}

