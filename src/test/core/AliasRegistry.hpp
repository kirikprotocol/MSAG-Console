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
	typedef vector<const AliasHolder*> AliasList;

	AliasRegistry(){}

	virtual ~AliasRegistry();

	bool putAlias(const AliasInfo& alias);

	void clear();
	
	//AliasIterator* iterator();

	const AliasList findAliasByAddress(const Address& addr) const;

	const AliasList findAddressByAlias(const Address& alias) const;

private:
	typedef map<const Address, AliasList, ltAddress> AddressMap;
	AddressMap addrMap;
	AddressMap aliasMap;
};

}
}
}

#endif /* TEST_CORE_ALIAS_REGISTRY */

