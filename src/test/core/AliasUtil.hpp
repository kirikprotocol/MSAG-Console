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

class AliasHolder
{
	char ch;
	int addrPos;
	int aliasPos;

public:
	const AliasInfo aliasInfo;
	
	AliasHolder(const AliasInfo& alias);

	AliasHolder(const AliasHolder& holder)
		: aliasInfo(holder.aliasInfo), ch(holder.ch),
		addrPos(holder.addrPos), aliasPos(holder.aliasPos) {}

	~AliasHolder() {}
	
	bool aliasToAddress(const Address& alias, Address& addr) const;
	bool addressToAlias(const Address& addr, Address& alias) const;

};

ostream& operator<< (ostream& os, const AliasInfo& alias);
ostream& operator<< (ostream& os, const AliasHolder& holder);

}
}
}

#endif /* TEST_CORE_ALIAS_UTIL */

