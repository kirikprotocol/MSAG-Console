#include "ProfileRegistry.hpp"

namespace smsc {
namespace test {
namespace core {

using namespace std;
using namespace smsc::sms; //AddressValue, constants

const Profile* ProfileRegistry::ProfileIterator::next()
{
	return (it1 != it2 ? &(it1++)->second->profile : NULL);
}

ProfileRegistry::~ProfileRegistry()
{
	for (ProfileMap::iterator it = profileMap.begin(); it != profileMap.end(); it++)
	{
		delete it->second;
	}
}

void ProfileRegistry::putProfile(const Address& addr, const Profile& profile)
{
	ProfileMap::iterator it = profileMap.find(addr);
	if (it != profileMap.end())
	{
		it->second->profile = profile;
		it->second->putTime = time(NULL);
	}
	else
	{
		profileMap[addr] = new ProfileData(profile, time(NULL));
	}
}

ProfileRegistry::ProfileIterator* ProfileRegistry::iterator() const
{
	return new ProfileIterator(profileMap.begin(), profileMap.end());
}

const Profile& ProfileRegistry::getProfile(const Address& addr, time_t& t) const
{
	Address tmp(addr);
	AddressValue addrVal;
	int addrLen = tmp.getValue(addrVal);
	for (int len = 0 ; len <= addrLen; len++)
	{
		if (len)
		{
			addrVal[addrLen - len] = '?';
			tmp.setValue(addrLen, addrVal);
		}
		ProfileMap::const_iterator it = profileMap.find(tmp);
		if (it != profileMap.end())
		{
			t = it->second->putTime;
			return it->second->profile;
		}
		/*
		if (addrLen - len < MAX_ADDRESS_VALUE_LENGTH)
		{
			addrVal[addrLen - len] = '*';
			tmp.setValue(addrLen - len + 1, addrVal);
			ProfileMap::const_iterator it = profileMap.find(tmp);
			if (it != addrMap.end())
			{
				return it->second;
			}
		}
		*/
	}
	t = time(NULL);
	return defProfile;
}

bool ProfileRegistry::checkExists(const Address& addr) const
{
	ProfileMap::const_iterator it = profileMap.find(addr);
	return (it != profileMap.end());
}

bool ProfileRegistry::registerDialogId(uint32_t dialogId, int cmdType)
{
	DialogMap::const_iterator it = dialogMap.find(dialogId);
	if (it != dialogMap.end())
	{
		return false;
	}
	dialogMap[dialogId] = cmdType;
	return true;
}

bool ProfileRegistry::unregisterDialogId(uint32_t dialogId, int& cmdType)
{
	DialogMap::iterator it = dialogMap.find(dialogId);
	if (it == dialogMap.end())
	{
		return false;
	}
	cmdType = it->second;
	dialogMap.erase(it);
	return true;
}

}
}
}

