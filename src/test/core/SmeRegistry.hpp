#ifndef TEST_CORE_SME_REGISTRY
#define TEST_CORE_SME_REGISTRY

#include "sms/sms.h"
#include "PduRegistry.hpp"
#include "test/sms/SmsUtil.hpp"
#include "smeman/smetypes.h"
#include "smeman/smeinfo.h"
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
using smsc::smeman::SmeInfo;
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
		PduRegistry* pduReg;
		SmeInfo* sme;
		SmeData(PduRegistry* reg, SmeInfo* smeInfo)
			: pduReg(reg), sme(smeInfo) {}
		SmeData(const SmeData& data)
			: pduReg(data.pduReg), sme(data.sme) {}
	};
	typedef map<const Address, SmeData, ltAddress> AddressMap;
	typedef set<SmeSystemId> SmeSystemIdSet;
	AddressMap addrMap;
	AddressList addrList;
	SmeSystemIdSet smeIdSet;

public:
	SmeRegistry() {}
	~SmeRegistry();

	void registerSme(const Address& smeAddr, SmeInfo* sme);

	void clear();
	
	PduRegistry* getPduRegistry(const Address& smeAddr) const;

	const Address* getRandomAddress() const;

	bool isSmeAvailable(const SmeSystemId& smeId) const;

	void saveConfig(const char* configFileName);
	
	void dump(FILE* log);
};

}
}
}

#endif /* TEST_CORE_SME_REGISTRY */
