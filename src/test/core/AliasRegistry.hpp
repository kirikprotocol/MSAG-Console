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

	const AliasHolder* findAliasByAddress(const Address& addr) const;

	const AliasHolder* findAddressByAlias(const Address& alias) const;

private:
	AddressMap addrMap;
	AddressMap aliasMap;

	int getPartLen(const char* str, char ch);
	int compareAddr(const Address& a1, const Address& a2);
	bool checkInverseTransformation(const Address& src1, const Address& dest1,
		const Address& src2, const Address& dest2);
};

}
}
}

#endif /* TEST_CORE_ALIAS_REGISTRY */

