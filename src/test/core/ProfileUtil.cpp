#include "ProfileUtil.hpp"
#include "test/util/Util.hpp"
#include <sstream>

namespace smsc {
namespace test {
namespace core {

using namespace std;
using namespace smsc::test::util;
using namespace smsc::profiler;

template<typename T, int size>
inline int sz(T (&)[size]) { return size; }

void ProfileUtil::setupRandomCorrectProfile(Profile& profile)
{
	static const int charsetOptions[] =
	{
		ProfileCharsetOptions::Default,
		ProfileCharsetOptions::Ucs2
	};
	static const int reportOptions[] =
	{
		ProfileReportOptions::ReportNone,
		ProfileReportOptions::ReportFull,
		ProfileReportOptions::ReportFinal
	};
	static const string locales[] =
	{
		"en_us", "en_gb", "ru_ru"
	};
	profile.codepage = charsetOptions[rand0(sz(charsetOptions) - 1)];
	profile.reportoptions = reportOptions[rand0(sz(reportOptions) - 1)];
	profile.locale = locales[rand0(sz(locales) - 1)];
	profile.hide = rand0(1);
	profile.hideModifiable = rand0(1);
}

#define __compare__(errCode, field) \
	if (p1.field != p2.field) { res.push_back(errCode); }
	
vector<int> ProfileUtil::compareProfiles(const Profile& p1, const Profile& p2)
{
	vector<int> res;
	__compare__(1, codepage);
	__compare__(2, reportoptions);
	return res;
}

bool operator==(const Profile& p1, const Profile& p2)
{
	return (p1.codepage == p2.codepage && p1.reportoptions == p2.reportoptions &&
		p1.locale == p2.locale);
}

bool operator!=(const Profile& p1, const Profile& p2)
{
	return !operator==(p1, p2);
}

ostream& operator<< (ostream& os, const Profile& p)
{
	os << "codepage = " << p.codepage <<
		", reportoptions = " << p.reportoptions <<
		", locale = " << p.locale <<
		", hide = " << p.hide <<
		", hideModifiable = " << (p.hideModifiable ? "true" : "false");
}

const string str(const Profile& p)
{
	ostringstream os;
	os << p;
	return os.str();
}

}
}
}

