#ifndef TEST_CORE_PROFILE_UTIL
#define TEST_CORE_PROFILE_UTIL

#include "profiler/profiler.hpp"
#include <ostream>
#include <string>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using std::ostream;
using std::string;
using std::vector;
using smsc::profiler::Profile;

class ProfileUtil
{
public:
	static void setupRandomCorrectProfile(Profile& profile);
	static vector<int> compareProfiles(const Profile& p1, const Profile& p2);
};

ostream& operator<< (ostream& os, const Profile& p);
const string str(const Profile& p);

}
}
}

#endif /* TEST_CORE_PROFILE_UTIL */

