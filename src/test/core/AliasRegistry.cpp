#include "AliasRegistry.hpp"

namespace smsc {
namespace test {
namespace core {

using namespace smsc::sms; //AddressValue, constants

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

void AliasRegistry::saveConfig(FILE* cfg)
{
	__require__(cfg);
	__require__(addrMap.size() == aliasMap.size());
	for (AddressMap::const_iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		//typedef map<const Address, AliasHolder*, ltAddress> AddressMap;
	}
}

}
}
}

