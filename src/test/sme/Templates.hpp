#ifndef TEST_SME_TEMPLATES
#define TEST_SME_TEMPLATES

#include "test/util/TextUtil.hpp"
#include "util/debug.h"
#include <string>
#include <sstream>

namespace smsc {
namespace test {
namespace sme {

using std::string;
using std::ostringstream;
using smsc::test::util::convert;

//��������� ������ ��������� �� 1 sms
//������������ �� ������� ������
struct T0
{
	static const pair<string, uint8_t> format(const Profile& profile)
	{
		return convert("Hello ���� �������", profile.codepage);
	}
};

//��������� ������ ��������� �� 1 sms
struct T1
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const string& name)
	{
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			ostringstream s;
			s << "Hello " << (name.length() ? name : "world");
			return convert(s.str(), profile.codepage);
		}
		else if (profile.locale == "ru_ru")
		{
			ostringstream s;
			s << "������ " << (name.length() ? name : "���");
			return convert(s.str(), profile.codepage);
		}
		__unreachable__("Invalid locale options");
	}
};

//��������� �� ��������� �� 1 sms
struct T2
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const string& name1, const string& name2)
	{
		if (!name2.length())
		{
			return convert("", profile.codepage);
		}
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			ostringstream s;
			s << "Hello " << (name1.length() ? name1 : "world");
			s << " and " << name2;
			s << " and all the other people in the world. If you have received this message please resend it to your ten friends and fill yourself happy. Sincerely, mega template.";
			return convert(s.str(), profile.codepage);
		}
		else if (profile.locale == "ru_ru")
		{
			ostringstream s;
			s << "������ " << (name1.length() ? name1 : "���");
			s << " � " << name2;
			s << " � ��� ���������. ��������� ��� ��������� 10 ����� �������. � ��������, ���� ��������.";
			return convert(s.str(), profile.codepage);
		}
		__unreachable__("Invalid locale options");
	}
};

}
}
}

#endif /* TEST_SME_TEMPLATES */
