#include "SmeRegistry.hpp"
#include "test/util/Util.hpp"
#include <fstream>
#include <sstream>

namespace smsc {
namespace test {
namespace core {

using namespace std;
using smsc::test::sms::operator<<;
using smsc::test::sms::operator==;
using namespace smsc::test::sms;
using namespace smsc::test::util;

SmeRegistry::~SmeRegistry()
{
	clear();
}

bool SmeRegistry::registerSme(const Address& smeAddr, const SmeInfo& sme,
	bool pduReg, bool externalSme)
{
	if (addrMap.find(smeAddr) != addrMap.end() ||
		smeIdMap.find(sme.systemId) != smeIdMap.end())
	{
		return false;
	}
	SmeData* smeData = new SmeData(smeAddr, sme, pduReg ? new PduRegistry() : NULL);
	addrMap[smeAddr] = smeData;
	smeIdMap[sme.systemId] = smeData;
	if (!externalSme)
	{
		addrList.push_back(new Address(smeAddr));
	}
	__trace2__("SmeRegistry::registerSme(): smeAddr = %s, smeId = %s, pduReg = %p, externalSme = %s",
		str(smeAddr).c_str(), sme.systemId.c_str(), smeData->pduReg, externalSme ? "true" : "false");
	return true;
}

bool SmeRegistry::registerAddress(const Address& addr)
{
	if (addrMap.find(addr) != addrMap.end())
	{
		return false;
	}
	addrMap[addr] = NULL;
	addrList.push_back(new Address(addr));
	__trace2__("SmeRegistry::registerAddress(): addr = %s", str(addr).c_str());
	return true;
}

void SmeRegistry::deleteSme(const SmeSystemId& smeId)
{
	SmeIdMap::iterator it = smeIdMap.find(smeId);
	if (it == smeIdMap.end())
	{
		return;
	}
	SmeData* smeData = it->second;
	smeIdMap.erase(it);
	bool res = addrMap.erase(smeData->smeAddr);
	__require__(res);
	for (AddressList::iterator it2 = addrList.begin(); it2 != addrList.end(); it2++)
	{
		if (**it2 == smeData->smeAddr)
		{
			delete *it2;
			addrList.erase(it2);
			break;
		}
	}
	delete smeData;
	__unreachable__("Address not found");
}

void SmeRegistry::bindSme(const SmeSystemId& smeId)
{
	SmeIdMap::iterator it = smeIdMap.find(smeId);
	__require__(it != smeIdMap.end());
	it->second->bound = true;
	__trace2__("SmeRegistry::bindSme(): smeId = %s", smeId.c_str());
}

void SmeRegistry::clear()
{
	//__require__(addrMap.size() == smeIdMap.size());
	for (AddressMap::iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		SmeData* smeData = it->second;
		if (smeData)
		{
			delete smeData;
		}
	}
	for (int i = 0; i < addrList.size(); i++)
	{
		delete addrList[i];
	}
	addrMap.clear();
	smeIdMap.clear();
	addrList.clear();
}

int SmeRegistry::size()
{
	//__require__(addrMap.size() == smeIdMap.size());
	return smeIdMap.size();
}

const SmeInfo* SmeRegistry::getSme(const SmeSystemId& smeId) const
{
	SmeIdMap::const_iterator it = smeIdMap.find(smeId);
	return (it == smeIdMap.end() ? NULL : &it->second->sme);
}

const SmeInfo* SmeRegistry::getSme(const Address& smeAddr) const
{
	AddressMap::const_iterator it = addrMap.find(smeAddr);
	return (it == addrMap.end() || !it->second ? NULL : &it->second->sme);
}

PduRegistry* SmeRegistry::getPduRegistry(const Address& smeAddr) const
{
	AddressMap::const_iterator it = addrMap.find(smeAddr);
	return (it == addrMap.end() || !it->second ? NULL : it->second->pduReg);
}

const Address* SmeRegistry::getRandomAddress() const
{
	return addrList[rand0(addrList.size() - 1)];
}

const vector<const Address*>& SmeRegistry::getAddressList()
{
	return addrList;
}

SmeRegistry::SmeIterator* SmeRegistry::iterator() const
{
	return new SmeIterator(smeIdMap.begin(), smeIdMap.end());
}

bool SmeRegistry::isSmeBound(const SmeSystemId& smeId) const
{
	SmeIdMap::const_iterator it = smeIdMap.find(smeId);
	return (it != smeIdMap.end() && it->second->bound);
}

void SmeRegistry::dump(FILE* log) const
{
	for (AddressMap::const_iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		const SmeData* smeData = it->second;
		ostringstream os;
		os << smeData->smeAddr;
		fprintf(TRACE_LOG_STREAM, "Sme = (systemId = %s, address = %s)\n",
			smeData->sme.systemId.c_str(), os.str().c_str());
		smeData->pduReg->dump(TRACE_LOG_STREAM);
	}
}

}
}
}

