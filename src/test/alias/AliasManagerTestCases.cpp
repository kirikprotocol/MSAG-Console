#include "AliasManagerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace alias {

using smsc::sms::AddressValue;
using smsc::util::Logger;
using smsc::test::sms::SmsUtil;
using smsc::test::sms::operator<<;
using smsc::test::core::operator<<;
using namespace smsc::sms; //constants
using namespace smsc::test::util;

AliasManagerTestCases::AliasManagerTestCases(AliasManager* manager,
	AliasRegistry* reg) : aliasMan(manager), aliasReg(reg)
{
	__require__(aliasMan);
	__require__(aliasReg);
}

Category& AliasManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmeManagerTestCases");
	return log;
}

void AliasManagerTestCases::debugAlias(const char* tc, int val,
	const AliasInfo* alias)
{
	ostringstream os;
	os << tc << "(" << val << "): " << *alias;
	getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
}

void AliasManagerTestCases::setupRandomAliasMatchWithQuestionMarks(
	AliasInfo* alias, int len)
{
	AddressValue addrVal, aliasVal, tmp;
	uint8_t addrLen = alias->addr.getValue(addrVal);
	uint8_t aliasLen = alias->alias.getValue(aliasVal);
	__require__(len >= 0 && len <= addrLen && len <= aliasLen);
	//alias -> addr
	memcpy(tmp, addrVal, addrLen - len);
	memset(tmp + addrLen - len, '?', len);
	alias->addr.setValue(addrLen, tmp);
	//addr -> alias
	memcpy(tmp, aliasVal, aliasLen - len);
	memset(tmp + aliasLen - len, '?', len);
	alias->alias.setValue(aliasLen, tmp);
}

void AliasManagerTestCases::setupRandomAliasMatchWithAsterisk(
	AliasInfo* alias, int adLen, int alLen)
{
	AddressValue addrVal, aliasVal, tmp;
	uint8_t addrLen = alias->addr.getValue(addrVal);
	uint8_t aliasLen = alias->alias.getValue(aliasVal);
	__require__(adLen >= 0 && adLen <= addrLen);
	__require__(alLen >= 0 && alLen <= aliasLen);
	__require__(addrLen - adLen < MAX_ADDRESS_VALUE_LENGTH);
	__require__(aliasLen - alLen < MAX_ADDRESS_VALUE_LENGTH);
	//alias -> addr
	memcpy(tmp, addrVal, addrLen - adLen);
	tmp[addrLen - adLen] = '*';
	alias->addr.setValue(addrLen - adLen + 1, tmp);
	//addr -> alias
	memcpy(tmp, aliasVal, aliasLen - alLen);
	tmp[aliasLen - alLen] = '*';
	alias->alias.setValue(aliasLen - alLen + 1, tmp);
}

