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

const int UPDATE_REPORT_OPTIONS = 1;
const int UPDATE_CODE_PAGE = 2;
const int INCORRECT_COMMAND_TEXT = 3;
const int INCORRECT_ESM_CLASS = 4;

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
	 * ����������� ��� ���������� ������������� �������.
	 * ���������� ������� ����� ���������� �������.
	 */
	void putProfile(const Address& addr, const Profile& profile);

	/**
	 * ���������� ������� � ����� ���������� ��������� �������.
	 * ���� ������� �� ���������������, ������������ ��������� � valid = true.
	 */
	const Profile& getProfile(const Address& addr, time_t& t) const;

	/**
	 * �������� ������� ������������������� �������.
	 * ������ ���������� ������������.
	 */
	bool checkExists(const Address& addr) const;

	/**
	 * ������������� �������� ����� ��������� �������.
	 */
	void setProfileUpdateTime(const Address& addr, time_t t);

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

