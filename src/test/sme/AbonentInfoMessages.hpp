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

//—ообщени€ всегда умещаютс€ на 1 sms
//Ќаследование из базовой локали
struct AbonentInfoSmeMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const string& abonent, const Profile& abonentProfile, SmeType abonentStatus,
		const string& msc)
	{
		//если задана пуста€ строка, то беретс€ из базовой локали
		/*
		if (profile.locale == "en_us" || profile.locale == "en_gb")
		{
		}
		if (profile.locale == "ru_ru")
		{
			return convert("", profile.codepage);
		}
		__unreachable__("Invalid locale options");
		*/
		ostringstream s;
		s << "Abonent " << abonent;
		switch (abonentStatus)
		{
			case SME_RECEIVER:
			case SME_TRANSMITTER:
			case SME_TRANSCEIVER:
				s << ": status 1";
				break;
			default:
				s << ": status 0";
		}
		s << ", encoding " << abonentProfile.codepage;
		s << ", msc " << msc;
		return convert(s.str(), profile.codepage);
	}
};

//—ообщени€ всегда умещаютс€ на 1 sms
//ѕустое сообщение
struct AbonentInfoMobileMessage
{
	static const pair<string, uint8_t> format(const Profile& profile,
		const string& abonent, const Profile& abonentProfile, SmeType abonentStatus,
		const string& msc)
	{
		ostringstream s;
		s << "јбонент " << abonent;
		switch (abonentStatus)
		{
			case SME_RECEIVER:
			case SME_TRANSMITTER:
			case SME_TRANSCEIVER:
				s << ": статус Online";
				break;
			default:
				s << ": статус Offline";
		}
		s << ", msc " << (msc.length() ? msc : "unknown");
		return convert(s.str(), profile.codepage);
	}
};

}
}
}

#endif /* TEST_SME_ABONENT_INFO_MESSAGES */
