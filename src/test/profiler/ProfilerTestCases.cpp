#include "ProfilerTestCases.hpp"
#include "test/core/ProfileUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include "util/debug.h"
#include "smeman/smsccmd.h"
#include <algorithm>

namespace smsc {
namespace test {
namespace profiler {

using smsc::util::Logger;
using smsc::smeman::SmsResp;
using smsc::test::conf::TestConfig;
using namespace std;
using namespace smsc::sms; //constants
using namespace smsc::profiler; //constants
using namespace smsc::test::sms;
using namespace smsc::test::smpp;
using namespace smsc::test::smpp::DataCoding;
using namespace smsc::test::core; //ProfileUtil, str()
using namespace smsc::test::util;

ProfilerTestCases::ProfilerTestCases(Profiler* _profiler,
  ProfileRegistry* _profileReg, CheckList* _chkList)
  : profiler(_profiler), profileReg(_profileReg), chkList(_chkList)
{
	//__require__(profiler);
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

bool ProfilerTestCases::updateProfile(const char* tc, int num,
  const Address& addr, const Profile& profile, bool create)
{
  if (profileReg)
  {
    if ((create && !profileReg->checkExists(addr)) ||
      !create && profileReg->checkExists(addr))
    {
      if (profiler)
      {
        profiler->update(addr, profile);
      }
      profileReg->putProfile(addr, profile);
      __trace2__("%s(): addr = %s, profile = %s",
        tc, str(addr).c_str(), str(profile).c_str());
      return true;
    }
  }
  else if (profiler)
  {
    profiler->update(addr, profile);
    __trace2__("%s(): addr = %s, profile = %s",
      tc, str(addr).c_str(), str(profile).c_str());
    return true;
  }
  return false;
}

void ProfilerTestCases::createProfileMatch(Address& addr, int num)
{
  TCSelector s(num, 1);
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
        /*
        case 2: //адрес с одним или несколькими '?'
          __tc__("createProfileMatch.withQuestionMarks");
          fillAddressWithQuestionMarks(addr, rand1(addr.getLength()));
          break;
        case 3: //адрес целиком из '?'
          __tc__("createProfileMatch.entirelyQuestionMarks");
          fillAddressWithQuestionMarks(addr, addr.getLength());
          break;
        */
        default:
          __unreachable__("Invalid num");
      }
      if (updateProfile("createProfileMatch", s.value(), addr, profile, true))
      {
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
  TCSelector s(num, 3);
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
		  SmsUtil::setupRandomCorrectAddressTon(&addr);
          break;
        case 2: //отличается numberingPlan
          __tc__("createProfileNotMatch.diffPlan");
		  SmsUtil::setupRandomCorrectAddressNpi(&addr);
          break;
        case 3: //отличается addressValue
          if (addrLen > 1 && next_permutation(addrVal, addrVal + addrLen))
          {
            __tc__("createProfileNotMatch.diffValue");
            addr.setValue(addrLen, addrVal);
          }
          break;
        /*
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
        */
        default:
          __unreachable__("Invalid num");
      }
      if (updateProfile("createProfileNotMatch", s.value(), addr, profile, true))
      {
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
    if (updateProfile("updateProfile", 1, addr, profile, false))
    {
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
  __require__(profiler && profileReg);
  __decl_tc__;
  __tc__("lookup");
  try
  {
    const Profile& p1 = profiler->lookup(addr);
    time_t t;
    const Profile& p2 = profileReg->getProfile(addr, t);
    __trace2__("lookupProfile(): addr = %s, profiler.lookup() = {%s}, profileReg.getProfile() = {%s}",
      str(addr).c_str(), str(p1).c_str(), str(p2).c_str());
    __tc_fail2__(ProfileUtil::compareProfiles(p1, p2), 0);
    __tc_ok_cond__;
  }
  catch(...)
  {
    __tc_fail__(100);
  }
}

void ProfilerTestCases::putCommand(const Address& addr, uint8_t dataCoding, int num)
{
	__require__(profiler);
	TCSelector s(num, 9);
	__decl_tc12__;
	for (; s.check(); s++)
	{
		try
		{
		  SMS sms;
		  SmsUtil::setupRandomCorrectSms(&sms);
		  sms.setOriginatingAddress(addr);
		  __cfg_addr__(profilerAddr);
		  sms.setDestinationAddress(profilerAddr);
		  sms.setIntProperty(Tag::SMPP_ESM_CLASS, 0x0);
		  string text;
		  int cmdType, codepage, reportoptions;
		  switch (s.value())
		  {
			  case 1: //report none
				__tc1__("putCommand.reportOptions.reportNoneMixedCase");
				text = "RePoRT NoNe";
				reportoptions = ProfileReportOptions::ReportNone;
				cmdType = UPDATE_REPORT_OPTIONS;
				break;
			  case 2: //report none
				__tc1__("putCommand.reportOptions.reportNoneSpaces");
				text = "  rEpOrt  nOnE  ";
				reportoptions = ProfileReportOptions::ReportNone;
				cmdType = UPDATE_REPORT_OPTIONS;
				break;
			  case 3: //report full
				__tc1__("putCommand.reportOptions.reportFullMixedCase");
				text = "RePoRT FuLL";
				reportoptions = ProfileReportOptions::ReportFull;
				cmdType = UPDATE_REPORT_OPTIONS;
				break;
			  case 4: //report full
				__tc1__("putCommand.reportOptions.reportFullSpaces");
				text = "  rEpOrt  fUll  ";
				reportoptions = ProfileReportOptions::ReportFull;
				cmdType = UPDATE_REPORT_OPTIONS;
				break;
			  case 5: //ucs2 codepage
				__tc1__("putCommand.dataCoding.ucs2CodepageMixedCase");
				text = "uCS2";
				codepage = ProfileCharsetOptions::Ucs2;
				cmdType = UPDATE_CODE_PAGE;
				break;
			  case 6: //usc2 codepage
				__tc1__("putCommand.dataCoding.ucs2CodepageSpaces");
				text = "  Ucs2  ";
				codepage = ProfileCharsetOptions::Ucs2;
				cmdType = UPDATE_CODE_PAGE;
				break;
			  case 7: //default codepage
				__tc1__("putCommand.dataCoding.defaultCodepageMixedCase");
				text = "DeFauLT";
				codepage = ProfileCharsetOptions::Default;
				cmdType = UPDATE_CODE_PAGE;
				break;
			  case 8: //default codepage
				__tc1__("putCommand.dataCoding.defaultCodepageSpaces");
				text = "  dEfAUlt  ";
				codepage = ProfileCharsetOptions::Default;
				cmdType = UPDATE_CODE_PAGE;
				break;
			  case 9: //неправильный текст
				__tc1__("putCommand.incorrectCmdText");
				cmdType = INCORRECT_COMMAND_TEXT;
				break;
			  default:
				__unreachable__("Invalid num");
		  }
		  //текст сообщения
		  switch (dataCoding)
		  {
			case DEFAULT:
			  __tc2__("putCommand.cmdTextDefault");
			  break;
			case UCS2:
			  __tc2__("putCommand.cmdTextUcs2");
			  break;
			default:
			  __unreachable__("Invalid dataCoding");
		  }
		  int msgLen;
		  auto_ptr<char> msg = encode(text, dataCoding, msgLen, true);
		  sms.setIntProperty(Tag::SMPP_SM_LENGTH, msgLen);
		  sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, msg.get(), msgLen);
		  sms.setIntProperty(Tag::SMPP_DATA_CODING, dataCoding);
		  SmscCommand cmd = SmscCommand::makeDeliverySm(sms, rand0(INT_MAX));
		  __trace2__("putProfilerCommand(): sms = %s", str(sms).c_str());
		  if (profileReg)
		  {
			time_t t;
			Profile profile = profileReg->getProfile(addr, t);
			if (cmdType == UPDATE_REPORT_OPTIONS)
			{
			  profile.reportoptions = reportoptions;
			}
			else if (cmdType == UPDATE_CODE_PAGE)
			{
			  profile.codepage = codepage;
			}
			profileReg->putProfile(addr, profile);
			profileReg->registerDialogId(cmd->get_dialogId(), cmdType);
		  }
		  profiler->putCommand(cmd);
		  __tc12_ok__;
		}
		catch(...)
		{
		  __tc12_fail__(100);
		  error();
		}
	}
}

#define __ignore__(field) \
  sms.set##field(ackSms->get##field());

#define __ignore_bool__(field) \
  sms.set##field(ackSms->is##field());

void ProfilerTestCases::onSubmit(SmscCommand& cmd)
{
  __decl_tc__;
  SMS* ackSms = cmd->get_sms();
  __require__(ackSms);
  __trace2__("onSubmit(): sms = %s", str(*ackSms).c_str());
  int cmdType;
  __tc__("getCommand.submit.checkDialogId");
  if (profileReg->unregisterDialogId(cmd->get_dialogId(), cmdType))
  {
    __tc_fail__(1);
  }
  __tc_ok_cond__;
  //поиск по алиасенному srcAddr
  time_t t;
  Profile profile = profileReg->getProfile(ackSms->getOriginatingAddress(), t);
  //проверки
  __tc__("getCommand.submit.checkFields");
  __cfg_int__(maxValidPeriod);
  __cfg_addr__(profilerAddr);
  __cfg_str__(profilerServiceType);
  __cfg_int__(profilerProtocolId);
  SMS sms;
  sms.setOriginatingAddress(profilerAddr);
  __ignore__(DestinationAddress);
  __ignore__(DealiasedDestinationAddress);
  __ignore__(OriginatingDescriptor);
  sms.setSubmitTime(time(NULL));
  sms.setValidTime(sms.getSubmitTime() + maxValidPeriod);
  sms.setNextTime(sms.getSubmitTime());
  __ignore__(MessageReference);
  __ignore__(DeliveryReport);
  __ignore__(BillingRecord);
  __ignore_bool__(ArchivationRequested);
  sms.setEServiceType(profilerServiceType.c_str());
  //sms.setIntProperty(Tag::SMPP_SCHEDULE_DELIVERY_TIME, sms.getSubmitTime());
  sms.setIntProperty(Tag::SMPP_ESM_CLASS, ESM_CLASS_NORMAL_MESSAGE);
  string text;
  if (cmdType == UPDATE_REPORT_OPTIONS)
  {
    __cfg_str__(profilerRespReportNone);
    __cfg_str__(profilerRespReportFull);
    switch (profile.reportoptions)
    {
      case ProfileReportOptions::ReportNone:
        text = profilerRespReportNone;
        break;
      case ProfileReportOptions::ReportFull:
        text = profilerRespReportFull;
        break;
      default:
        __unreachable__("Invalid reportoptions");
    }
  }
  else if (cmdType == UPDATE_CODE_PAGE)
  {
    __cfg_str__(profilerRespDataCodingDefault);
    __cfg_str__(profilerRespDataCodingUcs2);
    switch (profile.codepage)
    {
      case ProfileCharsetOptions::Default:
        text = profilerRespDataCodingDefault;
        break;
      case ProfileCharsetOptions::Ucs2:
        text = profilerRespDataCodingUcs2;
        break;
      default:
        __unreachable__("Invalid codepage");
    }
  }
  else if (cmdType == INCORRECT_COMMAND_TEXT)
  {
    __cfg_str__(profilerRespInvalidCmdText);
    text = profilerRespInvalidCmdText;
  }
  uint8_t dataCoding = getDataCoding(profile.codepage);
  int msgLen;
  auto_ptr<char> msg = encode(text, dataCoding, msgLen, true);
  sms.setIntProperty(Tag::SMPP_DATA_CODING, dataCoding);
  sms.setIntProperty(Tag::SMPP_SM_LENGTH, msgLen);
  sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, msg.get(), msgLen);
  __tc_fail2__(SmsUtil::compareMessages(*ackSms, sms), 0);
  __tc_ok_cond__;
}

void ProfilerTestCases::onDeliveryResp(SmscCommand& cmd)
{
  __decl_tc__;
  SmsResp* respSms = cmd->get_resp();
  __require__(respSms);
  //const char* respSms->get_messageId();
  __tc__("getCommand.deliverResp");
  if (respSms->get_status())
  {
	  __trace2__("deliver_sm_resp status = %d", respSms->get_status());
    __tc_fail__(1);
  }
  __tc_ok_cond__;
}

void ProfilerTestCases::onCommand()
{
  __require__(profiler);
  __decl_tc__;
  while (profiler->hasInput())
  {
    SmscCommand cmd = profiler->getCommand();
    switch (cmd->get_commandId())
    {
      case SUBMIT:
        onSubmit(cmd);
        break;
      case DELIVERY_RESP:
        onDeliveryResp(cmd);
        break;
      default:
        __unreachable__("Invalid commandId");
    }
  }
}

}
}
}
