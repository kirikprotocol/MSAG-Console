#include "AliasUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

using smsc::test::sms::operator<<;
using namespace smsc::sms; //AddressValue, constants

AliasHolder::AliasHolder(const AliasInfo& alias)
	: aliasInfo(alias)
{
	AddressValue val;
	aliasInfo.addr.getValue(val);
	char* tmp = strchr(val, '?');
	if (tmp)
	{
		ch = '?';
		addrPos = tmp - val;
		aliasInfo.alias.getValue(val);
		tmp = strchr(val, '?');
		__require__(tmp);
		aliasPos = tmp - val;
		return;
	}
	tmp = strchr(val, '*');
	if (tmp)
	{
		ch = '*';
		addrPos = tmp - val;
		aliasInfo.alias.getValue(val);
		tmp = strchr(val, '*');
		__require__(tmp);
		aliasPos = tmp - val;
		return;
	}
	ch = ' ';
	addrPos = aliasInfo.addr.getLenght();
	aliasPos = aliasInfo.alias.getLenght();
}

bool AliasHolder::aliasToAddress(const Address& alias, Address& addr) const
{
	__require__(alias.getTypeOfNumber() ==  aliasInfo.alias.getTypeOfNumber());
	__require__(alias.getNumberingPlan() ==  aliasInfo.alias.getNumberingPlan());
	if (ch == ' ')
	{
		__require__(strcmp(alias.value, aliasInfo.alias.value) == 0);
		addr = aliasInfo.addr;
		return true;
	}
	__require__(aliasPos <= alias.getLenght());
	__require__(strncmp(alias.value, aliasInfo.alias.value, aliasPos) == 0);
	int len = addrPos + alias.getLenght() - aliasPos;
	if (len == 0 || len > MAX_ADDRESS_VALUE_LENGTH)
	{
		return false;
	}
	if (ch == '?')
	{
		__require__(alias.getLenght() == aliasInfo.alias.getLenght());
	}
	AddressValue tmp;
	memcpy(tmp, aliasInfo.addr.value, addrPos);
	memcpy(tmp + addrPos, alias.value + aliasPos, alias.getLenght() - aliasPos);
	addr.setTypeOfNumber(aliasInfo.addr.getTypeOfNumber());
	addr.setNumberingPlan(aliasInfo.addr.getNumberingPlan());
	addr.setValue(len, tmp);
	return true;
}

bool AliasHolder::addressToAlias(const Address& addr, Address& alias) const
{
	__require__(addr.getTypeOfNumber() ==  aliasInfo.addr.getTypeOfNumber());
	__require__(addr.getNumberingPlan() ==  aliasInfo.addr.getNumberingPlan());
	if (ch == ' ')
	{
		__require__(strcmp(addr.value, aliasInfo.addr.value) == 0);
		alias = aliasInfo.alias;
		return true;
	}
	__require__(addrPos <= addr.getLenght());
	__require__(strncmp(addr.value, aliasInfo.addr.value, addrPos) == 0);
	int len = aliasPos + addr.getLenght() - addrPos;
	if (len == 0 || len > MAX_ADDRESS_VALUE_LENGTH)
	{
		return false;
	}
	if (ch == '?')
	{
		__require__(addr.getLenght() == aliasInfo.addr.getLenght());
	}
	AddressValue tmp;
	memcpy(tmp, aliasInfo.alias.value, aliasPos);
	memcpy(tmp + aliasPos, addr.value + addrPos, addr.getLenght() - addrPos);
	alias.setTypeOfNumber(aliasInfo.alias.getTypeOfNumber());
	alias.setNumberingPlan(aliasInfo.alias.getNumberingPlan());
	alias.setValue(len, tmp);
	return true;
}

ostream& operator<< (ostream& os, const AliasInfo& alias)
{
	os << "alias = " << alias.alias;
	os << ", addr = " << alias.addr;
	return os;
}

ostream& operator<< (ostream& os, const AliasHolder& holder)
{
	os << holder.aliasInfo;
}

}
}
}

