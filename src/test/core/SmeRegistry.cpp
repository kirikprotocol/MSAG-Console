#include "SmeRegistry.hpp"
#include "test/util/Util.hpp"

namespace smsc {
namespace test {
namespace core {

using namespace smsc::test::util;

/*
	typedef map<const Address, PduRegistry*, ltAddress> AddressMap;
	AddressMap addrMap;
*/

SmeRegistry::~SmeRegistry()
{
	for (AddressMap::iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		delete it->second.pduReg;
	}
	for (int i = 0; i < addrList.size(); i++)
	{
		delete addrList[i];
	}
}

void SmeRegistry::registerSme(const Address& smeAddr, const SmeSystemId& smeId)
{
	addrMap.insert(AddressMap::value_type(smeAddr, SmeData(smeId, new PduRegistry())));
	smeIdSet.insert(smeId);
	addrList.push_back(new Address(smeAddr));
}

void SmeRegistry::registerAddressWithNoSme(const Address& addr)
{
	addrList.push_back(new Address(addr));
}
	
PduRegistry* SmeRegistry::getPduRegistry(const Address& smeAddr) const
{
	AddressMap::const_iterator it = addrMap.find(smeAddr);
	return (it == addrMap.end() ? NULL : it->second.pduReg);
}

const Address* SmeRegistry::getRandomAddress() const
{
	return addrList[rand0(addrList.size() - 1)];
}

bool SmeRegistry::isSmeRegistered(const SmeSystemId& smeId) const
{
	SmeSystemIdSet::const_iterator it = smeIdSet.find(smeId);
	return (it != smeIdSet.end());
}

const SmeRegistry::AddressList& SmeRegistry::list()
{
	return addrList;
}

}
}
}

