#ifndef TEST_ALIAS_ALIAS_MANAGER_TEST_CASES
#define TEST_ALIAS_ALIAS_MANAGER_TEST_CASES

#include "alias/aliasman.h"
#include "test/util/BaseTestCases.hpp"
#include "test/core/AliasUtil.hpp"
#include "test/core/AliasRegistry.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace alias {

using std::vector;
using log4cpp::Category;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::test::core::AliasRegistry;
using smsc::test::core::AliasHolder;
using smsc::sms::Address;
using smsc::alias::AliasInfo;
using smsc::alias::AliasManager;

/**
 * ���� ����� �������� test cases ����������� ��� ������������ ����������
 * alias manager.
 * @author bryz
 */
class AliasManagerTestCases : BaseTestCases
{
public:
	AliasManagerTestCases(AliasManager* manager, AliasRegistry* aliasReg,
		CheckList* chkList);

	virtual ~AliasManagerTestCases() {}

	void commit();

	/**
	 * ����������� ������ � ��������������� addr->alias � alias->addr.
	 */
	void addCorrectAliasMatch(AliasInfo* alias, int num);

	/**
	 * ����������� ������ � ��������������� ������ alias->addr.
	 */
	void addCorrectAliasNotMatchAddress(AliasInfo* alias, int num);

	/**
	 * ����������� ������ � ��������������� ������ addr->alias.
	 */
	void addCorrectAliasNotMatchAlias(AliasInfo* alias, int num);

	/**
	 * ����������� ������ � ������������� ������ ��� alias->addr ��� ������ ��� addr->alias.
	 */
	//void addCorrectAliasException(AliasInfo* alias, int num);

	/**
	 * ����������� ������ � ������������� �����������.
	 */
	//void addIncorrectAlias();

	/**
	 * ��������� ������� �������.
	 */
	void deleteAliases();

	/**
	 * ����� ������ �� ������.
	 */
	void findAliasByAddress(const Address& addr);

	/**
	 * ����� ������ �� ������.
	 */
	void findAddressByAlias(const Address& alias);

	/**
	 * �������������� addr->alias->addr � alias->addr->alias �����������.
	 */
	void checkInverseTransformation(const Address& addr);

	/**
	 * ������������ �� ������ ������������������ �������.
	 */
	void iterateAliases();

protected:
	virtual Category& getLog();

private:
	AliasManager* aliasMan;
	AliasRegistry* aliasReg;
	CheckList* chkList;
	
	void debugAlias(const char* tc, int val, const AliasInfo* alias);
	bool addAlias(const char* tc, int num, const AliasInfo* alias);
	void setupRandomAliasMatchWithQuestionMarks(AliasInfo* alias, int len);
	//void setupRandomAliasMatchWithAsterisk(AliasInfo* alias, int adLen, int alLen);
	void setupAliasHide(AliasInfo* alias);
	void printFindResult(const char* name, const Address& param,
		const Address* result, const AliasInfo* aliasInfo);
	void printFindResult(const char* tc, const Address& param,
		const Address& result, bool found);
};

}
}
}

#endif /* TEST_ALIAS_ALIAS_MANAGER_TEST_CASES */