TCResult* AliasManagerTestCases::addCorrectAliasMatch(AliasInfo* alias, int num)
{
	TCSelector s(num, 10);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS_MATCH, s.getChoice());
	getLog().debugStream() << "[" << thr_self() <<
		"]\taddCorrectAliasMatch(" << s.value() << ")";
	try
	{
		uint8_t addrLen = alias->addr.getLenght();
		uint8_t aliasLen = alias->alias.getLenght();
		int minLen = min(addrLen, aliasLen);
		int maxLen = max(addrLen, aliasLen);
		switch(s.value())
		{
			case 1: //адрес и алиас без знаков подстановки
				break;
			case 2: //адрес и алиас с одним или несколькими '?' в конце
				if (minLen > 1)
				{
					setupRandomAliasMatchWithQuestionMarks(alias, rand1(minLen - 1));
				}
				break;
			case 3: //адрес или алиас целиком из '?'
				setupRandomAliasMatchWithQuestionMarks(alias, minLen);
				break;
			case 4: //адрес со '*' в конце, алиас со '*' в конце
				if (addrLen > 1 && aliasLen > 1)
				{
					int adLen = rand1(addrLen - 1);
					int alLen = rand1(aliasLen - 1);
					setupRandomAliasMatchWithAsterisk(alias, adLen, alLen);
				}
				break;
			case 5: //адрес со '*' в конце, алиас = '*'
				if (addrLen > 1)
				{
					int adLen = rand1(addrLen - 1);
					//int alLen = aliasLen;
					setupRandomAliasMatchWithAsterisk(alias, adLen, aliasLen);
				}
				break;
			case 6: //адрес со '*' в конце, алиас равен оригиналу + '*' в конце
				if (addrLen > 1 && aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					int adLen = rand1(addrLen - 1);
					//int alLen = 0;
					setupRandomAliasMatchWithAsterisk(alias, adLen, 0);
				}
				break;
			case 7: //адрес = '*', алиас со '*' в конце
				if (aliasLen > 1)
				{
					//int adLen = addrLen;
					int alLen = rand1(aliasLen - 1);
					setupRandomAliasMatchWithAsterisk(alias, addrLen, alLen);
				}
				break;
			case 8: //адрес = '*', алиас = '*'
				{
					//int adLen = addrLen;
					//int alLen = aliasLen;
					setupRandomAliasMatchWithAsterisk(alias, addrLen, aliasLen);
				}
				break;
			case 9: //адрес равен оригиналу + '*' в конце, алиас со '*' в конце
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH && aliasLen > 1)
				{
					//int adLen = 0;
					int alLen = rand1(aliasLen - 1);
					setupRandomAliasMatchWithAsterisk(alias, 0, alLen);
				}
				break;
			case 10: //адрес равен оригиналу + '*' в конце, алиас равен оригиналу + '*' в конце
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH && aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					//int adLen = 0;
					//int alLen = 0;
					setupRandomAliasMatchWithAsterisk(alias, 0, 0);
				}
				break;
			default:
				throw s;
		}
		debugAlias("addCorrectAliasMatch", s.value(), alias);
		aliasMan->addAlias(*alias);
		aliasReg->putAlias(*alias);
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::addCorrectAliasNotMatchAddress(
	AliasInfo* alias, int num)
{
	TCSelector s(num, 5);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS_NOT_MATCH_ADDRESS, s.getChoice());
	getLog().debugStream() << "[" << thr_self() <<
		"]\taddCorrectAliasNotMatchAddress(" << s.value() << ")";
	try
	{
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = alias->addr.getValue(addrVal);
		uint8_t aliasLen = alias->alias.getValue(aliasVal);
		switch(s.value())
		{
			case 1: //в адресе отличается typeOfNumber
				{
					alias->addr.setTypeOfNumber(alias->addr.getTypeOfNumber() + 1);
				}
				break;
			case 2: //в адресе отличается numberingPlan
				{
					alias->addr.setNumberingPlan(alias->addr.getNumberingPlan() + 1);
				}
				break;
			case 3: //в адресе отличается addressValue
				{
					memcpy(tmp, addrVal, addrLen);
					tmp[rand0(addrLen - 1)] = '+';
					alias->addr.setValue(addrLen, tmp);
				}
				break;
			case 4: //в адресе '?' меньше, чем нужно
				if (addrLen > 1)
				{
					int len1 = rand1(min((int) aliasLen, addrLen - 1));
					int len2 = rand1(addrLen - len1);
					__require__(len1 > 0 && len2 > 0);
					__require__(len1 + len2 <= addrLen);
					__require__(len1 <= aliasLen);
					//alias -> addr
					memcpy(tmp, addrVal, addrLen - len1 - len2);
					memset(tmp + addrLen - len1 - len2, '?', len1);
					alias->addr.setValue(addrLen - len2, tmp);
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - len1);
					memset(tmp + aliasLen - len1, '?', len1);
					alias->alias.setValue(aliasLen, tmp);
				}
				break;
			case 5: //в адресе '?' больше, чем нужно
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH && aliasLen > 1)
				{
					int len = min(MAX_ADDRESS_VALUE_LENGTH - addrLen, aliasLen - 1);
					int len2 = rand1(len);
					int len1 = rand1(min(aliasLen - len2, (int) addrLen));
					__require__(len1 > 0 && len2 > 0);
					__require__(len1 <= addrLen);
					__require__(addrLen + len2 <= MAX_ADDRESS_VALUE_LENGTH);
					__require__(len1 + len2 <= aliasLen);
					//alias -> addr
					memcpy(tmp, addrVal, addrLen - len1);
					memset(tmp + addrLen - len1, '?', len1 + len2);
					alias->addr.setValue(addrLen + len2, tmp);
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - len1 - len2);
					memset(tmp + aliasLen - len1 - len2, '?', len1 + len2);
					alias->alias.setValue(aliasLen, tmp);
				}
				break;
			default:
				throw s;
		}
		debugAlias("addCorrectAliasNotMatchAddress", s.value(), alias);
		aliasMan->addAlias(*alias);
		aliasReg->putAlias(*alias);
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::addCorrectAliasNotMatchAlias(
	AliasInfo* alias, int num)
{
	TCSelector s(num, 5);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS_NOT_MATCH_ALIAS, s.getChoice());
	getLog().debugStream() << "[" << thr_self() <<
		"]\taddCorrectAliasNotMatchAlias(" << s.value() << ")";
	try
	{
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = alias->addr.getValue(addrVal);
		uint8_t aliasLen = alias->alias.getValue(aliasVal);
		switch(s.value())
		{
			case 1: //в алиасе отличается typeOfNumber
				{
					alias->alias.setTypeOfNumber(alias->alias.getTypeOfNumber() + 1);
				}
				break;
			case 2: //в алиасе отличается numberingPlan
				{
					alias->alias.setNumberingPlan(alias->alias.getNumberingPlan() + 1);
				}
				break;
			case 3: //в алиасе отличается addressValue
				{
					memcpy(tmp, aliasVal, aliasLen);
					tmp[rand0(aliasLen - 1)] = '+';
					alias->alias.setValue(aliasLen, tmp);
				}
				break;
			case 4: //в алиасе '?' меньше, чем нужно
				if (aliasLen > 1)
				{
					int len1 = rand1(min((int) addrLen, aliasLen - 1));
					int len2 = rand1(aliasLen - len1);
					__require__(len1 > 0 && len2 > 0);
					__require__(len1 <= addrLen);
					__require__(len1 + len2 <= aliasLen);
					//alias -> addr
					memcpy(tmp, addrVal, addrLen - len1);
					memset(tmp + addrLen - len1, '?', len1);
					alias->addr.setValue(addrLen, tmp);
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - len1 - len2);
					memset(tmp + aliasLen - len1 - len2, '?', len1);
					alias->alias.setValue(aliasLen - len2, tmp);
				}
				break;
			case 5: //в алиасе '?' больше, чем нужно
				if (aliasLen < MAX_ADDRESS_VALUE_LENGTH && addrLen > 1)
				{
					int len = min(MAX_ADDRESS_VALUE_LENGTH - aliasLen, addrLen - 1);
					int len2 = rand1(len);
					int len1 = rand1(min(addrLen - len2, (int) aliasLen));
					__require__(len1 > 0 && len2 > 0);
					__require__(len1 + len2 <= addrLen);
					__require__(len1 <= aliasLen);
					__require__(aliasLen + len2 <= MAX_ADDRESS_VALUE_LENGTH);
					//alias -> addr
					memcpy(tmp, addrVal, addrLen - len1 - len2);
					memset(tmp + addrLen - len1 - len2, '?', len1 + len2);
					alias->addr.setValue(addrLen, tmp);
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - len1);
					memset(tmp + aliasLen - len1, '?', len1 + len2);
					alias->alias.setValue(aliasLen + len2, tmp);
				}
				break;
			default:
				throw s;
		}
		debugAlias("addCorrectAliasNotMatchAlias", s.value(), alias);
		aliasMan->addAlias(*alias);
		aliasReg->putAlias(*alias);
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::addCorrectAliasException(
	AliasInfo* alias, int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS_EXCEPTION, s.getChoice());
	getLog().debugStream() << "[" << thr_self() <<
		"]\taddCorrectAliasException(" << s.value() << ")";
	try
	{
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = alias->addr.getValue(addrVal);
		uint8_t aliasLen = alias->alias.getValue(aliasVal);
		switch(s.value())
		{
			case 1: //адрес = '*', алиас равен оригиналу + '*' в конце
				if (aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					//alias -> addr (результат = адрес нулевой длины)
					alias->addr.setValue(1, "*");
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen);
					tmp[aliasLen] = '*';
					alias->alias.setValue(aliasLen + 1, tmp);
				}
				break;
			case 2: //адрес равен оригиналу + '*' в конце, алиас = '*'
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					//alias -> addr
					memcpy(tmp, addrVal, addrLen);
					tmp[addrLen] = '*';
					alias->addr.setValue(addrLen + 1, tmp);
					//addr -> alias (результат = алиас нулевой длины)
					alias->alias.setValue(1, "*");
				}
				break;
			case 3: //преобразование алиас -> адрес вызывает переполнение адреса
				if (aliasLen + addrLen > MAX_ADDRESS_VALUE_LENGTH + 1)
				{
					int adLen = rand1(aliasLen + addrLen - MAX_ADDRESS_VALUE_LENGTH - 1);
					int alLen = rand2(aliasLen,
						MAX_ADDRESS_VALUE_LENGTH - addrLen + adLen + 1);
					__require__(0 <= adLen && adLen <= addrLen);
					__require__(0 <= alLen && alLen <= aliasLen);
					__require__(addrLen - adLen < MAX_ADDRESS_VALUE_LENGTH);
					__require__(aliasLen - alLen < MAX_ADDRESS_VALUE_LENGTH);
					//alias -> addr
					memcpy(tmp, addrVal, addrLen - adLen);
					tmp[addrLen - adLen] = '*';
					alias->addr.setValue(addrLen - adLen + 1, tmp);
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - alLen);
					tmp[aliasLen - alLen] = '*';
					alias->alias.setValue(aliasLen - alLen + 1, tmp);
				}
				break;
			case 4: //преобразование адрес -> алиас вызывает переполнение алиаса
				if (aliasLen + addrLen > MAX_ADDRESS_VALUE_LENGTH + 1)
				{
					int alLen = rand1(aliasLen + addrLen - MAX_ADDRESS_VALUE_LENGTH - 1);
					int adLen = rand2(MAX_ADDRESS_VALUE_LENGTH - aliasLen + alLen + 1, addrLen);
					__require__(0 <= adLen && adLen <= addrLen);
					__require__(0 <= alLen && alLen <= aliasLen);
					__require__(addrLen - adLen < MAX_ADDRESS_VALUE_LENGTH);
					__require__(aliasLen - alLen < MAX_ADDRESS_VALUE_LENGTH);
					//alias -> addr
					memcpy(tmp, addrVal, addrLen - adLen);
					tmp[addrLen - adLen] = '*';
					alias->addr.setValue(addrLen - adLen + 1, tmp);
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - alLen);
					tmp[aliasLen - alLen] = '*';
					alias->alias.setValue(aliasLen - alLen + 1, tmp);
				}
				break;
			default:
				throw s;
		}
		debugAlias("addCorrectAliasException", s.value(), alias);
		aliasMan->addAlias(*alias);
		aliasReg->putAlias(*alias);
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}
			
