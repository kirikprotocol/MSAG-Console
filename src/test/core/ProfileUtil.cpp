#include "ProfileUtil.hpp"
#include "test/util/Util.hpp"
#include <sstream>

namespace smsc {
namespace test {
namespace core {

using namespace std;
using namespace smsc::test::util;

void ProfileUtil::setupRandomCorrectProfile(Profile& profile)
{
	profile.codepage = rand0(1); //Default=0, Ucs2=1
	profile.reportoptions = rand0(1); //ReportNone=0, ReportFull=1
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
	os << "codepage = " << p.codepage << ", reportoptions = " << p.reportoptions;
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

