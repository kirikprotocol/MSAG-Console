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

const int INCORRECT_COMMAND_TEXT = 0;
const int UPDATE_REPORT_OPTIONS = 1;
const int UPDATE_CODE_PAGE = 2;

class ProfileRegistry
{
	struct ProfileData
	{
		Profile profile;
		time_t putTime;
		ProfileData(Profile p, time_t t) : profile(p), putTime(t) {}
	};
	typedef map<const Address, ProfileData*, ltAddress> ProfileMap;
	typedef map<const uint32_t, int> DialogMap;

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

	/**
	 * Регистрация профиля.
	 */
	void putProfile(const Address& addr, const Profile& profile);

	/**
	 * Возвращает профиль и время последнего изменения профиля.
	 * Если профиль не зарегистрирован, возвращается дефолтный и valid = true.
	 */
	const Profile& getProfile(const Address& addr, time_t& t) const;

	/**
	 * Проверка наличия зарегистрированного профиля.
	 * Призак валидности игнорируется.
	 */
	bool checkExists(const Address& addr) const;

	ProfileIterator* iterator() const;

	bool registerDialogId(uint32_t dialogId, int cmdType);
	bool unregisterDialogId(uint32_t dialogId, int& cmdType);

private:
	const Profile defProfile;
	ProfileMap profileMap;
	DialogMap dialogMap;
};

}
}
}

#endif /* TEST_CORE_PROFILE_REGISTRY */

