#ifndef TEST_CORE_ALIAS_REGISTRY
#define TEST_CORE_ALIAS_REGISTRY

#include "sms/sms.h"
#include "test/sms/SmsUtil.hpp"
#include "AliasUtil.hpp"
#include <map>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using std::map;
using std::vector;
using smsc::sms::Address;
using smsc::test::sms::ltAddress;

class AliasRegistry
{
public:
	AliasRegistry(){}

	virtual ~AliasRegistry();

	bool putAlias(const AliasInfo& alias);

	void clear();
	
	//AliasIterator* iterator();

	const AliasHolder* findAliasByAddress(const Address& addr) const;

	const AliasHolder* findAddressByAlias(const Address& alias) const;

private:
	typedef map<const Address, AliasHolder*, ltAddress> AddressMap;
	AddressMap addrMap;
	AddressMap aliasMap;
};

}
}
}

#endif /* TEST_CORE_ALIAS_REGISTRY */

