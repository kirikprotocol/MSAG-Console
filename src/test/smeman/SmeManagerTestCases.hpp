#ifndef TEST_SMEMAN_SME_MANAGER_TEST_CASES
#define TEST_SMEMAN_SME_MANAGER_TEST_CASES

#include "smeman/smeman.h"
#include "test/util/Util.hpp"
#include "test/util/BaseTestCases.hpp"
#include <vector>

namespace smsc {
namespace test {
namespace smeman {

using std::vector;
using log4cpp::Category;
using smsc::test::util::TCResult;
using smsc::test::util::BaseTestCases;
using smsc::smeman::SmeManager;
using smsc::smeman::SmeSystemId;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeProxy;

//implemented
const char* const TC_ADD_CORRECT_SME = "addCorrectSme";
const char* const TC_ADD_INCORRECT_SME = "addIncorrectSme";
const char* const TC_DELETE_EXISTENT_SME = "deleteExistentSme";
const char* const TC_DELETE_NON_EXISTENT_SME = "deleteNonExistentSme";
const char* const TC_DISABLE_EXISTENT_SME = "disableExistentSme";
const char* const TC_DISABLE_NON_EXISTENT_SME = "disableNonExistentSme";
const char* const TC_ENABLE_EXISTENT_SME = "enableExistentSme";
const char* const TC_ENABLE_NON_EXISTENT_SME = "enableNonExistentSme";
const char* const TC_GET_EXISTENT_SME = "getExistentSme";
const char* const TC_GET_NON_EXISTENT_SME = "getNonExistentSme";
const char* const TC_ITERATE_SME = "iterateSme";

//max values
const int MAX_SYSTEM_ID_LENGTH = 15;
const int MAX_PASSWORD_LENGTH = 8;
const int MAX_SYSTEM_TYPE_LENGTH = 12;
const int MAX_ADDRESS_RANGE_LENGTH = 40;

/**
 * Этот класс содержит test cases необходимые для тестирования подсистемы
 * SME manager.
 * 
 * @author bryz
 */
class SmeManagerTestCases : BaseTestCases
{
public:
	SmeManagerTestCases();

	virtual ~SmeManagerTestCases() {}

	/**
	 * Регистрация sme с корректными параметрами.
	 */
	TCResult* addCorrectSme(SmeInfo* sme, int num);
	
	/**
	 * Регистрация sme с некорректными параметрами.
	 */
	TCResult* addIncorrectSme(const SmeInfo& existentSme);

	/**
	 * Удаление зарегистрированного sme.
	 */
	TCResult* deleteExistentSme(const SmeSystemId& smeId);

	/**
	 * Удаление незарегистрированного/несуществующего sme.
	 */
	TCResult* deleteNonExistentSme();
	
	/**
	 * Disable зарегистрированного sme.
	 */
	TCResult* disableExistentSme(SmeInfo* sme);
	
	/**
	 * Disable незарегистрированного/несуществующего sme.
	 */
	TCResult* disableNonExistentSme();
	
	/**
	 * Enable зарегистрированного sme.
	 */
	TCResult* enableExistentSme(SmeInfo* sme);
	
	/**
	 * Enable незарегистрированного/несуществующего sme.
	 */
	TCResult* enableNonExistentSme();

	/**
	 * Получение зарегистрированного sme.
	 */
	TCResult* getExistentSme(const SmeInfo& sme, SmeProxy* proxy);
	
	/**
	 * Получение незарегистрированного/несуществующего sme.
	 */
	TCResult* getNonExistentSme(const SmeSystemId& smeId, int num);

	/**
	 * Итерирование по списку зарегистрированных sme.
	 */
	TCResult* iterateSme(const vector<SmeInfo*> sme);

protected:
	virtual Category& getLog();

private:
	SmeManager* smeMan;
	
	void setupRandomCorrectSmeInfo(SmeInfo* info);
	vector<int> compareSmeInfo(const SmeInfo& info1, const SmeInfo& info2);
};

}
}
}

#endif /* TEST_SMEMAN_SME_MANAGER_TEST_CASES */

