#ifndef TEST_CORE_ALIAS_REGISTRY
#define TEST_CORE_ALIAS_REGISTRY

#include "sms/sms.h"
#include "test/sms/SmsUtil.hpp"
#include "AliasUtil.hpp"
#include <map>
#include <vector>
#include <memory>

namespace smsc {
namespace test {
namespace core {

using std::map;
using std::vector;
using std::auto_ptr;
using smsc::sms::Address;
using smsc::test::sms::ltAddress;

class AliasRegistry
{
	typedef map<const Address, AliasHolder*, ltAddress> AddressMap;

public:
	struct AliasIterator
	{
		AddressMap::const_iterator it1;
		AddressMap::const_iterator it2;
		AliasIterator(AddressMap::const_iterator i1, AddressMap::const_iterator i2)
			: it1(i1), it2(i2) {}
		const AliasHolder* next();
	};
	
	AliasRegistry(){}

	virtual ~AliasRegistry();

	bool putAlias(const AliasInfo& alias);

	void clear();
	
	AliasIterator* iterator() const;

	auto_ptr<const Address> findAliasByAddress(const Address& addr,
		const AliasInfo** aliasInfo = NULL) const;

	auto_ptr<const Address> findAddressByAlias(const Address& alias,
		const AliasInfo** aliasInfo = NULL) const;

private:
	AddressMap addrMap;
	AddressMap aliasMap;

	int getPartLen(const char* str, char ch);
	int compareAddr(const Address& a1, const Address& a2);
	bool checkAddr2Alias2AddrTransformation(const AliasInfo& alias1,
		const AliasInfo& alias2);
};

}
}
}

#endif /* TEST_CORE_ALIAS_REGISTRY */