TCResult* AliasManagerTestCases::addIncorrectAlias()
{
	//При добавлении дублированных алиасов (по addr или alias)
	//выборка будет происходить случайным образом
	TCResult* res = new TCResult(TC_ADD_INCORRECT_ALIAS);
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
		aliasReg->clear();
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

void AliasManagerTestCases::printFindResult(const char* name,
	const Address& param, const AliasRegistry::AliasList& list)
{
	ostringstream os;
	os << name << ": param = " << param;
	os << ", result = (";
	for (int i = 0; i < list.size(); i++)
	{
		if (i)
		{
			os << ",";
		}
		os << *list[i];
	}
	os << ")";
	getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
}

void AliasManagerTestCases::printFindResult(const char* tc,
	const Address& param, bool found, const Address& result)
{
	ostringstream os;
	os << tc << ": param = " << param;
	os << ", found = " << (found ? "true" : "false");
	if (found)
	{
		os << ", result = " << result;
	}
	getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
}

TCResult* AliasManagerTestCases::findAliasByAddress(const Address& addr)
{
	TCResult* res = new TCResult(TC_FIND_ALIAS_BY_ADDRESS);
	const AliasRegistry::AliasList list = aliasReg->findAliasByAddress(addr);
	printFindResult("AliasRegistry::findAliasByAddress()", addr, list);
	try
	{
		Address alias;
		bool found = aliasMan->AddressToAlias(addr, alias);
		printFindResult("AliasManager::AddressToAlias()", addr, found, alias);
		if (!found && list.size())
		{
			res->addFailure(101);
		}
		else if (found && !list.size())
		{
			res->addFailure(102);
		}
		else if (found && list.size())
		{
			found = false;
			for (int i = 0; i < list.size(); i++)
			{
				Address tmp;
				if (list[i]->addressToAlias(addr, tmp))
				{
					found |= SmsUtil::compareAddresses(alias, tmp);
				}
			}
			if (!found)
			{
				res->addFailure(103);
			}
		}
	}
	catch(...)
	{
		bool exception = false;
		for (int i = 0; i < list.size(); i++)
		{
			Address tmp;
			exception |= !(list[i]->addressToAlias(addr, tmp));
		}
		if (!exception)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::findAddressByAlias(const Address& alias)
{
	TCResult* res = new TCResult(TC_FIND_ADDRESS_BY_ALIAS);
	const AliasRegistry::AliasList list = aliasReg->findAddressByAlias(alias);
	printFindResult("AliasRegistry::findAddressByAlias()", alias, list);
	try
	{
		Address addr;
		bool found = aliasMan->AliasToAddress(alias, addr);
		printFindResult("AliasManager::AliasToAddress()", alias, found, addr);
		if (!found && list.size())
		{
			res->addFailure(101);
		}
		else if (found && !list.size())
		{
			res->addFailure(102);
		}
		else if (found && list.size())
		{
			found = false;
			for (int i = 0; i < list.size(); i++)
			{
				Address tmp;
				if (list[i]->aliasToAddress(alias, tmp))
				{
					found |= SmsUtil::compareAddresses(addr, tmp);
				}
			}
			if (!found)
			{
				res->addFailure(103);
			}
		}
	}
	catch(...)
	{
		bool exception = false;
		for (int i = 0; i < list.size(); i++)
		{
			Address tmp;
			exception |= !(list[i]->aliasToAddress(alias, tmp));
		}
		if (!exception)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::iterateAliases()
{
	TCResult* res = new TCResult(TC_ITERATE_ALIASES);
	res->addFailure(100);
	debug(res);
	return res;
}

}
}
}

