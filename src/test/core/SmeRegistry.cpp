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
		delete it->second;
	}
	for (int i = 0; i < addrList.size(); i++)
	{
		delete addrList[i];
	}
}

void SmeRegistry::registerSme(const Address& smeAddr)
{
	addrMap[smeAddr] = new PduRegistry();
	addrList.push_back(new Address(smeAddr));
}

void SmeRegistry::registerAddressWithNoSme(const Address& addr)
{
	addrMap[addr] = NULL;
}
	
PduRegistry* SmeRegistry::getPduRegistry(const Address& smeAddr) const
{
	AddressMap::const_iterator it = addrMap.find(smeAddr);
	return (it == addrMap.end() ? NULL : it->second);
}

const Address* SmeRegistry::getRandomAddress() const
{
	return addrList[rand0(addrList.size() - 1)];
}

bool SmeRegistry::isSmeRegistered(const Address& smeAddr) const
{
	AddressMap::const_iterator it = addrMap.find(smeAddr);
	return (it != addrMap.end());
}

const Address& SmeRegistry::getSmscAddr() const
{
	return smscAddr;
}

}
}
}

