#ifndef TEST_CORE_SME_REGISTRY
#define TEST_CORE_SME_REGISTRY

#include "sms/sms.h"
#include "PduRegistry.hpp"
#include "test/sms/SmsUtil.hpp"
#include "smeman/smetypes.h"
#include <map>
#include <vector>
#include <set>

namespace smsc {
namespace test {
namespace core {

using std::map;
using std::vector;
using std::set;
using smsc::sms::Address;
using smsc::smeman::SmeSystemId;
using smsc::test::sms::ltAddress;

/**
 * Реестр sme. Все методы несинхронизованы и требуют внешней синхронизации.
 */
class SmeRegistry
{
public:
	typedef vector<const Address*> AddressList;

private:
	struct SmeData
	{
		const SmeSystemId systemId;
		PduRegistry* pduReg;
		SmeData(const SmeSystemId& id, PduRegistry* reg)
			: systemId(id), pduReg(reg) {}
		SmeData(const SmeData& data)
			: systemId(data.systemId), pduReg(data.pduReg) {}
	};
	typedef map<const Address, SmeData, ltAddress> AddressMap;
	typedef set<SmeSystemId> SmeSystemIdSet;
	AddressMap addrMap;
	AddressList addrList;
	SmeSystemIdSet smeIdSet;

public:
	SmeRegistry() {}
	~SmeRegistry();

	void registerSme(const Address& smeAddr, const SmeSystemId& smeId);

	void registerAddressWithNoSme(const Address& addr);
	
	PduRegistry* getPduRegistry(const Address& smeAddr) const;

	const Address* getRandomAddress() const;

	bool isSmeRegistered(const SmeSystemId& smeId) const;

	const AddressList& list();
};

}
}
}

#endif /* TEST_CORE_SME_REGISTRY */
