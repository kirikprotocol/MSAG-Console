#ifndef TEST_ALIAS_ALIAS_MANAGER_TEST_CASES
#define TEST_ALIAS_ALIAS_MANAGER_TEST_CASES

#include "alias/aliasman.h"
#include "test/util/Util.hpp"
#include "test/util/BaseTestCases.hpp"
#include "test/core/AliasUtil.hpp"
#include "test/core/AliasRegistry.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace alias {

using std::vector;
using log4cpp::Category;
using smsc::test::util::TCResult;
using smsc::test::util::BaseTestCases;
using smsc::test::core::TestAliasData;
using smsc::test::core::AliasRegistry;
using smsc::sms::Address;
using smsc::alias::AliasInfo;
using smsc::alias::AliasManager;

//implemented
const char* const TC_ADD_CORRECT_ALIAS_MATCH = "addCorrectAliasMatch";
const char* const TC_ADD_CORRECT_ALIAS_NOT_MATCH_ADDRESS =
	"addCorrectAliasNotMatchAddress";
const char* const TC_ADD_CORRECT_ALIAS_NOT_MATCH_ALIAS =
	"addCorrectAliasNotMatchAlias";
const char* const TC_ADD_CORRECT_ALIAS_EXCEPTION = "addCorrectAliasException";
const char* const TC_ADD_INCORRECT_ALIAS = "addIncorrectAlias";
const char* const TC_DELETE_ALIASES = "deleteAliases";
const char* const TC_FIND_ALIAS_BY_ADDRESS = "findAliasByAddress";
const char* const TC_FIND_ADDRESS_BY_ALIAS = "findAddressByAlias";
const char* const TC_ITERATE_ALIASES = "iterateAliases";

/**
 * ���� ����� �������� test cases ����������� ��� ������������ ����������
 * alias manager.
 * @author bryz
 */
class AliasManagerTestCases : BaseTestCases
{
public:
	AliasManagerTestCases(AliasManager* manager);

	virtual ~AliasManagerTestCases() {}

	/**
	 * ����������� ������ � ��������������� addr->alias � alias->addr.
	 */
	TCResult* addCorrectAliasMatch(TestAliasData* data, int num);

	/**
	 * ����������� ������ � ��������������� ������ alias->addr.
	 */
	TCResult* addCorrectAliasNotMatchAddress(TestAliasData* data, int num);

	/**
	 * ����������� ������ � ��������������� ������ addr->alias.
	 */
	TCResult* addCorrectAliasNotMatchAlias(TestAliasData* data, int num);

	/**
	 * ����������� ������ � ������������� ������ ��� alias->addr ��� ������ ��� addr->alias.
	 */
	TCResult* addCorrectAliasException(TestAliasData* data, int num);

	/**
	 * ����������� ������ � ������������� �����������.
	 */
	TCResult* addIncorrectAlias();

	/**
	 * ��������� ������� �������.
	 */
	TCResult* deleteAliases();

	/**
	 * ����� ������ �� ������.
	 */
	TCResult* findAliasByAddress(const AliasRegistry& aliasReg,
		const Address& addr);

	/**
	 * ����� ������ �� ������.
	 */
	TCResult* findAddressByAlias(const AliasRegistry& aliasReg,
		const Address& alias);

	/**
	 * ������������ �� ������ ������������������ �������.
	 */
	TCResult* iterateAliases(const AliasRegistry& aliasReg);

protected:
	virtual Category& getLog();

private:
	AliasManager* aliasMan;
	
	AliasInfo* newAliasInfo(TestAliasData* data);
	void setupRandomAliasMatchWithQuestionMarks(TestAliasData* data, int len);
	void setupRandomAliasMatchWithAsterisk(TestAliasData* data, int adLen, int alLen);
};

}
}
}

#endif /* TEST_ALIAS_ALIAS_MANAGER_TEST_CASES */

