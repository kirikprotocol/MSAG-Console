#ifndef TEST_CORE_ALIAS_UTIL
#define TEST_CORE_ALIAS_UTIL

#include "sms/sms.h"
#include "alias/aliasman.h"
#include <ostream>

namespace smsc {
namespace test {
namespace core {

using std::ostream;
using smsc::sms::Address;
using smsc::alias::AliasInfo;

struct TestAliasData
{
	float addrMatch;
	bool addrMatchException;
	const Address origAddr;
	Address destAlias;

	float aliasMatch;
	bool aliasMatchException;
	const Address origAlias;
	Address destAddr;

	AliasInfo* alias;

	TestAliasData(const Address alias, const Address addr)
		: addrMatch(0.0), addrMatchException(false), origAddr(addr),
		aliasMatch(0.0), aliasMatchException(false), origAlias(alias), alias(NULL) {}

	TestAliasData(const TestAliasData& data)
		: addrMatch(data.addrMatch), addrMatchException(data.addrMatchException),
		origAddr(data.origAddr), destAlias(data.destAlias),
		aliasMatch(data.aliasMatch), aliasMatchException(data.aliasMatchException),
        origAlias(data.origAlias), destAddr(data.destAddr),
		alias(new AliasInfo(*data.alias)) {}

	~TestAliasData()
	{
		if (alias)
		{
			delete alias;
		}
	}
};

ostream& operator<< (ostream& os, const AliasInfo& alias);
ostream& operator<< (ostream& os, const TestAliasData& data);

}
}
}

#endif /* TEST_CORE_ALIAS_UTIL */

