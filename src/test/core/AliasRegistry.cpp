#include "AliasRegistry.hpp"

namespace smsc {
namespace test {
namespace core {

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
	//для aliasMap ничего удалять не нужно, т.к. объекты в aliasMap и addrMap общие
}

void AliasRegistry::putAlias(const TestAliasData& data)
{
	TestAliasData* tmp = new TestAliasData(data);
	addrMap[data.origAddr].push_back(tmp);
	aliasMap[data.origAlias].push_back(tmp);
}

void AliasRegistry::clear()
{
	addrMap.clear();
	aliasMap.clear();
}

const AliasRegistry::AliasList AliasRegistry::findAliasByAddress(const Address& addr) const
{
	AliasList res;
	AddressMap::const_iterator it = addrMap.find(addr);
	if (it == addrMap.end())
	{
		return res;
	}
    const AliasList& list = it->second;
	//список алиасов с максимальными addrMatch
	for (int i = 0; i < list.size(); i++)
	{
		if (!list[i]->addrMatch /*|| list[i]->addrToAliasException*/)
		{
			continue;
		}
		if (!res.size())
		{
			res.push_back(list[i]);
			continue;
		}
		if (list[i]->addrMatch > res.back()->addrMatch)
		{
			res.clear();
			res.push_back(list[i]);
			continue;
		}
		if (list[i]->addrMatch == res.back()->addrMatch)
		{
			res.push_back(list[i]);
			continue;
		}
	}
	return res;
}

const AliasRegistry::AliasList AliasRegistry::findAddressByAlias(const Address& alias) const
{
	AliasList res;
	AddressMap::const_iterator it = aliasMap.find(alias);
	if (it == aliasMap.end())
	{
		return res;
	}
    const AliasList& list = it->second;
	//список алиасов с максимальными aliasMatch
	for (int i = 0; i < list.size(); i++)
	{
		if (!list[i]->aliasMatch /*|| list[i]->aliasToAddrException*/)
		{
			continue;
		}
		if (!res.size())
		{
			res.push_back(list[i]);
			continue;
		}
		if (list[i]->aliasMatch > res.back()->aliasMatch)
		{
			res.clear();
			res.push_back(list[i]);
			continue;
		}
		if (list[i]->aliasMatch == res.back()->aliasMatch)
		{
			res.push_back(list[i]);
			continue;
		}
	}
	return res;
}

}
}
}

