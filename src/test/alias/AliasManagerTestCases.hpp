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
using smsc::test::core::AliasRegistry;
using smsc::test::core::AliasHolder;
using smsc::sms::Address;
using smsc::alias::AliasInfo;
using smsc::alias::AliasManager;

//implemented
const char* const TC_ADD_CORRECT_ALIAS_MATCH = "addCorrectAliasMatch";
const char* const TC_ADD_CORRECT_ALIAS_NOT_MATCH_ADDRESS =
	"addCorrectAliasNotMatchAddress";
const char* const TC_ADD_CORRECT_ALIAS_NOT_MATCH_ALIAS =
	"addCorrectAliasNotMatchAlias";
//const char* const TC_ADD_CORRECT_ALIAS_EXCEPTION = "addCorrectAliasException";
//const char* const TC_ADD_INCORRECT_ALIAS = "addIncorrectAlias";
const char* const TC_DELETE_ALIASES = "deleteAliases";
const char* const TC_FIND_ALIAS_BY_ADDRESS = "findAliasByAddress";
const char* const TC_FIND_ADDRESS_BY_ALIAS = "findAddressByAlias";
const char* const TC_ITERATE_ALIASES = "iterateAliases";

/**
 * Ётот класс содержит test cases необходимые дл€ тестировани€ подсистемы
 * alias manager.
 * @author bryz
 */
class AliasManagerTestCases : BaseTestCases
{
public:
	AliasManagerTestCases(AliasManager* manager, AliasRegistry* aliasReg);

	virtual ~AliasManagerTestCases() {}

	void commit();

	/**
	 * –егистраци€ алиаса с преобразованием addr->alias и alias->addr.
	 */
	TCResult* addCorrectAliasMatch(AliasInfo* alias, int num);

	/**
	 * –егистраци€ алиаса с преобразованием только alias->addr.
	 */
	TCResult* addCorrectAliasNotMatchAddress(AliasInfo* alias, int num);

	/**
	 * –егистраци€ алиаса с преобразованием только addr->alias.
	 */
	TCResult* addCorrectAliasNotMatchAlias(AliasInfo* alias, int num);

	/**
	 * –егистраци€ алиаса с переполнением адреса при alias->addr или алиаса при addr->alias.
	 */
	//TCResult* addCorrectAliasException(AliasInfo* alias, int num);

	/**
	 * –егистраци€ алиаса с некорректными параметрами.
	 */
	//TCResult* addIncorrectAlias();

	/**
	 * ќбнуление таблицы алиасов.
	 */
	TCResult* deleteAliases();

	/**
	 * ѕоиск алиаса по адресу.
	 */
	TCResult* findAliasByAddress(const Address& addr);

	/**
	 * ѕоиск адреса по алиасу.
	 */
	TCResult* findAddressByAlias(const Address& alias);

	/**
	 * »терирование по списку зарегистрированных алиасов.
	 */
	TCResult* iterateAliases();

protected:
	virtual Category& getLog();

private:
	AliasManager* aliasMan;
	AliasRegistry* aliasReg;
	
	void debugAlias(const char* tc, int val, const AliasInfo* alias);
	void setupRandomAliasMatchWithQuestionMarks(AliasInfo* alias, int len);
	//void setupRandomAliasMatchWithAsterisk(AliasInfo* alias, int adLen, int alLen);
	void printFindResult(const char* name, const Address& param,
		const AliasHolder* aliasHolder);
	void printFindResult(const char* tc, const Address& param, bool found,
		const Address& result);
};

}
}
}

#endif /* TEST_ALIAS_ALIAS_MANAGER_TEST_CASES */

