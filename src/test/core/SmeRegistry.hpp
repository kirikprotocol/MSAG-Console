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

typedef enum
{
	SME_NO_ROUTE = 1,
	SME_NOT_BOUND = 2,
	SME_RECEIVER = 3,
	SME_TRANSMITTER = 4,
	SME_TRANSCEIVER = 5
} SmeType;

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
		SmeType smeType;
		SmeData(const Address& _smeAddr, const SmeInfo& _sme, PduRegistry* _pduReg)
		: smeAddr(_smeAddr), sme(_sme), pduReg(_pduReg), smeType(SME_NO_ROUTE) {}
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

	bool registerSme(const Address& smeAddr, const SmeInfo& sme,
		bool pduReg = true, bool externalSme = false);

	bool registerAddress(const Address& addr);

	void deleteSme(const SmeSystemId& smeId);

	void bindSme(const SmeSystemId& smeId, SmeType smeType);

	void clear();

	int size();

	const SmeInfo* getSme(const SmeSystemId& smeId) const;
	const SmeInfo* getSme(const Address& smeAddr) const;

	PduRegistry* getPduRegistry(const Address& smeAddr) const;

	const Address* getRandomAddress() const;
	const vector<const Address*>& getAddressList();

	SmeIterator* iterator() const;

	SmeType getSmeBindType(const SmeSystemId& smeId) const;
	SmeType getSmeBindType(const Address& smeAddr) const;

	void dump(FILE* log) const;
};

}
}
}

#endif /* TEST_CORE_SME_REGISTRY */
