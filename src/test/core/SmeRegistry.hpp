#ifndef TEST_CORE_SME_REGISTRY
#define TEST_CORE_SME_REGISTRY

#include "sms/sms.h"
#include "PduRegistry.hpp"
#include "test/sms/SmsUtil.hpp"
#include <map>

namespace smsc {
namespace test {
namespace core {

using std::map;
using smsc::sms::Address;
using smsc::test::sms::ltAddress;

/**
 * Реестр sme. Все методы несинхронизованы и требуют внешней синхронизации.
 */
class SmeRegistry
{
	typedef map<const Address, PduRegistry*, ltAddress> AddressMap;
	AddressMap addrMap;

public:
	void registerSme(const Address& smeAddr);

	void registerAddressWithNoSme(const Address& addr);
	
	PduRegistry* getPduRegistry(const Address& smeAddr) const;

	const Address* getRandomAddress() const;

	bool isSmeRegistered(const Address& smeAddr) const;

	const Address& getSmscAddr() const;
};

}
}
}

#endif /* TEST_CORE_SME_REGISTRY */
