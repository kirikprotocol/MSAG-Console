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

bool AliasRegistry::checkAddr2Alias2AddrTransformation(const AliasInfo& alias1,
	const AliasInfo& alias2)
{
	//addr->alias->addr
	int len = compareAddr(alias1.alias, alias2.alias);
	if (len >= 0)
	{
		int destPartLen = getPartLen(alias1.alias.value, '?');
		if (len == destPartLen && alias1.hide)
		{
			int srcPartLen = getPartLen(alias1.addr.value, '?');
			if (compareAddr(alias1.addr, alias2.addr) != srcPartLen)
			{
				return false;
			}
			if (strcmp(alias2.addr.value + srcPartLen, alias2.alias.value + destPartLen) != 0)
			{
				return false;
			}
		}
	}
	return true;
}

bool AliasRegistry::putAlias(const AliasInfo& alias)
{
	//запретить знаки подстановки для hide = true
	__require__(!alias.hide || (!strchr(alias.alias.value, '?') && !strchr(alias.addr.value, '?')));

	//дублированные алиасы не сохраняются
	AddressMap::const_iterator it = addrMap.find(alias.addr);
	if (it != addrMap.end())
	{
		ostringstream os;
		os << "AliasRegistry::putAlias(): " << alias <<
			" duplicates exising " << (it->second->aliasInfo);
		__trace2__("%s", os.str().c_str());
		return false;
	}
	AddressMap::const_iterator it2 = aliasMap.find(alias.alias);
	if (it2 != aliasMap.end())
	{
		ostringstream os;
		os << "AliasRegistry::putAlias(): " << alias <<
			" duplicates exising " << ((it2->second->aliasInfo));
		__trace2__("%s", os.str().c_str());
		return false;
	}
	//должно выполняться правило: d(a(A)) = A
	//проверка только для '?', '*' не проверяю
	__require__(addrMap.size() <= aliasMap.size());
	for (AddressMap::const_iterator it = aliasMap.begin(); it != aliasMap.end(); it++)
	{
		const AliasInfo& info = it->second->aliasInfo;
		//addr->alias->addr
		if (!checkAddr2Alias2AddrTransformation(alias, info) ||
			!checkAddr2Alias2AddrTransformation(info, alias))
		{
			ostringstream os;
			os << "AliasRegistry::putAlias(): " << alias <<
				" conflicts with exising " << info;
			__trace2__("%s", os.str().c_str());
			return false;
		}
		/*
		//alias->addr->alias
		if (!checkAlias2Addr2AliasTransformation(alias, info) ||
			!checkAlias2Addr2AliasTransformation(info, alias))
		{
			ostringstream os;
			os << "AliasRegistry::putAlias(): " << alias <<
				" conflicts with exising " << info;
			__trace2__("%s", os.str().c_str());
			return false;
		}
		*/
	}
	//добавить алиас
	AliasHolder* holder = new AliasHolder(alias);
	if (alias.hide)
	{
		addrMap[alias.addr] = holder;
	}
	aliasMap[alias.alias] = holder;
	{
		ostringstream os;
		os << "AliasRegistry::putAlias(): " << alias << " added successfully";
		__trace2__("%s", os.str().c_str());
	}
	return true;
}

void AliasRegistry::clear()
{
	__require__(addrMap.size() <= aliasMap.size());
	for (AddressMap::iterator it = aliasMap.begin(); it != aliasMap.end(); it++)
	{
		delete it->second;
	}
	//для addrMap ничего удалять не нужно, т.к. объекты в aliasMap и addrMap общие
	addrMap.clear();
	aliasMap.clear();
}

AliasRegistry::AliasIterator* AliasRegistry::iterator() const
{
	__require__(addrMap.size() <= aliasMap.size());
	return new AliasIterator(aliasMap.begin(), aliasMap.end());
}

auto_ptr<const Address> AliasRegistry::findAliasByAddress(const Address& addr,
	const AliasInfo** aliasInfo) const
{
	Address tmp(addr);
	AddressValue addrVal;
	int addrLen = tmp.getValue(addrVal);
	AliasHolder* aliasHolder = NULL;
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
			aliasHolder = it->second;
			__require__(aliasHolder);
			break;
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
	//если не задано явное преобразование addr->alias, то alias=addr
	Address* alias(new Address(addr));
	if (aliasInfo)
	{
		*aliasInfo = aliasHolder ? &aliasHolder->aliasInfo : NULL;
	}
	if (aliasHolder)
	{
		bool res = aliasHolder->addressToAlias(addr, *alias);
		__require__(res);
	}
	return auto_ptr<const Address>(alias);
}

auto_ptr<const Address> AliasRegistry::findAddressByAlias(const Address& alias,
	const AliasInfo** aliasInfo) const
{
	Address tmp(alias);
	AddressValue aliasVal;
	int aliasLen = tmp.getValue(aliasVal);
	AliasHolder* aliasHolder = NULL;
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
			aliasHolder = it->second;
			__require__(aliasHolder);
			break;
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
	//если не задано явное преобразование alias->addr, то addr=alias
	Address* addr(new Address(alias));
	if (aliasInfo)
	{
		*aliasInfo = aliasHolder ? &aliasHolder->aliasInfo : NULL;
	}
	if (aliasHolder)
	{
		bool res = aliasHolder->aliasToAddress(alias, *addr);
		__require__(res);
	}
	return auto_ptr<const Address>(addr);
}

}
}
}

