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
using namespace smsc::sms; //constants
using namespace smsc::test::util;

AliasManagerTestCases::AliasManagerTestCases(AliasManager* manager)
{
	aliasMan = manager;
}

Category& AliasManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmeManagerTestCases");
	return log;
}

inline AliasInfo* AliasManagerTestCases::newAliasInfo(TestAliasData* data)
{
	AliasInfo* alias = new AliasInfo();
	data->alias = alias;
	//addr -> alias
	alias->addr = data->origAddr;
	data->destAlias = data->origAlias;
	data->addrMatch = 100;
	//alias -> addr
	alias->alias = data->origAlias;
	data->destAddr = data->origAddr;
	data->aliasMatch = 100;
	return alias;
}

void AliasManagerTestCases::setupRandomAliasMatchWithQuestionMarks(
	TestAliasData* data, int len)
{
	AddressValue addrVal, aliasVal, tmp;
	uint8_t addrLen = data->origAddr.getValue(addrVal);
	uint8_t aliasLen = data->origAlias.getValue(aliasVal);
	__require__(len >= 0 && len <= addrLen && len <= aliasLen);
	//alias -> addr
	memcpy(tmp, addrVal, addrLen - len);
	memset(tmp + addrLen - len, '?', len);
	data->alias->addr.setValue(addrLen, tmp);
	memcpy(tmp + addrLen - len, aliasVal + aliasLen - len, len);
	data->destAddr.setValue(addrLen, tmp);
	data->aliasMatch = 100 - len;
	//addr -> alias
	memcpy(tmp, aliasVal, aliasLen - len);
	memset(tmp + aliasLen - len, '?', len);
	data->alias->alias.setValue(aliasLen, aliasVal);
	memcpy(tmp + aliasLen - len, addrVal + addrLen - len, len);
	data->destAlias.setValue(aliasLen, tmp);
	data->addrMatch = 100 - len;
}

void AliasManagerTestCases::setupRandomAliasMatchWithAsterisk(
	TestAliasData* data, int adLen, int alLen)
{
	AddressValue addrVal, aliasVal, tmp;
	uint8_t addrLen = data->origAddr.getValue(addrVal);
	uint8_t aliasLen = data->origAlias.getValue(aliasVal);
	__require__(adLen >= 0 && adLen <= addrLen);
	__require__(alLen >= 0 && alLen <= aliasLen);
	__require__(addrLen - adLen < MAX_ADDRESS_VALUE_LENGTH);
	__require__(aliasLen - alLen < MAX_ADDRESS_VALUE_LENGTH);
	__require__(addrLen - adLen + alLen <= MAX_ADDRESS_VALUE_LENGTH);
	__require__(aliasLen - alLen + adLen <= MAX_ADDRESS_VALUE_LENGTH);
	//alias -> addr
	memcpy(tmp, addrVal, addrLen - adLen);
	tmp[addrLen - adLen] = '*';
	data->alias->addr.setValue(addrLen - adLen + 1, tmp);
	memcpy(tmp + addrLen - adLen, aliasVal + aliasLen - alLen, alLen);
	data->destAddr.setValue(addrLen - adLen + alLen, tmp);
	data->aliasMatch = 100 - alLen - 0.5; //'*' менее приоритетна '?'
	//addr -> alias
	memcpy(tmp, aliasVal, aliasLen - alLen);
	tmp[aliasLen - alLen] = '*';
	data->alias->alias.setValue(aliasLen - alLen + 1, tmp);
	memcpy(tmp + aliasLen - alLen, addrVal + addrLen - adLen, adLen);
	data->destAlias.setValue(aliasLen - alLen + adLen, tmp);
	data->addrMatch = 100 - adLen - 0.5; //'*' менее приоритетна '?'
}

