#include "SmeRegistry.hpp"
#include "test/util/Util.hpp"
#include <fstream>
#include <sstream>

namespace smsc {
namespace test {
namespace core {

using namespace std;
using smsc::test::sms::operator<<;
using namespace smsc::test::util;

SmeRegistry::~SmeRegistry()
{
	clear();
}

void SmeRegistry::registerSme(const Address& smeAddr, SmeInfo* sme)
{
	if (sme)
	{
		SmeData smeData(new PduRegistry(), new SmeInfo(*sme));
		addrMap.insert(AddressMap::value_type(smeAddr, smeData));
		smeIdSet.insert(sme->systemId);
	}
	else
	{
		SmeData smeData(new PduRegistry(), (sme ? new SmeInfo(*sme) : NULL));
		addrMap.insert(AddressMap::value_type(smeAddr, SmeData(NULL, NULL)));
	}
	addrList.push_back(new Address(smeAddr));
}

void SmeRegistry::clear()
{
	for (AddressMap::iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		if (it->second.sme)
		{
			delete it->second.sme;
		}
		if (it->second.pduReg)
		{
			delete it->second.pduReg;
		}
	}
	for (int i = 0; i < addrList.size(); i++)
	{
		delete addrList[i];
	}
	addrMap.clear();
	smeIdSet.clear();
	addrList.clear();
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

bool SmeRegistry::isSmeAvailable(const SmeSystemId& smeId) const
{
	SmeSystemIdSet::const_iterator it = smeIdSet.find(smeId);
	return (it != smeIdSet.end());
}

void SmeRegistry::saveConfig(const char* configFileName)
{
	__require__(configFileName);
	ofstream os(configFileName);
	os << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << endl;
	os << "<!DOCTYPE records SYSTEM \"SmeRecords.dtd\">" << endl;
	os << "<records>" << endl;
	int i = 0;
	for (AddressMap::const_iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		SmeInfo* sme = it->second.sme;
		if (sme)
		{
			//sme->hostname;
			//sme->disabled;
			os << "<smerecord type=\"smpp\" uid=\"" << i++ << "\">" << endl;
			os << "\t<param name=\"typeOfNumber\" value=\"" <<
				sme->typeOfNumber << "\"/>" << endl;
			os << "\t<param name=\"numberingPlan\" value=\"" <<
				sme->numberingPlan << "\"/>" << endl;
			os << "\t<param name=\"interfaceVersion\" value=\"" <<
				sme->interfaceVersion << "\"/>" << endl;
			os << "\t<param name=\"systemType\" value=\"" <<
				sme->systemType << "\"/>" << endl;
			os << "\t<param name=\"systemId\" value=\"" <<
				sme->systemId << "\"/>" << endl;
			os << "\t<param name=\"password\" value=\"" <<
				sme->password << "\"/>" << endl;
			os << "\t<param name=\"addrRange\" value=\"" <<
				sme->rangeOfAddress << "\"/>" << endl;
			os << "\t<param name=\"smeN\" value=\"" <<
				sme->SME_N << "\"/>" << endl;
			os << "</smerecord>" << endl;
		}
	}
	os << "</records>" << endl;
}

void SmeRegistry::dump(FILE* log)
{
	for (AddressMap::const_iterator it = addrMap.begin(); it != addrMap.end(); it++)
	{
		const SmeData& smeData = it->second;
		if (smeData.sme)
		{
			ostringstream os;
			os << it->first;
			fprintf(TRACE_LOG_STREAM, "Sme = (systemId = %s, address = %s)\n",
				smeData.sme->systemId.c_str(), os.str().c_str());
			smeData.pduReg->dump(TRACE_LOG_STREAM);
		}
	}
}

}
}
}

