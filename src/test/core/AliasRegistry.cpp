#include "AliasRegistry.hpp"
#include "test/sms/SmsUtil.hpp"
#include <fstream>

namespace smsc {
namespace test {
namespace core {

using namespace std;
using namespace smsc::sms; //AddressValue, constants
using smsc::test::sms::SmsUtil;

const AliasHolder* AliasRegistry::AliasIterator::next()
{
	return (it1 != it2 ? (it1++)->second : NULL);
}

AliasRegistry::~AliasRegistry()
{
	clear();
}

bool AliasRegistry::putAlias(const AliasInfo& alias)
{
	//дублированные алиасы не сохран€ютс€
	if (addrMap.find(alias.addr) != addrMap.end() ||
		aliasMap.find(alias.alias) != aliasMap.end())
	{
		return false;
	}
	AliasHolder* holder = new AliasHolder(alias);
	addrMap[alias.addr] = holder;
	aliasMap[alias.alias] = holder;
	return true;
}

void AliasRegistry::clear()
{
	for (AddressMap::iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		delete it->second;
	}
	//дл€ aliasMap ничего удал€ть не нужно, т.к. объекты в aliasMap и addrMap общие
	addrMap.clear();
	aliasMap.clear();
}

AliasRegistry::AliasIterator* AliasRegistry::iterator() const
{
	__require__(addrMap.size() == aliasMap.size());
	return new AliasIterator(addrMap.begin(), addrMap.end());
}

const AliasHolder* AliasRegistry::findAliasByAddress(const Address& addr) const
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
		AddressMap::const_iterator it = addrMap.find(tmp);
		if (it != addrMap.end())
		{
			return it->second;
		}
		/*
		if (addrLen - len < MAX_ADDRESS_VALUE_LENGTH)
		{
			addrVal[addrLen - len] = '*';
			tmp.setValue(addrLen - len + 1, addrVal);
			AddressMap::const_iterator it = addrMap.find(tmp);
			if (it != addrMap.end())
			{
				return it->second;
			}
		}
		*/
	}
	return NULL;
}

const AliasHolder* AliasRegistry::findAddressByAlias(const Address& alias) const
{
	Address tmp(alias);
	AddressValue aliasVal;
	int aliasLen = tmp.getValue(aliasVal);
	for (int len = 0 ; len <= aliasLen; len++)
	{
		if (len)
		{
			aliasVal[aliasLen - len] = '?';
			tmp.setValue(aliasLen, aliasVal);
		}
		AddressMap::const_iterator it = aliasMap.find(tmp);
		if (it != aliasMap.end())
		{
			return it->second;
		}
		/*
		if (aliasLen - len < MAX_ADDRESS_VALUE_LENGTH)
		{
			aliasVal[aliasLen - len] = '*';
			tmp.setValue(aliasLen - len + 1, aliasVal);
			AddressMap::const_iterator it = aliasMap.find(tmp);
			if (it != aliasMap.end())
			{
				return it->second;
			}
		}
		*/
	}
	return NULL;
}

}
}
}

