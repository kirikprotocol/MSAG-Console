#include "AliasRegistry.hpp"
#include <fstream>

namespace smsc {
namespace test {
namespace core {

using namespace std;
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

void AliasRegistry::saveConfig(const char* configFileName)
{
	__require__(configFileName);
	__require__(addrMap.size() == aliasMap.size());
	ofstream os(configFileName);
	os << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << endl;
	os << "<!DOCTYPE aliases SYSTEM \"AliasRecords.dtd\">" << endl;
	os << "<aliases>" << endl;
	for (AddressMap::const_iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		const AliasInfo& aliasInfo = it->second->aliasInfo;
		AddressValue addrVal, aliasVal;
		aliasInfo.addr.getValue(addrVal);
		aliasInfo.alias.getValue(aliasVal);
		os << "\t<record addr=\"tni:" << aliasInfo.addr.getTypeOfNumber() <<
			",npi:" << aliasInfo.addr.getNumberingPlan() << ",val:" << addrVal <<
			"\" alias=\"tni:" << aliasInfo.alias.getTypeOfNumber() <<
			",npi:" << aliasInfo.alias.getNumberingPlan() << ",val:" << aliasVal <<
			"\"/>" << endl;
	}
	os << "</aliases>" << endl;
}

}
}
}

