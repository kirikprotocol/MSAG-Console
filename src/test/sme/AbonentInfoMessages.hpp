#ifndef TEST_SME_ABONENT_INFO_MESSAGES
#define TEST_SME_ABONENT_INFO_MESSAGES

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
struct AbonentInfoSmeMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const string& abonent, int status, const string& msc)
	{
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
			ostringstream s;
			s << "Abonent " << abonent;
			s << ": status " << status;
			s << ", encoding " << profile.codepage;
			s << ", msc " << msc;
			return convert(s.str(), profile.codepage);
		}
		if (profile.locale == "ru_ru")
		{
			return convert("", profile.codepage);
		}
		__unreachable__("Invalid locale options");
	}
};

//��������� ������ ��������� �� 1 sms
//������ ���������
struct AbonentInfoMobileMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const string& abonent, int status, const string& msc)
	{
		ostringstream s;
		s << "������� " << abonent;
		s << ": ������ " << (status ? "Online" : "Offline");
		s << ", msc " << (msc.length() ? msc : "unknown");
		return convert(s.str(), profile.codepage);
	}
};

}
}
}

#endif /* TEST_SME_ABONENT_INFO_MESSAGES */
