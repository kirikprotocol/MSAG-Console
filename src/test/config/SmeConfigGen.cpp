#include "SmeConfigGen.hpp"
#include "smeman/smeinfo.h"
#include <fstream>

namespace smsc {
namespace test {
namespace config {

using namespace std;
using smsc::smeman::SmeInfo;
using namespace smsc::test::util;

void SmeConfigGen::saveConfig(const char* configFileName)
{
	__require__(configFileName);
	__decl_tc__;
	ofstream os(configFileName);
	os << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << endl;
	os << "<!DOCTYPE records SYSTEM \"SmeRecords.dtd\">" << endl;
	os << "<records>" << endl;
	SmeRegistry::SmeIterator* it = smeReg->iterator();
	__require__(it);
	while (const SmeInfo* sme = it->next())
	{
		__tc__("smeConfig.correctSme");
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
		os << "\t<param name=\"wantAlias\" value=\"" <<
			(sme->wantAlias ? "yes" : "no") << "\"/>" << endl;
		os << "\t<param name=\"timeout\" value=\"" << sme->timeout <<
            "\"/>" << endl;
		os << "\t<param name=\"priority\" value=\"" << sme->priority <<
            "\"/>" << endl;
		os << "</smerecord>" << endl;
		__tc_ok__;
	}
	os << "</records>" << endl;
}

}
}
}

