#include "AliasManagerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"

namespace smsc {
namespace test {
namespace alias {

using smsc::sms::AddressValue;
using smsc::util::Logger;
using smsc::test::sms::SmsUtil;
using namespace smsc::test::util;

Category& AliasManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmeManagerTestCases");
	return log;
}

TCResult* AliasManagerTestCases::addCorrectAlias(AliasInfo* aliasp)
{
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS);
	try
	{
		AliasInfo alias;
		SmsUtil::setupRandomCorrectAddress(&alias.addr);
		SmsUtil::setupRandomCorrectAddress(&alias.alias);
		aliasMan->addAlias(alias);
		if (aliasp)
		{
			*aliasp = alias;
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::addCorrectAlias(AliasInfo* aliasp,
	const AliasInfo& existentAlias, int num)
{
	int num1 = 3; int num2 = 3;
	TCSelector s(num, num1 * num2, 1000);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			AliasInfo alias(existentAlias);
			//адрес
			switch(s.value1(num1))
			{
				case 1: //отличающийся typeOfNumber
					alias.addr.setTypeOfNumber(alias.addr.getTypeOfNumber() + 1);
					break;
				case 2: //отличающийся numberingPlan
					alias.addr.setNumberingPlan(alias.addr.getNumberingPlan() + 1);
					break;
				case 3: //отличающийся addressValue
					{
						AddressValue addrVal;
						uint8_t addrLen = alias.addr.getValue(addrVal);
						addrVal[addrLen - 1] = '+';
						alias.addr.setValue(addrLen, addrVal);
					}
					break;
				default:
					throw s;
			}
			//алиас
			switch(s.value2(num1))
			{
				case 1: //отличающийся typeOfNumber
					alias.alias.setTypeOfNumber(alias.alias.getTypeOfNumber() + 1);
					break;
				case 2: //отличающийся numberingPlan
					alias.alias.setNumberingPlan(alias.alias.getNumberingPlan() + 1);
					break;
				case 3: //отличающийся addressValue
					{
						AddressValue addrVal;
						uint8_t addrLen = alias.alias.getValue(addrVal);
						addrVal[addrLen - 1] = '+';
						alias.alias.setValue(addrLen, addrVal);
					}
					break;
				default:
					throw s;
			}
			aliasMan->addAlias(alias);
			if (aliasp)
			{
				*aliasp = alias;
			}
		}
		catch(...)
		{
			error();
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::addIncorrectAlias(
	const AliasInfo& existentAlias, int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_ADD_INCORRECT_ALIAS, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			AliasInfo alias;
			SmsUtil::setupRandomCorrectAddress(&alias.addr);
			SmsUtil::setupRandomCorrectAddress(&alias.alias);
			switch(s.value())
			{
				case 1: //пустой адрес
					alias.addr.setValue(0, NULL);
					break;
				case 2: //пустой адрес
					alias.addr.setValue(0, "*");
					break;
				case 3: //пустой алиас
					alias.alias.setValue(0, NULL);
					break;
				case 4: //пустой алиас
					alias.alias.setValue(0, "*");
					break;
				default:
					throw s;
			}
			aliasMan->addAlias(alias);
			res->addFailure(s.value());
		}
		catch(...)
		{
			//ok
		}
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::deleteAliases()
{
	TCResult* res = new TCResult(TC_DELETE_ALIASES);
	try
	{
		AliasInfo alias;
		aliasMan->clean();
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::findExistentAliasByAddress(
	const AliasInfo& existentAlias)
{
	TCResult* res = new TCResult(TC_FIND_EXISTENT_ALIAS_BY_ADDRESS);
	try
	{
		Address alias;
		bool found = aliasMan->AddressToAlias(existentAlias.addr, alias);
		if (!found)
		{
			res->addFailure(101);
		}
		else if (!SmsUtil::compareAddresses(alias, existentAlias.alias))
		{
			res->addFailure(102);
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::findExistentAddressByAlias(
	const AliasInfo& existentAlias)
{
	TCResult* res = new TCResult(TC_FIND_EXISTENT_ADDRESS_BY_ALIAS);
	try
	{
		Address addr;
		bool found = aliasMan->AliasToAddress(existentAlias.alias, addr);
		if (!found)
		{
			res->addFailure(101);
		}
		else if (!SmsUtil::compareAddresses(addr, existentAlias.addr))
		{
			res->addFailure(102);
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::findNonExistentAliasByAddress(
	const Address& addr)
{
	TCResult* res = new TCResult(TC_FIND_NON_EXISTENT_ALIAS_BY_ADDRESS);
	try
	{
		Address alias;
		bool found = aliasMan->AddressToAlias(addr, alias);
		if (found)
		{
			res->addFailure(101);
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::findNonExistentAddressByAlias(
	const Address& alias)
{
	TCResult* res = new TCResult(TC_FIND_NON_EXISTENT_ADDRESS_BY_ALIAS);
	try
	{
		Address addr;
		bool found = aliasMan->AliasToAddress(alias, addr);
		if (found)
		{
			res->addFailure(101);
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::iterateAliases(const vector<AliasInfo*> sme)
{
	TCResult* res = new TCResult(TC_ITERATE_ALIASES);
	res->addFailure(100);
	debug(res);
	return res;
}

}
}
}

