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
 * Ётот класс содержит test cases необходимые дл€ тестировани€ подсистемы
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
	 * –егистраци€ алиаса с преобразованием addr->alias и alias->addr.
	 */
	void addCorrectAliasMatch(AliasInfo* alias, int num);

	/**
	 * –егистраци€ алиаса с преобразованием только alias->addr.
	 */
	void addCorrectAliasNotMatchAddress(AliasInfo* alias, int num);

	/**
	 * –егистраци€ алиаса с преобразованием только addr->alias.
	 */
	void addCorrectAliasNotMatchAlias(AliasInfo* alias, int num);

	/**
	 * –егистраци€ алиаса с переполнением адреса при alias->addr или алиаса при addr->alias.
	 */
	//void addCorrectAliasException(AliasInfo* alias, int num);

	/**
	 * –егистраци€ алиаса с некорректными параметрами.
	 */
	//void addIncorrectAlias();

	/**
	 * ќбнуление таблицы алиасов.
	 */
	void deleteAliases();

	/**
	 * ѕоиск алиаса по адресу.
	 */
	void findAliasByAddress(const Address& addr);

	/**
	 * ѕоиск адреса по алиасу.
	 */
	void findAddressByAlias(const Address& alias);

	/**
	 * ѕреобразование addr->alias->addr и alias->addr->alias однозначное.
	 */
	void checkInverseTransformation(const Address& addr);

	/**
	 * »терирование по списку зарегистрированных алиасов.
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

