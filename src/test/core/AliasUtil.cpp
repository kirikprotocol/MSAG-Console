#include "AliasUtil.hpp"
#include "test/sms/SmsUtil.hpp"

namespace smsc {
namespace test {
namespace core {

using smsc::test::sms::operator<<;

ostream& operator<< (ostream& os, const AliasInfo& alias)
{
	os << "alias = " << alias.alias;
	os << ", addr = " << alias.addr;
	return os;
}

ostream& operator<< (ostream& os, const TestAliasData& data)
{
	os << "addrMatch = " << data.addrMatch;
	os << ", addrToAliasException = " << (data.addrToAliasException ? "true" : "false");
	os << ", origAddr = " << data.origAddr;
	os << ", destAlias = " << data.destAlias;

	os << ", aliasMatch = " << data.aliasMatch;
	os << ", aliasToAddrException = " << (data.aliasToAddrException ? "true" : "false");
	os << ", origAlias = " << data.origAlias;
	os << ", destAddr = " << data.destAddr;

	os << ", alias = {" << *data.alias << "}";
}

}
}
}

