#include "AliasRegistry.hpp"

namespace smsc {
namespace test {
namespace core {

using namespace smsc::sms; //AddressValue, constants

AliasRegistry::~AliasRegistry()
{
	for (AddressMap::iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		AliasList& list = it->second;
		for (int i = 0; i < list.size(); i++)
		{
			delete list[i];
		}
	}
	//дл€ aliasMap ничего удал€ть не нужно, т.к. объекты в aliasMap и addrMap общие
}

void AliasRegistry::putAlias(const AliasInfo& alias)
{
	//дублированные алиасы не сохран€ютс€
	if (addrMap.find(alias.addr) != addrMap.end() ||
		aliasMap.find(alias.alias) != aliasMap.end())
	{
		return;
	}
	AliasHolder* holder = new AliasHolder(alias);
	addrMap[alias.addr].push_back(holder);
	aliasMap[alias.alias].push_back(holder);
}

void AliasRegistry::clear()
{
	addrMap.clear();
	aliasMap.clear();
}

const AliasRegistry::AliasList AliasRegistry::findAliasByAddress(const Address& addr) const
{
#define __findAlias__() \
	AddressMap::const_iterator it = addrMap.find(tmp); \
	if (it != addrMap.end()) { return it->second; }

	Address tmp(addr);
	AddressValue addrVal;
	int addrLen = tmp.getValue(addrVal);
	for (int len = 0 ; len <= addrLen; len++)
	{
		if (len)
		{
			memset(addrVal + addrLen - len, '?', len);
			tmp.setValue(addrLen, addrVal);
		}
		__findAlias__();
		if (addrLen - len < MAX_ADDRESS_VALUE_LENGTH)
		{
			addrVal[addrLen - len] = '*';
			tmp.setValue(addrLen - len + 1, addrVal);
			__findAlias__();
		}
	}
	return AliasList();
}

const AliasRegistry::AliasList AliasRegistry::findAddressByAlias(const Address& alias) const
{
#define __findAddress__() \
	AddressMap::const_iterator it = aliasMap.find(tmp); \
	if (it != aliasMap.end()) { return it->second; }

	Address tmp(alias);
	AddressValue aliasVal;
	int aliasLen = tmp.getValue(aliasVal);
	for (int len = 0 ; len <= aliasLen; len++)
	{
		if (len)
		{
			memset(aliasVal + aliasLen - len, '?', len);
			tmp.setValue(aliasLen, aliasVal);
		}
		__findAddress__();
		if (aliasLen - len < MAX_ADDRESS_VALUE_LENGTH)
		{
			aliasVal[aliasLen - len] = '*';
			tmp.setValue(aliasLen - len + 1, aliasVal);
			__findAddress__();
		}
	}
	return AliasList();
}

}
}
}

