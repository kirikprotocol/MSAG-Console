#ifndef TEST_CORE_PROFILE_UTIL
#define TEST_CORE_PROFILE_UTIL

#include "profiler/profiler.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace core {

using std::vector;
using smsc::profiler::Profile;

class ProfileUtil
{
public:
	static void setupRandomCorrectProfile(Profile& profile);
	static vector<int> compareProfiles(const Profile& p1, const Profile& p2);
};

}
}
}

#endif /* TEST_CORE_PROFILE_UTIL */

