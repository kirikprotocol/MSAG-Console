#ifndef TEST_CORE_PROFILE_REGISTRY
#define TEST_CORE_PROFILE_REGISTRY

#include "profiler/profiler.hpp"
#include "test/sms/SmsUtil.hpp"
#include <map>

namespace smsc {
namespace test {
namespace core {

using std::map;
using smsc::sms::Address;
using smsc::profiler::Profile;
using smsc::test::sms::ltAddress;

class ProfileRegistry
{
	typedef map<const Address, Profile*, ltAddress> ProfileMap;

public:
	struct ProfileIterator
	{
		ProfileMap::const_iterator it1;
		ProfileMap::const_iterator it2;
		ProfileIterator(ProfileMap::const_iterator i1, ProfileMap::const_iterator i2)
			: it1(i1), it2(i2) {}
		const Profile* next();
	};
	
	ProfileRegistry(const Profile& _defProfile)
		: defProfile(_defProfile) {}

	virtual ~ProfileRegistry();

	void putProfile(const Address& addr, const Profile& profile);

	ProfileIterator* iterator() const;

	const Profile& getProfile(const Address& addr) const;

	bool checkExists(const Address& addr) const;

private:
	ProfileMap profileMap;
	const Profile defProfile;
};

}
}
}

#endif /* TEST_CORE_PROFILE_REGISTRY */

