#include "AliasConfigGen.hpp"
#include "test/core/AliasUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include <fstream>

namespace smsc {
namespace test {
namespace config {

using namespace std;
using smsc::alias::AliasInfo;
using smsc::test::core::AliasHolder;
using smsc::test::sms::SmsUtil;

void AliasConfigGen::saveConfig(const char* configFileName)
{
	__require__(configFileName);
	ofstream os(configFileName);
	os << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << endl;
	os << "<!DOCTYPE aliases SYSTEM \"AliasRecords.dtd\">" << endl;
	os << "<aliases>" << endl;
	AliasRegistry::AliasIterator* it = aliasReg->iterator();
	__require__(it);
	while (const AliasHolder* aliasHolder = it->next())
	{
		const AliasInfo& aliasInfo = aliasHolder->aliasInfo;
		auto_ptr<char> addr = SmsUtil::configString(aliasInfo.addr);
		auto_ptr<char> alias = SmsUtil::configString(aliasInfo.alias);
		os << "\t<record addr=\"" << addr.get() <<
			"\" alias=\"" << alias.get() << "\"/>" << endl;
	}
	delete it;
	os << "</aliases>" << endl;
}

}
}
}

