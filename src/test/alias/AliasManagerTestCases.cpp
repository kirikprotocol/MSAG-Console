#include "AliasManagerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "util/debug.h"
#include <algorithm>

namespace smsc {
namespace test {
namespace alias {

using smsc::sms::AddressValue;
using smsc::util::Logger;
using smsc::test::sms::SmsUtil;
using smsc::test::sms::operator<<;
using smsc::test::sms::operator!=;
using smsc::test::core::operator<<;
using namespace std;
using namespace smsc::sms; //constants
using namespace smsc::test::util;

AliasManagerTestCases::AliasManagerTestCases(AliasManager* _aliasMan,
	AliasRegistry* _aliasReg, CheckList* _chkList)
	: aliasMan(_aliasMan), aliasReg(_aliasReg), chkList(_chkList)
{
	//__require__(aliasMan);
	//__require__(aliasReg);
	//__require__(chkList);
}

Category& AliasManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("AliasManagerTestCases");
	return log;
}

void AliasManagerTestCases::debugAlias(const char* tc, int val,
	const AliasInfo* alias)
{
	ostringstream os;
	os << tc << "(" << val << "): " << *alias;
	__trace2__("%s", os.str().c_str());
}

void AliasManagerTestCases::commit()
{
	__require__(aliasMan);
	try
	{
		aliasMan->commit();
		__trace__("Commit successful");
	}
	catch(...)
	{
		error();
		__trace__("Commit failed");
	}
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

/*
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
*/

void AliasManagerTestCases::setupAliasHide(AliasInfo* alias)
{
	AddressValue addrVal;
	alias->addr.getValue(addrVal);
	const char* tmp = strchr(addrVal, '?');
	//обязательно hide = false для алиасов со знаками подстановки
	alias->hide = (tmp ? false : rand0(3));
}

bool AliasManagerTestCases::addAlias(const char* tc, int num, const AliasInfo* alias)
{
	if (aliasReg)
	{
		//дублированные алиасы не сохраняются
		if (aliasReg->putAlias(*alias))
		{
			debugAlias(tc, num, alias);
			if (aliasMan)
			{
				aliasMan->addAlias(*alias);
			}
			return true;
		}
	}
	else if (aliasMan)
	{
		debugAlias(tc, num, alias);
		aliasMan->addAlias(*alias);
		return true;
	}
	return false;
}

void AliasManagerTestCases::addCorrectAliasMatch(AliasInfo* alias, int num)
{
	__require__(alias);
	TCSelector s(num, 3);
	__decl_tc__;
	__tc__("addCorrectAlias.allMatchNoSubstSymbols"); //по умолчанию, там где не выполняются проверки
	try
	{
		uint8_t addrLen = alias->addr.getLenght();
		uint8_t aliasLen = alias->alias.getLenght();
		int minLen = min(addrLen, aliasLen);
		int maxLen = max(addrLen, aliasLen);
		switch(s.value())
		{
			case 1: //адрес и алиас без знаков подстановки
				//__tc__("addCorrectAlias.allMatchNoSubstSymbols");
				break;
			case 2: //адрес и алиас с одним или несколькими '?' в конце
				if (minLen > 1)
				{
					__tc__("addCorrectAlias.allMatchWithQuestionMarks");
					setupRandomAliasMatchWithQuestionMarks(alias, rand1(minLen - 1));
				}
				break;
			case 3: //адрес или алиас целиком из '?'
				__tc__("addCorrectAlias.allMatchEntirelyQuestionMarks");
				setupRandomAliasMatchWithQuestionMarks(alias, minLen);
				break;
			/*
			case 4: //адрес со '*' в конце, алиас со '*' в конце
				if (addrLen > 1 && aliasLen > 1)
				{
					__tc__("...");
					int adLen = rand1(addrLen - 1);
					int alLen = rand1(aliasLen - 1);
					setupRandomAliasMatchWithAsterisk(alias, adLen, alLen);
				}
				break;
			case 5: //адрес со '*' в конце, алиас = '*'
				if (addrLen > 1)
				{
					__tc__("...");
					int adLen = rand1(addrLen - 1);
					//int alLen = aliasLen;
					setupRandomAliasMatchWithAsterisk(alias, adLen, aliasLen);
				}
				break;
			case 6: //адрес со '*' в конце, алиас равен оригиналу + '*' в конце
				if (addrLen > 1 && aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					__tc__("...");
					int adLen = rand1(addrLen - 1);
					//int alLen = 0;
					setupRandomAliasMatchWithAsterisk(alias, adLen, 0);
				}
				break;
			case 7: //адрес = '*', алиас со '*' в конце
				if (aliasLen > 1)
				{
					__tc__("...");
					//int adLen = addrLen;
					int alLen = rand1(aliasLen - 1);
					setupRandomAliasMatchWithAsterisk(alias, addrLen, alLen);
				}
				break;
			case 8: //адрес = '*', алиас = '*'
				{
					__tc__("...");
					//int adLen = addrLen;
					//int alLen = aliasLen;
					setupRandomAliasMatchWithAsterisk(alias, addrLen, aliasLen);
				}
				break;
			case 9: //адрес равен оригиналу + '*' в конце, алиас со '*' в конце
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH && aliasLen > 1)
				{
					__tc__("...");
					//int adLen = 0;
					int alLen = rand1(aliasLen - 1);
					setupRandomAliasMatchWithAsterisk(alias, 0, alLen);
				}
				break;
			case 10: //адрес равен оригиналу + '*' в конце, алиас равен оригиналу + '*' в конце
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH && aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					__tc__("...");
					//int adLen = 0;
					//int alLen = 0;
					setupRandomAliasMatchWithAsterisk(alias, 0, 0);
				}
				break;
			*/
			default:
				throw s;
		}
		setupAliasHide(alias);
		if (addAlias("addCorrectAliasMatch", s.value(), alias))
		{
			__tc_ok__;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void AliasManagerTestCases::addCorrectAliasNotMatchAddress(
	AliasInfo* alias, int num)
{
	__require__(alias);
	TCSelector s(num, 5);
	__decl_tc__;
	__tc__("addCorrectAlias.allMatchNoSubstSymbols"); //по умолчанию, там где не выполняются проверки
	try
	{
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = alias->addr.getValue(addrVal);
		uint8_t aliasLen = alias->alias.getValue(aliasVal);
		int minLen = min(addrLen, aliasLen);
		switch(s.value())
		{
			case 1: //в адресе отличается typeOfNumber
				{
					__tc__("addCorrectAlias.addrNotMatch.diffType");
					int len = rand0(minLen);
					setupRandomAliasMatchWithQuestionMarks(alias, len);
					SmsUtil::setupRandomCorrectAddressTon(&alias->addr);
				}
				break;
			case 2: //в адресе отличается numberingPlan
				{
					__tc__("addCorrectAlias.addrNotMatch.diffPlan");
					int len = rand0(minLen);
					setupRandomAliasMatchWithQuestionMarks(alias, len);
					SmsUtil::setupRandomCorrectAddressNpi(&alias->addr);
				}
				break;
			case 3: //в адресе отличается addressValue
				if (minLen > 2)
				{
					__tc__("addCorrectAlias.allMatchWithQuestionMarks");
					int len = rand0(minLen - 2);
					setupRandomAliasMatchWithQuestionMarks(alias, len);
					alias->addr.getValue(tmp);
					if (next_permutation(tmp, tmp + addrLen - len))
					{
						__tc__("addCorrectAlias.addrNotMatch.diffValue");
						alias->addr.setValue(addrLen, tmp);
					}
				}
				break;
			case 4: //в адресе '?' меньше, чем нужно
				if (addrLen > 1)
				{
					__tc__("addCorrectAlias.addrNotMatch.diffValueLength");
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
					__tc__("addCorrectAlias.addrNotMatch.diffValueLength");
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
		setupAliasHide(alias);
		if (addAlias("addCorrectAliasNotMatchAddress", s.value(), alias))
		{
			__tc_ok__;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void AliasManagerTestCases::addCorrectAliasNotMatchAlias(
	AliasInfo* alias, int num)
{
	__require__(alias);
	TCSelector s(num, 5);
	__decl_tc__;
	__tc__("addCorrectAlias.allMatchNoSubstSymbols"); //по умолчанию, там где не выполняются проверки
	try
	{
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = alias->addr.getValue(addrVal);
		uint8_t aliasLen = alias->alias.getValue(aliasVal);
		int minLen = min(addrLen, aliasLen);
		switch(s.value())
		{
			case 1: //в алиасе отличается typeOfNumber
				__tc__("addCorrectAlias.aliasNotMatch.diffType");
				setupRandomAliasMatchWithQuestionMarks(alias, rand0(minLen));
				SmsUtil::setupRandomCorrectAddressTon(&alias->alias);
				break;
			case 2: //в алиасе отличается numberingPlan
				__tc__("addCorrectAlias.aliasNotMatch.diffPlan");
				setupRandomAliasMatchWithQuestionMarks(alias, rand0(minLen));
				SmsUtil::setupRandomCorrectAddressNpi(&alias->alias);
				break;
			case 3: //в алиасе отличается addressValue
				if (minLen > 2)
				{
					__tc__("addCorrectAlias.allMatchWithQuestionMarks");
					int len = rand0(minLen - 2);
					setupRandomAliasMatchWithQuestionMarks(alias, len);
					alias->alias.getValue(tmp);
					if (next_permutation(tmp, tmp + aliasLen - len))
					{
						__tc__("addCorrectAlias.aliasNotMatch.diffValue");
						alias->alias.setValue(aliasLen, tmp);
					}
				}
				break;
			case 4: //в алиасе '?' меньше, чем нужно
				if (aliasLen > 1)
				{
					__tc__("addCorrectAlias.aliasNotMatch.diffValueLength");
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
					__tc__("addCorrectAlias.aliasNotMatch.diffValueLength");
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
		setupAliasHide(alias);
		if (addAlias("addCorrectAliasNotMatchAlias", s.value(), alias))
		{
			__tc_ok__;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

/*
void AliasManagerTestCases::addCorrectAliasException(
	AliasInfo* alias, int num)
{
	TCSelector s(num, 4);
	void res = new TCResult(TC_ADD_CORRECT_ALIAS_EXCEPTION, s.getChoice());
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
		if (aliasReg->putAlias(*alias))
		{
			debugAlias("addCorrectAliasException", s.value(), alias);
			aliasMan->addAlias(*alias);
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
			
void AliasManagerTestCases::addIncorrectAlias()
{
	//При добавлении дублированных алиасов (по addr или alias)
	//выборка будет происходить случайным образом
	void res = new TCResult(TC_ADD_INCORRECT_ALIAS);
	debug(res);
	return res;
}
*/

void AliasManagerTestCases::deleteAliases()
{
	__decl_tc__;
	__tc__("deleteAliases");
	try
	{
		AliasInfo alias;
		if (aliasMan)
		{
			aliasMan->clean();
		}
		if (aliasReg)
		{
			aliasReg->clear();
		}
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void AliasManagerTestCases::printFindResult(const char* name,
	const Address& param, const Address& result, const AliasInfo* aliasInfo)
{
	ostringstream os;
	os << name << ": param = " << param;
	os << ", result = " << result << ", aliasInfo = ";
	if (aliasInfo)
	{
		os << "{" << *aliasInfo << "}";
	}
	else
	{
		os << "NULL";
	}
	__trace2__("%s", os.str().c_str());
}

void AliasManagerTestCases::printFindResult(const char* tc,
	const Address& param, const Address& result, bool found)
{
	ostringstream os;
	os << tc << ": param = " << param;
	os << ", found = " << (found ? "true" : "false");
	if (found)
	{
		os << ", result = " << result;
	}
	__trace2__("%s", os.str().c_str());
}

void AliasManagerTestCases::findAliasByAddress(const Address& addr)
{
	__require__(aliasReg && aliasMan);
	__decl_tc__;
	__tc__("findAliasByAddress");
	const AliasInfo* aliasInfo;
	const Address alias2 = aliasReg->findAliasByAddress(addr, &aliasInfo);
	printFindResult("AliasRegistry::findAliasByAddress()", addr, alias2, aliasInfo);
	try
	{
		Address alias;
		bool found = aliasMan->AddressToAlias(addr, alias);
		printFindResult("AliasManager::AddressToAlias()", addr, alias, found);
		if (!found && aliasInfo)
		{
			__tc_fail__(101);
		}
		else if (found && !aliasInfo)
		{
			__tc_fail__(103);
		}
		else if (found && aliasInfo && alias != alias2)
		{
			__tc_fail__(104);
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
	}
}

void AliasManagerTestCases::findAddressByAlias(const Address& alias)
{
	__require__(aliasReg && aliasMan);
	__decl_tc__;
	__tc__("findAddressByAlias");
	const AliasInfo* aliasInfo;
	Address addr2 = aliasReg->findAddressByAlias(alias, &aliasInfo);
	printFindResult("AliasRegistry::findAddressByAlias()", alias, addr2, aliasInfo);
	try
	{
		Address addr;
		bool found = aliasMan->AliasToAddress(alias, addr);
		printFindResult("AliasManager::AliasToAddress()", alias, addr, found);
		if (!found && aliasInfo)
		{
			__tc_fail__(101);
		}
		else if (found && !aliasInfo)
		{
			__tc_fail__(102);
		}
		else if (found && aliasInfo && addr != addr2)
		{
			__tc_fail__(103);
		}
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
	}
}

void AliasManagerTestCases::checkInverseTransformation(const Address& addr)
{
	__require__(aliasReg && aliasMan);
	__decl_tc__;
	__tc__("checkInverseTransformation");
	try
	{
		//должно выполняться правило: d(a(A)) = A
		Address alias = addr, addr2, alias2;
		//addr->alias->addr
		if (aliasMan->AddressToAlias(addr, alias2))
		{
			__trace__("AliasManagerTestCases::checkInverseTransformation(): addr->alias->addr");
			if (!aliasMan->AliasToAddress(alias2, addr2))
			{
				__tc_fail__(101);
			}
			else if (addr != addr2)
			{
				__tc_fail__(102);
			}
		}
		/*
		//alias->addr->alias
		if (aliasMan->AliasToAddress(alias, addr2))
		{
			__trace__("AliasManagerTestCases::checkInverseTransformation(): alias->addr->alias");
			if (!aliasMan->AddressToAlias(addr2, alias2))
			{
				__tc_fail__(103);
			}
			else if (!SmsUtil::compareAddresses(alias, alias2))
			{
				__tc_fail__(104);
			}
		}
		*/
		__tc_ok_cond__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void AliasManagerTestCases::iterateAliases()
{
	__decl_tc__;
	__tc__("iterateAliases");
	__tc_fail__(100);
}

}
}
}

