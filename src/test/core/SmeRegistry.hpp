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
	struct SmeData
	{
		const Address smeAddr;
		const SmeInfo sme;
		PduRegistry* pduReg;
		bool bound;
		SmeData(const Address& _smeAddr, const SmeInfo& _sme, PduRegistry* _pduReg)
			: smeAddr(_smeAddr), sme(_sme), pduReg(_pduReg), bound(false) {}
		~SmeData()
		{
			if (pduReg)
			{
				delete pduReg;
			}
		}
	};
	typedef map<const Address, SmeData*, ltAddress> AddressMap;
	typedef map<const SmeSystemId, SmeData*> SmeIdMap;
	typedef vector<const Address*> AddressList;

	AddressMap addrMap;
	SmeIdMap smeIdMap;
	AddressList addrList;

public:
	struct SmeIterator
	{
		SmeIdMap::const_iterator it1;
		SmeIdMap::const_iterator it2;
		SmeIterator(SmeIdMap::const_iterator i1, SmeIdMap::const_iterator i2)
			: it1(i1), it2(i2) {}
		const SmeInfo* next()
		{
			return (it1 != it2 ? &(it1++)->second->sme : NULL);
		}
	};

public:
	SmeRegistry() {}
	~SmeRegistry();

	bool registerSme(const Address& smeAddr, const SmeInfo& sme, bool pduReg = true);

	void deleteSme(const SmeSystemId& smeId);

	void bindSme(const SmeSystemId& smeId);

	void clear();

	int size();

	const SmeInfo* getSme(const SmeSystemId& smeId) const;
	
	PduRegistry* getPduRegistry(const Address& smeAddr) const;

	const Address* getRandomAddress() const;

	SmeIterator* iterator() const;

	bool isSmeBound(const SmeSystemId& smeId) const;

	void dump(FILE* log) const;
};

}
}
}

#endif /* TEST_CORE_SME_REGISTRY */
