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

SmeRegistry::SmeData::~SmeData()
{
	for (int i = 0; i < smeAddr.size(); i++)
	{
		__require__(smeAddr[i]);
		delete smeAddr[i];
	}
	if (pduReg)
	{
		delete pduReg;
	}
}

SmeRegistry::~SmeRegistry()
{
	clear();
}

bool SmeRegistry::registerSme(const Address& smeAddr, const SmeInfo& sme,
	bool pduReg, bool externalSme)
{
	vector<const Address*> tmp;
	tmp.push_back(new Address(smeAddr));
	return registerSme(tmp, sme, pduReg, externalSme);
}

bool SmeRegistry::registerSme(const vector<const Address*>& smeAddr,
	const SmeInfo& sme, bool pduReg, bool externalSme)
{
	//проверки
	for (int i = 0; i < smeAddr.size(); i++)
	{
		if (addrMap.find(*smeAddr[i]) != addrMap.end())
		{
			return false;
		}
	}
	if (smeIdMap.find(sme.systemId) != smeIdMap.end())
	{
		return false;
	}
	//регистрация
	SmeData* smeData = new SmeData(smeAddr, sme, pduReg ? new PduRegistry() : NULL);
	for (int i = 0; i < smeAddr.size(); i++)
	{
		addrMap[*smeAddr[i]] = smeData;
		if (!externalSme)
		{
			addrList.push_back(new Address(*smeAddr[i]));
		}
	}
	smeIdMap[sme.systemId] = smeData;
#ifndef DISABLE_TRACING
	ostringstream os;
	for (int i = 0; i < smeAddr.size(); i++)
	{
		if (i)
		{
			os << ",";
		}
		os << *smeAddr[i];
	}
	__trace2__("SmeRegistry::registerSme(): smeId = %s, smeAddr = {%s}, pduReg = %p, externalSme = %s",
		sme.systemId.c_str(), os.str().c_str(), smeData->pduReg, externalSme ? "true" : "false");
#endif
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
	for (int i = 0; i < smeData->smeAddr.size(); i++)
	{
		bool res = addrMap.erase(*smeData->smeAddr[i]);
		__require__(res);
		bool res2 = false;
		for (AddressList::iterator it2 = addrList.begin(); it2 != addrList.end(); it2++)
		{
			if (**it2 == *smeData->smeAddr[i])
			{
				delete *it2;
				addrList.erase(it2);
				res2 = true;
				break;
			}
		}
		__require__(res2);
	}
	delete smeData;
}

void SmeRegistry::bindSme(const SmeSystemId& smeId, SmeType smeType)
{
	SmeIdMap::iterator it = smeIdMap.find(smeId);
	__require__(it != smeIdMap.end());
	it->second->smeType = smeType;
	__trace2__("SmeRegistry::bindSme(): smeId = %s, smeType = %d",
		smeId.c_str(), smeType);
}

void SmeRegistry::clear()
{
	//__require__(addrMap.size() == smeIdMap.size());
	for (SmeIdMap::iterator it = smeIdMap.begin(); it != smeIdMap.end(); it++)
	{
		SmeData* smeData = it->second;
		__require__(smeData);
		delete smeData;
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

SmeType SmeRegistry::getSmeBindType(const SmeSystemId& smeId) const
{
	SmeIdMap::const_iterator it = smeIdMap.find(smeId);
	return (it == smeIdMap.end() ? SME_NOT_BOUND : it->second->smeType);
}

SmeType SmeRegistry::getSmeBindType(const Address& smeAddr) const
{
	AddressMap::const_iterator it = addrMap.find(smeAddr);
	return (it == addrMap.end() ? SME_NOT_BOUND : it->second->smeType);
}

}
}
}

