#include "SmeConfigGen.hpp"
#include "smeman/smeinfo.h"
#include <fstream>

namespace smsc {
namespace test {
namespace config {

using namespace std;
using smsc::smeman::SmeInfo;

void SmeConfigGen::saveConfig(const char* configFileName)
{
	__require__(configFileName);
	ofstream os(configFileName);
	os << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << endl;
	os << "<!DOCTYPE records SYSTEM \"SmeRecords.dtd\">" << endl;
	os << "<records>" << endl;
	SmeRegistry::SmeIterator* it = smeReg->iterator();
	__require__(it);
	while (const SmeInfo* sme = it->next())
	{
		//sme.hostname;
		//sme.disabled;
		os << "<smerecord type=\"smpp\" uid=\"" << sme->systemId << "\">" << endl;
		os << "\t<param name=\"typeOfNumber\" value=\"" <<
			(int) sme->typeOfNumber << "\"/>" << endl;
		os << "\t<param name=\"numberingPlan\" value=\"" <<
			(int) sme->numberingPlan << "\"/>" << endl;
		os << "\t<param name=\"interfaceVersion\" value=\"" <<
			(int) sme->interfaceVersion << "\"/>" << endl;
		os << "\t<param name=\"systemType\" value=\"" <<
			sme->systemType << "\"/>" << endl;
		os << "\t<param name=\"password\" value=\"" <<
			sme->password << "\"/>" << endl;
		os << "\t<param name=\"addrRange\" value=\"" <<
			sme->rangeOfAddress << "\"/>" << endl;
		os << "\t<param name=\"smeN\" value=\"" <<
			sme->SME_N << "\"/>" << endl;
		os << "</smerecord>" << endl;
	}
	os << "</records>" << endl;
}

}
}
}