TCResult* AliasManagerTestCases::addCorrectAliasMatch(TestAliasData* data, int num)
{
	TCSelector s(num, 10);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS_MATCH, s.getChoice());
	getLog().debugStream() << "[" << thr_self() <<
		"]\taddCorrectAliasMatch(" << s.value() << ")";
	try
	{
		AliasInfo* alias = newAliasInfo(data);
		uint8_t addrLen = data->origAddr.getLenght();
		uint8_t aliasLen = data->origAlias.getLenght();
		int minLen = min(addrLen, aliasLen);
		int maxLen = max(addrLen, aliasLen);
		switch(s.value())
		{
			case 1: //адрес и алиас без знаков подстановки
				break;
			case 2: //адрес и алиас с одним или несколькими '?' в конце
				setupRandomAliasMatchWithQuestionMarks(data, rand1(minLen));
				break;
			case 3: //адрес или алиас целиком из '?'
				setupRandomAliasMatchWithQuestionMarks(data, minLen);
				break;
			case 4: //адрес со '*' в конце, алиас со '*' в конце
				if (addrLen + aliasLen < 2 * MAX_ADDRESS_VALUE_LENGTH)
				{
					int adLen = rand1(addrLen);
					int alLen = rand2(adLen - (MAX_ADDRESS_VALUE_LENGTH - aliasLen),
						adLen + (MAX_ADDRESS_VALUE_LENGTH - addrLen));
					setupRandomAliasMatchWithAsterisk(data, adLen, alLen);
				}
				break;
			case 5: //адрес со '*' в конце, алиас = '*'
				{
					int adLen = rand2(addrLen,
						max(1, addrLen + aliasLen - MAX_ADDRESS_VALUE_LENGTH));
					//int alLen = aliasLen;
					setupRandomAliasMatchWithAsterisk(data, adLen, aliasLen);
				}
				break;
			case 6: //адрес со '*' в конце, алиас равен оригиналу + '*' в конце
				if (aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					int adLen = rand1(min((int) addrLen,
						MAX_ADDRESS_VALUE_LENGTH - aliasLen));
					//int alLen = 0;
					setupRandomAliasMatchWithAsterisk(data, adLen, 0);
				}
				break;
			case 7: //адрес = '*', алиас со '*' в конце
				{
					//int adLen = addrLen;
					int alLen = rand2(aliasLen,
						max(1, addrLen + aliasLen - MAX_ADDRESS_VALUE_LENGTH));
					setupRandomAliasMatchWithAsterisk(data, addrLen, alLen);
				}
				break;
			case 8: //адрес = '*', алиас = '*'
				{
					//int adLen = addrLen;
					//int alLen = aliasLen;
					setupRandomAliasMatchWithAsterisk(data, addrLen, aliasLen);
				}
				break;
			case 9: //адрес равен оригиналу + '*' в конце, алиас со '*' в конце
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					//int adLen = 0;
					int alLen = rand1(min((int) aliasLen,
						MAX_ADDRESS_VALUE_LENGTH - addrLen));
					setupRandomAliasMatchWithAsterisk(data, 0, alLen);
				}
				break;
			case 10: //адрес равен оригиналу + '*' в конце, алиас равен оригиналу + '*' в конце
				if (aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					//int adLen = 0;
					//int alLen = 0;
					setupRandomAliasMatchWithAsterisk(data, 0, 0);
				}
				break;
			default:
				throw s;
		}
		getLog().debugStream() << "[" << thr_self() <<
			"]\taddCorrectAliasMatch(" << s.value() << "): " << *data;
		aliasMan->addAlias(*alias);
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
	TestAliasData* data, int num)
{
	TCSelector s(num, 5);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS_NOT_MATCH_ADDRESS, s.getChoice());
	getLog().debugStream() << "[" << thr_self() <<
		"]\taddCorrectAliasNotMatchAddress(" << s.value() << ")";
	try
	{
		AliasInfo* alias = newAliasInfo(data);
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = data->origAddr.getValue(addrVal);
		uint8_t aliasLen = data->origAlias.getValue(aliasVal);
		switch(s.value())
		{
			case 1: //в адресе отличается typeOfNumber
				{
					uint8_t ton = data->origAddr.getTypeOfNumber() + 1;
					//alias -> addr
					alias->addr.setTypeOfNumber(ton);
					data->destAddr.setTypeOfNumber(ton);
					data->aliasMatch = 100;
					//addr -> alias
					data->addrMatch = 0;
				}
				break;
			case 2: //в адресе отличается numberingPlan
				{
					//alias -> addr
					uint8_t npi = data->origAddr.getNumberingPlan() + 1;
					alias->addr.setNumberingPlan(npi);
					data->destAddr.setNumberingPlan(npi);
					data->aliasMatch = 100;
					//addr -> alias
					data->addrMatch = 0;
				}
				break;
			case 3: //в адресе отличается addressValue
				{
					//alias -> addr
					memcpy(tmp, addrVal, addrLen);
					tmp[rand0(addrLen - 1)] = '+';
					alias->addr.setValue(addrLen, tmp);
					data->destAddr.setValue(addrLen, tmp);
					data->aliasMatch = 100;
					//addr -> alias
					data->addrMatch = 0;
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
					memcpy(tmp + addrLen - len1 - len2, aliasVal + aliasLen - len1, len1);
					data->destAddr.setValue(addrLen - len2, tmp);
					data->aliasMatch = 100 - len1;
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - len1);
					memset(tmp + aliasLen - len1, '?', len1);
					alias->alias.setValue(aliasLen, tmp);
					data->addrMatch = 0;
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
					__require__(len1 + len2 <= aliasLen);
					__require__(addrLen + len2 <= MAX_ADDRESS_VALUE_LENGTH);
					//alias -> addr
					memcpy(tmp, addrVal, addrLen - len1);
					memset(tmp + addrLen - len1, '?', len1 + len2);
					alias->addr.setValue(addrLen + len2, tmp);
					memcpy(tmp + addrLen - len1,
						aliasVal + aliasLen - len1 - len2, len1 + len2);
					data->destAddr.setValue(addrLen + len2, tmp);
					data->aliasMatch = 100 - len1 - len2;
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - len1 - len2);
					memset(tmp + aliasLen - len1 - len2, '?', len1 + len2);
					alias->alias.setValue(aliasLen, tmp);
					data->addrMatch = 0;
				}
				break;
			default:
				throw s;
		}
		getLog().debugStream() << "[" << thr_self() <<
			"]\taddCorrectAliasNotMatchAddress(" << s.value() << "): " << *data;
		aliasMan->addAlias(*alias);
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
	TestAliasData* data, int num)
{
	TCSelector s(num, 5);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS_NOT_MATCH_ALIAS, s.getChoice());
	getLog().debugStream() << "[" << thr_self() <<
		"]\taddCorrectAliasNotMatchAlias(" << s.value() << ")";
	try
	{
		AliasInfo* alias = newAliasInfo(data);
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = data->origAddr.getValue(addrVal);
		uint8_t aliasLen = data->origAlias.getValue(aliasVal);
		switch(s.value())
		{
			case 1: //в алиасе отличается typeOfNumber
				{
					uint8_t ton = data->origAlias.getTypeOfNumber() + 1;
					//alias -> addr
					data->aliasMatch = 0;
					//addr -> alias
					alias->alias.setTypeOfNumber(ton);
					data->destAlias.setTypeOfNumber(ton);
					data->addrMatch = 100;
				}
				break;
			case 2: //в алиасе отличается numberingPlan
				{
					//alias -> addr
					data->aliasMatch = 0;
					//addr -> alias
					uint8_t npi = data->origAlias.getNumberingPlan() + 1;
					alias->alias.setNumberingPlan(npi);
					data->destAlias.setNumberingPlan(npi);
					data->addrMatch = 100;
				}
				break;
			case 3: //в алиасе отличается addressValue
				{
					//alias -> addr
					data->aliasMatch = 0;
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen);
					tmp[rand0(aliasLen - 1)] = '+';
					alias->alias.setValue(aliasLen, tmp);
					data->destAlias.setValue(aliasLen, tmp);
					data->addrMatch = 100;
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
					data->aliasMatch = 0;
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - len1 - len2);
					memset(tmp + aliasLen - len1 - len2, '?', len1);
					alias->alias.setValue(aliasLen - len2, tmp);
					memcpy(tmp + aliasLen - len1 - len2, addrVal + addrLen - len1, len1);
					data->destAlias.setValue(aliasLen - len2, tmp);
					data->addrMatch = 100 - len1;
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
					data->aliasMatch = 0;
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - len1);
					memset(tmp + aliasLen - len1, '?', len1 + len2);
					alias->alias.setValue(aliasLen + len2, tmp);
					memcpy(tmp + aliasLen - len1,
						addrVal + addrLen - len1 - len2, len1 + len2);
					data->destAlias.setValue(aliasLen + len2, tmp);
					data->addrMatch = 100 - len1 - len2;
				}
				break;
			default:
				throw s;
		}
		getLog().debugStream() << "[" << thr_self() <<
			"]\taddCorrectAliasNotMatchAlias(" << s.value() << "): " << *data;
		aliasMan->addAlias(*alias);
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
	TestAliasData* data, int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS_EXCEPTION, s.getChoice());
	getLog().debugStream() << "[" << thr_self() <<
		"]\taddCorrectAliasException(" << s.value() << ")";
	try
	{
		AliasInfo* alias = newAliasInfo(data);
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = data->origAddr.getValue(addrVal);
		uint8_t aliasLen = data->origAlias.getValue(aliasVal);
		switch(s.value())
		{
			case 1: //адрес = '*', алиас равен оригиналу + '*' в конце
				if (addrLen + aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					__require__(aliasLen < MAX_ADDRESS_VALUE_LENGTH);
					__require__(aliasLen + addrLen <= MAX_ADDRESS_VALUE_LENGTH);
					//alias -> addr (результат = адрес нулевой длины)
					data->alias->addr.setValue(1, "*");
					data->aliasMatch = 99.5; //'*' менее приоритетна '?'
					data->aliasToAddrException = true;
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen);
					tmp[aliasLen] = '*';
					data->alias->alias.setValue(aliasLen + 1, tmp);
					memcpy(tmp + aliasLen, addrVal, addrLen);
					data->destAlias.setValue(aliasLen + addrLen, tmp);
					data->addrMatch = 100 - addrLen - 0.5; //'*' менее приоритетна '?'

				}
				break;
			case 2: //адрес равен оригиналу + '*' в конце, алиас = '*'
				if (addrLen + aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					__require__(addrLen < MAX_ADDRESS_VALUE_LENGTH);
					__require__(addrLen + aliasLen <= MAX_ADDRESS_VALUE_LENGTH);
					//alias -> addr
					memcpy(tmp, addrVal, addrLen);
					tmp[addrLen] = '*';
					data->alias->addr.setValue(addrLen + 1, tmp);
					memcpy(tmp + addrLen, aliasVal, aliasLen);
					data->destAddr.setValue(addrLen + aliasLen, tmp);
					data->aliasMatch = 100 - aliasLen - 0.5; //'*' менее приоритетна '?'
					//addr -> alias (результат = алиас нулевой длины)
					data->alias->alias.setValue(1, "*");
					data->addrMatch = 99.5; //'*' менее приоритетна '?'
					data->addrToAliasException = true;
				}
				break;
			case 3: //преобразование алиас -> адрес вызывает переполнение адреса
				if (aliasLen + addrLen > MAX_ADDRESS_VALUE_LENGTH)
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
					data->aliasMatch = 100 - alLen - 0.5; //'*' менее приоритетна '?'
					data->aliasToAddrException = true;
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - alLen);
					tmp[aliasLen - alLen] = '*';
					alias->alias.setValue(aliasLen - alLen + 1, tmp);
					memcpy(tmp + aliasLen - alLen, addrVal + addrLen - adLen, adLen);
					data->destAlias.setValue(aliasLen - alLen + adLen, tmp);
					data->addrMatch = 100 - adLen - 0.5; //'*' менее приоритетна '?'
				}
				break;
			case 4: //преобразование адрес -> алиас вызывает переполнение алиаса
				if (aliasLen + addrLen > MAX_ADDRESS_VALUE_LENGTH)
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
					memcpy(tmp + addrLen - adLen, aliasVal + aliasLen - alLen, alLen);
					data->destAddr.setValue(addrLen - adLen + alLen, tmp);
					data->aliasMatch = 100 - alLen - 0.5; //'*' менее приоритетна '?'
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen - alLen);
					tmp[aliasLen - alLen] = '*';
					alias->alias.setValue(aliasLen - alLen + 1, tmp);
					data->addrMatch = 100 - adLen - 0.5; //'*' менее приоритетна '?'
					data->addrToAliasException = true;
				}
				break;
			default:
				throw s;
		}
		getLog().debugStream() << "[" << thr_self() <<
			"]\taddCorrectAliasException(" << s.value() << "): " << *data;
		aliasMan->addAlias(*alias);
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
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::findAliasByAddress(
	const AliasRegistry& aliasReg, const Address& addr)
{
	TCResult* res = new TCResult(TC_FIND_ALIAS_BY_ADDRESS);
	{
		ostringstream os;
		os << "findAliasByAddress(): addr = " << addr;
		getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
	}
	const AliasRegistry::AliasList data = aliasReg.findAliasByAddress(addr);
	try
	{
		Address alias;
		bool found = aliasMan->AddressToAlias(addr, alias);
		if (!found && data.size())
		{
			res->addFailure(101);
		}
		else if (found && !data.size())
		{
			res->addFailure(102);
		}
		else if (found && data.size())
		{
			found = false;
			for (int i = 0; i < data.size(); i++)
			{
				if (!data[i]->addrToAliasException)
				{
					found |= SmsUtil::compareAddresses(alias, data[i]->destAlias);
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
		bool found = false;
		for (int i = 0; i < data.size(); i++)
		{
			found |= data[i]->addrToAliasException;
		}
		if (!found)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::findAddressByAlias(
	const AliasRegistry& aliasReg, const Address& alias)
{
	TCResult* res = new TCResult(TC_FIND_ADDRESS_BY_ALIAS);
	{
		ostringstream os;
		os << "findAddressByAlias(): alias = " << alias;
		getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
	}
	const AliasRegistry::AliasList data = aliasReg.findAddressByAlias(alias);
	try
	{
		Address addr;
		bool found = aliasMan->AliasToAddress(alias, addr);
		if (!found && data.size())
		{
			res->addFailure(101);
		}
		else if (found && !data.size())
		{
			res->addFailure(102);
		}
		else if (found && data.size())
		{
			found = false;
			for (int i = 0; i < data.size(); i++)
			{
				if (!data[i]->aliasToAddrException)
				{
					found |= SmsUtil::compareAddresses(addr, data[i]->destAddr);
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
		bool found = false;
		for (int i = 0; i < data.size(); i++)
		{
			found |= data[i]->aliasToAddrException;
		}
		if (!found)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::iterateAliases(const AliasRegistry& aliasReg)
{
	TCResult* res = new TCResult(TC_ITERATE_ALIASES);
	res->addFailure(100);
	debug(res);
	return res;
}

}
}
}

