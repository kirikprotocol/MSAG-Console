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
	//__require__(aliasMan);
	//__require__(aliasReg);
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
	__trace2__("%s", os.str().c_str());
}

void AliasManagerTestCases::commit()
{
	__require__(aliasMan);
	try
	{
		aliasMan->commit();
		getLog().debug("[%d]\tCommit successful");
	}
	catch(...)
	{
		error();
		getLog().error("[%d]\tCommit failed");
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

void AliasManagerTestCases::addAlias(const char* tc, int num, const AliasInfo* alias)
{
	if (aliasReg)
	{
		if (aliasReg->putAlias(*alias))
		{
			debugAlias(tc, num, alias);
			if (aliasMan)
			{
				aliasMan->addAlias(*alias);
			}
		}
	}
	else if (aliasMan)
	{
		debugAlias(tc, num, alias);
		aliasMan->addAlias(*alias);
	}
}

TCResult* AliasManagerTestCases::addCorrectAliasMatch(AliasInfo* alias, int num)
{
	TCSelector s(num, 3);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ALIAS_MATCH, s.getChoice());
	try
	{
		uint8_t addrLen = alias->addr.getLenght();
		uint8_t aliasLen = alias->alias.getLenght();
		int minLen = min(addrLen, aliasLen);
		int maxLen = max(addrLen, aliasLen);
		switch(s.value())
		{
			case 1: //����� � ����� ��� ������ �����������
				break;
			case 2: //����� � ����� � ����� ��� ����������� '?' � �����
				if (minLen > 1)
				{
					setupRandomAliasMatchWithQuestionMarks(alias, rand1(minLen - 1));
				}
				break;
			case 3: //����� ��� ����� ������� �� '?'
				setupRandomAliasMatchWithQuestionMarks(alias, minLen);
				break;
			/*
			case 4: //����� �� '*' � �����, ����� �� '*' � �����
				if (addrLen > 1 && aliasLen > 1)
				{
					int adLen = rand1(addrLen - 1);
					int alLen = rand1(aliasLen - 1);
					setupRandomAliasMatchWithAsterisk(alias, adLen, alLen);
				}
				break;
			case 5: //����� �� '*' � �����, ����� = '*'
				if (addrLen > 1)
				{
					int adLen = rand1(addrLen - 1);
					//int alLen = aliasLen;
					setupRandomAliasMatchWithAsterisk(alias, adLen, aliasLen);
				}
				break;
			case 6: //����� �� '*' � �����, ����� ����� ��������� + '*' � �����
				if (addrLen > 1 && aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					int adLen = rand1(addrLen - 1);
					//int alLen = 0;
					setupRandomAliasMatchWithAsterisk(alias, adLen, 0);
				}
				break;
			case 7: //����� = '*', ����� �� '*' � �����
				if (aliasLen > 1)
				{
					//int adLen = addrLen;
					int alLen = rand1(aliasLen - 1);
					setupRandomAliasMatchWithAsterisk(alias, addrLen, alLen);
				}
				break;
			case 8: //����� = '*', ����� = '*'
				{
					//int adLen = addrLen;
					//int alLen = aliasLen;
					setupRandomAliasMatchWithAsterisk(alias, addrLen, aliasLen);
				}
				break;
			case 9: //����� ����� ��������� + '*' � �����, ����� �� '*' � �����
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH && aliasLen > 1)
				{
					//int adLen = 0;
					int alLen = rand1(aliasLen - 1);
					setupRandomAliasMatchWithAsterisk(alias, 0, alLen);
				}
				break;
			case 10: //����� ����� ��������� + '*' � �����, ����� ����� ��������� + '*' � �����
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH && aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					//int adLen = 0;
					//int alLen = 0;
					setupRandomAliasMatchWithAsterisk(alias, 0, 0);
				}
				break;
			*/
			default:
				throw s;
		}
		addAlias("addCorrectAliasMatch", s.value(), alias);
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
	try
	{
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = alias->addr.getValue(addrVal);
		uint8_t aliasLen = alias->alias.getValue(aliasVal);
		switch(s.value())
		{
			case 1: //� ������ ���������� typeOfNumber
				{
					alias->addr.setTypeOfNumber(alias->addr.getTypeOfNumber() + 1);
				}
				break;
			case 2: //� ������ ���������� numberingPlan
				{
					alias->addr.setNumberingPlan(alias->addr.getNumberingPlan() + 1);
				}
				break;
			case 3: //� ������ ���������� addressValue
				{
					memcpy(tmp, addrVal, addrLen);
					tmp[rand0(addrLen - 1)] = '@';
					alias->addr.setValue(addrLen, tmp);
				}
				break;
			case 4: //� ������ '?' ������, ��� �����
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
			case 5: //� ������ '?' ������, ��� �����
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
		addAlias("addCorrectAliasNotMatchAddress", s.value(), alias);
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
	try
	{
		AddressValue addrVal, aliasVal, tmp;
		uint8_t addrLen = alias->addr.getValue(addrVal);
		uint8_t aliasLen = alias->alias.getValue(aliasVal);
		switch(s.value())
		{
			case 1: //� ������ ���������� typeOfNumber
				{
					alias->alias.setTypeOfNumber(alias->alias.getTypeOfNumber() + 1);
				}
				break;
			case 2: //� ������ ���������� numberingPlan
				{
					alias->alias.setNumberingPlan(alias->alias.getNumberingPlan() + 1);
				}
				break;
			case 3: //� ������ ���������� addressValue
				{
					memcpy(tmp, aliasVal, aliasLen);
					tmp[rand0(aliasLen - 1)] = '@';
					alias->alias.setValue(aliasLen, tmp);
				}
				break;
			case 4: //� ������ '?' ������, ��� �����
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
			case 5: //� ������ '?' ������, ��� �����
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
		addAlias("addCorrectAliasNotMatchAlias", s.value(), alias);
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

/*
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
			case 1: //����� = '*', ����� ����� ��������� + '*' � �����
				if (aliasLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					//alias -> addr (��������� = ����� ������� �����)
					alias->addr.setValue(1, "*");
					//addr -> alias
					memcpy(tmp, aliasVal, aliasLen);
					tmp[aliasLen] = '*';
					alias->alias.setValue(aliasLen + 1, tmp);
				}
				break;
			case 2: //����� ����� ��������� + '*' � �����, ����� = '*'
				if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
				{
					//alias -> addr
					memcpy(tmp, addrVal, addrLen);
					tmp[addrLen] = '*';
					alias->addr.setValue(addrLen + 1, tmp);
					//addr -> alias (��������� = ����� ������� �����)
					alias->alias.setValue(1, "*");
				}
				break;
			case 3: //�������������� ����� -> ����� �������� ������������ ������
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
			case 4: //�������������� ����� -> ����� �������� ������������ ������
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
			
TCResult* AliasManagerTestCases::addIncorrectAlias()
{
	//��� ���������� ������������� ������� (�� addr ��� alias)
	//������� ����� ����������� ��������� �������
	TCResult* res = new TCResult(TC_ADD_INCORRECT_ALIAS);
	debug(res);
	return res;
}
*/

TCResult* AliasManagerTestCases::deleteAliases()
{
	TCResult* res = new TCResult(TC_DELETE_ALIASES);
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
	const Address& param, const AliasHolder* aliasHolder)
{
	ostringstream os;
	os << name << ": param = " << param;
	os << ", result = ";
	if (aliasHolder)
	{
		os << aliasHolder->aliasInfo;
	}
	else
	{
		os << "NULL";
	}
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
	__require__(aliasReg && aliasMan);
	TCResult* res = new TCResult(TC_FIND_ALIAS_BY_ADDRESS);
	const AliasHolder* aliasHolder = aliasReg->findAliasByAddress(addr);
	printFindResult("AliasRegistry::findAliasByAddress()", addr, aliasHolder);
	try
	{
		Address alias;
		bool found = aliasMan->AddressToAlias(addr, alias);
		printFindResult("AliasManager::AddressToAlias()", addr, found, alias);
		if (!found && aliasHolder)
		{
			res->addFailure(101);
		}
		else if (found && !aliasHolder)
		{
			res->addFailure(102);
		}
		else if (found && aliasHolder)
		{
			Address tmp;
			found = (aliasHolder->addressToAlias(addr, tmp) &&
				SmsUtil::compareAddresses(alias, tmp));
			if (!found)
			{
				res->addFailure(103);
			}
		}
	}
	catch(...)
	{
		Address tmp;
		if (aliasHolder->addressToAlias(addr, tmp))
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
	__require__(aliasReg && aliasMan);
	TCResult* res = new TCResult(TC_FIND_ADDRESS_BY_ALIAS);
	const AliasHolder* aliasHolder = aliasReg->findAddressByAlias(alias);
	printFindResult("AliasRegistry::findAddressByAlias()", alias, aliasHolder);
	try
	{
		Address addr;
		bool found = aliasMan->AliasToAddress(alias, addr);
		printFindResult("AliasManager::AliasToAddress()", alias, found, addr);
		if (!found && aliasHolder)
		{
			res->addFailure(101);
		}
		else if (found && !aliasHolder)
		{
			res->addFailure(102);
		}
		else if (found && aliasHolder)
		{
			Address tmp;
			found = (aliasHolder->aliasToAddress(alias, tmp) &&
				SmsUtil::compareAddresses(addr, tmp));
			if (!found)
			{
				res->addFailure(103);
			}
		}
	}
	catch(...)
	{
		Address tmp;
		if (aliasHolder->aliasToAddress(alias, tmp))
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* AliasManagerTestCases::checkInverseTransformation(const Address& addr)
{
	__require__(aliasReg && aliasMan);
	TCResult* res = new TCResult(TC_CHECK_INVERSE_TRANSFORMATION);
	try
	{
		Address alias = addr, addr2, alias2;
		/*
		if (aliasMan->AddressToAlias(addr, alias2))
		{
			__trace__("AliasManagerTestCases::checkInverseTransformation(): addr->alias->addr");
			if (!aliasMan->AliasToAddress(alias2, addr2))
			{
				res->addFailure(101);
			}
			else if (!SmsUtil::compareAddresses(addr, addr2))
			{
				res->addFailure(102);
			}
		}
		*/
		if (aliasMan->AliasToAddress(alias, addr2))
		{
			__trace__("AliasManagerTestCases::checkInverseTransformation(): alias->addr->alias");
			if (!aliasMan->AddressToAlias(addr2, alias2))
			{
				res->addFailure(103);
			}
			else if (!SmsUtil::compareAddresses(alias, alias2))
			{
				res->addFailure(104);
			}
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

