#ifndef TEST_SME_PROFILER_MESSAGES
#define TEST_SME_PROFILER_MESSAGES

#include "util/debug.h"
#include "profiler/profiler.hpp"
#include "test/util/TextUtil.hpp"
#include <string>

namespace smsc {
namespace test {
namespace sme {

using std::string;
using smsc::test::util::convert;
using namespace smsc::profiler;

//��������� ������ ��������� �� 1 sms
//������������ �� ������� ������
struct ProfilerReportMessage
{
	static const pair<string, uint8_t> format(const Profile& profile)
	{
		switch (profile.reportoptions)
		{
			case ProfileReportOptions::ReportNone:
				return convert("Delivery reports are disabled", profile.codepage);
			case ProfileReportOptions::ReportFull:
				return convert("������ �������� ���������", profile.codepage);
		}
		__unreachable__("Invalid locale or report options");
	}
};

//��������� �������� �� ��������� �� 1 sms
//�������� ��� en, �������� ��� ru
struct ProfilerCodepageMessage
{
	static const pair<string, uint8_t> format(const Profile& profile)
	{
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			switch (profile.codepage)
			{
				case ProfileCharsetOptions::Default:
					return convert("Message character encoding is set to default encoding (now you are not able to receive messages with russian characters in the text even if your cell phone supports ucs2 messages)", profile.codepage);
				case ProfileCharsetOptions::Ucs2:
					return convert("Message character encoding is set to ucs2 encoding (now you will be able to receive messages with russian characters in the case your cell phone supports ucs2 messages)", profile.codepage);
			}
		}
		if (profile.locale == "ru_ru")
		{
			switch (profile.codepage)
			{
				case ProfileCharsetOptions::Default:
					return convert("��������� ������ ��������� ����������� � default (������ �� �� ������ �������� ��������� � �������� ��������� � ������, ���� ���� ��� ������� ������������ ucs2 ���������)", profile.codepage);
				case ProfileCharsetOptions::Ucs2:
					return convert("��������� ������ ��������� ����������� � ucs2 (������ �� ������� �������� ��������� ���������� ������� ������� � ������, ������ ��� ������� ������ ������������ ucs2 ���������)", profile.codepage);
			}
		}
		__unreachable__("Invalid locale or data coding options");
	}
};

//��������� ������ ��������� �� 1 sms
//�������� ��� en, �������� ��� ru
struct ProfilerLocaleMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const string& newLocale)
	{
		if (newLocale == "")
		{
			if (profile.locale == "en_us" || profile.locale == "en_gb")
			{
				return convert("Unknown locale", profile.codepage);
			}
			if (profile.locale == "ru_ru")
			{
				return convert("����������� ����� locale", profile.codepage);
			}
		}
		else
		{
			if (newLocale == "en_us" || newLocale == "en_gb")
			{
				return convert("Locale is successfully changed", profile.codepage);
			}
			if (newLocale == "ru_ru")
			{
				return convert("��������� locale ������� ��������", profile.codepage);
			}
		}
		__unreachable__("Invalid locale options");
	}
};

//��������� ������ ��������� �� 1 sms
//�������� ��� en, �������� ��� ru
struct ProfilerHideMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		int hide)
	{
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			if (hide)
			{
				return convert("Hide option is turned on", profile.codepage);
			}
			return convert("Unhide option is turned on", profile.codepage);
		}
		if (profile.locale == "ru_ru")
		{
			if (hide)
			{
				return convert("�������� hide �����", profile.codepage);
			}
			return convert("�������� unhide �����", profile.codepage);
		}
		__unreachable__("Invalid locale options");
	}
};

//������ ���������
struct ProfilerErrorMessage
{
	static const pair<string, uint8_t> format(const Profile& profile)
	{
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			return convert("������������ ������� (!@#$%^&*( )_+-=|\\:;'<,>.?/)", profile.codepage);
		}
		if (profile.locale == "ru_ru")
		{
			return convert("", profile.codepage);
		}
		__unreachable__("Invalid locale options");
	}
};

}
}
}

#endif /* TEST_SME_PROFILER_MESSAGES */
