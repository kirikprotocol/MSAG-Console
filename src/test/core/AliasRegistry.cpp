#include "AliasRegistry.hpp"
#include "test/sms/SmsUtil.hpp"
#include <fstream>
#include <sstream>

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

inline int AliasRegistry::getPartLen(const char* str, char ch)
{
	const char* tmp = strchr(str, ch);
	if (tmp)
	{
		return (tmp - str);
	}
	return strlen(str);
}

inline int AliasRegistry::compareAddr(const Address& a1, const Address& a2)
{
	if (a1.getTypeOfNumber() != a2.getTypeOfNumber() ||
		a1.getNumberingPlan() != a2.getNumberingPlan() ||
		a1.getLenght() != a2.getLenght())
	{
		return -1;
	}
	for (int i = 0; i < a1.getLenght(); i++)
	{
		if (a1.value[i] != a2.value[i])
		{
			return i;
		}
	}
	return a1.getLenght();
}

bool AliasRegistry::checkInverseTransformation(const Address& src1,
	const Address& dest1, const Address& src2, const Address& dest2)
{
	//new: src1, dest1
	//existing: src2, dest2
	//check: src1->dest1->src1
	int len = compareAddr(dest1, dest2);
	if (len >= 0)
	{
		int destPartLen = getPartLen(dest1.value, '?');
		if (len == destPartLen)
		{
			int srcPartLen = getPartLen(src1.value, '?');
			if (compareAddr(src1, src2) != srcPartLen)
			{
				return false;
			}
			if (strcmp(src2.value + srcPartLen, dest2.value + destPartLen) != 0)
			{
				return false;
			}
		}
	}
	return true;
}

bool AliasRegistry::putAlias(const AliasInfo& alias)
{
	//дублированные алиасы не сохраняются
	if (addrMap.find(alias.addr) != addrMap.end() ||
		aliasMap.find(alias.alias) != aliasMap.end())
	{
		ostringstream os;
		os << alias;
		__trace2__("Duplicate alias: %s", os.str().c_str());
		return false;
	}
	//алиасы допускающие неоднозначные преобразования addr->alias->addr или
	//alias->addr->alias не сохраняются (проверка только для '?', '*' не проверяю)
	for (AddressMap::const_iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		const AliasInfo& info = it->second->aliasInfo;
		if (!checkInverseTransformation(alias.alias, alias.addr, info.alias, info.addr) ||
			!checkInverseTransformation(info.alias, info.addr, alias.alias, alias.addr) ||
			!checkInverseTransformation(alias.addr, alias.alias, info.addr, info.alias) ||
			!checkInverseTransformation(info.addr, info.alias, alias.addr, alias.alias))
		{
			ostringstream os;
			os << alias;
			__trace2__("Inverse transformation check failed for alias = %s",
				os.str().c_str());
		}
	}
	//добавить алиас
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
	//для aliasMap ничего удалять не нужно, т.к. объекты в aliasMap и addrMap общие
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

