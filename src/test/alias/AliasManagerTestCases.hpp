#ifndef TEST_ALIAS_ALIAS_MANAGER_TEST_CASES
#define TEST_ALIAS_ALIAS_MANAGER_TEST_CASES

#include "alias/aliasman.h"
#include "test/util/Util.hpp"
#include "test/util/BaseTestCases.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace alias {

using std::vector;
using log4cpp::Category;
using smsc::test::util::TCResult;
using smsc::test::util::BaseTestCases;
using smsc::sms::Address;
using smsc::alias::AliasInfo;
using smsc::alias::AliasManager;

//implemented
const char* const TC_ADD_CORRECT_ALIAS = "addCorrectAlias";
const char* const TC_ADD_INCORRECT_ALIAS = "addIncorrectAlias";
const char* const TC_DELETE_ALIASES = "deleteAliases";
const char* const TC_FIND_EXISTENT_ALIAS_BY_ADDRESS =
	"findExistentAliasByAddress";
const char* const TC_FIND_EXISTENT_ADDRESS_BY_ALIAS =
	"findExistentAddressByAlias";
const char* const TC_FIND_NON_EXISTENT_ALIAS_BY_ADDRESS =
	"findNonExistentAliasByAddress";
const char* const TC_FIND_NON_EXISTENT_ADDRESS_BY_ALIAS =
	"findNonExistentAddressByAlias";
const char* const TC_ITERATE_ALIASES = "iterateAliases";

/**
 * Ётот класс содержит test cases необходимые дл€ тестировани€ подсистемы
 * alias manager.
 * @author bryz
 */
class AliasManagerTestCases : BaseTestCases
{
public:
	AliasManagerTestCases();

	virtual ~AliasManagerTestCases() {}

	/**
	 * –егистраци€ алиаса с корректными параметрами.
	 */
	TCResult* addCorrectAlias(AliasInfo* alias);

	/**
	 * –егистраци€ алиаса с корректными параметрами, но очень похожими на
	 * уже зарегистрированный алиас.
	 */
	TCResult* addCorrectAlias(AliasInfo* alias,
		const AliasInfo& existentAlias, int num);

	/**
	 * –егистраци€ алиаса с некорректными параметрами.
	 */
	TCResult* addIncorrectAlias(const AliasInfo& existentAlias, int num);

	/**
	 * ќбнуление таблицы алиасов.
	 */
	TCResult* deleteAliases();

	/**
	 * ѕоиск существующего алиаса по адресу.
	 */
	TCResult* findExistentAliasByAddress(const AliasInfo& existentAlias);

	/**
	 * ѕоиск существующего адреса по алиасу.
	 */
	TCResult* findExistentAddressByAlias(const AliasInfo& existentAlias);

	/**
	 * ѕоиск несуществующего алиаса по адресу.
	 */
	TCResult* findNonExistentAliasByAddress(const Address& addr);

	/**
	 * ѕоиск несуществующего адреса по алиасу.
	 */
	TCResult* findNonExistentAddressByAlias(const Address& alias);

	/**
	 * »терирование по списку зарегистрированных алиасов.
	 */
	TCResult* iterateAliases(const vector<AliasInfo*> sme);

protected:
	virtual Category& getLog();

private:
	AliasManager* aliasMan;
};

}
}
}

#endif /* TEST_ALIAS_ALIAS_MANAGER_TEST_CASES */

