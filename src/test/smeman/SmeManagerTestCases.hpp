#ifndef TEST_SMEMAN_SME_MANAGER_TEST_CASES
#define TEST_SMEMAN_SME_MANAGER_TEST_CASES

#include "smeman/smeman.h"
#include "test/util/Util.hpp"
#include "test/util/BaseTestCases.hpp"
#include "test/core/SmeRegistry.hpp"
#include <map>
#include <vector>

namespace smsc {
namespace test {
namespace smeman {

using std::ostream;
using std::map;
using std::vector;
using log4cpp::Category;
using smsc::test::core::SmeRegistry;
using smsc::test::util::TCResult;
using smsc::test::util::BaseTestCases;
using namespace smsc::smeman; //SmeManager, SmeSystemId, SmeInfo, SmeProxy

//implemented
const char* const TC_ADD_CORRECT_SME = "addCorrectSme";
const char* const TC_ADD_INCORRECT_SME = "addIncorrectSme";
const char* const TC_DELETE_EXISTENT_SME = "deleteExistentSme";
const char* const TC_DELETE_NON_EXISTENT_SME = "deleteNonExistentSme";
//const char* const TC_DISABLE_EXISTENT_SME = "disableExistentSme";
//const char* const TC_DISABLE_NON_EXISTENT_SME = "disableNonExistentSme";
//const char* const TC_ENABLE_EXISTENT_SME = "enableExistentSme";
//const char* const TC_ENABLE_NON_EXISTENT_SME = "enableNonExistentSme";
const char* const TC_GET_EXISTENT_SME = "getExistentSme";
const char* const TC_GET_NON_EXISTENT_SME = "getNonExistentSme";
const char* const TC_ITERATE_SME = "iterateSme";
const char* const TC_SELECT_SME = "selectSme";
const char* const TC_REGISTER_CORRECT_SME_PROXY = "registerCorrectSmeProxy";

ostream& operator<< (ostream& os, const SmeInfo& sme);

/**
 * Ётот класс содержит test cases необходимые дл€ тестировани€ подсистемы
 * SME manager.
 * “ест кейсы registerSmeProxy, unregisterSmeProxy, selectSmeProxy
 * не имплементированы и предполагаетс€ их тестировать извне на рабочем
 * smsc.
 * 
 * @author bryz
 */
class SmeManagerTestCases : BaseTestCases
{
public:
	SmeManagerTestCases(SmeManager* smeMan, SmeRegistry* smeReg);

	virtual ~SmeManagerTestCases() {}

	/**
	 * –егистраци€ sme с корректными параметрами.
	 */
	TCResult* addCorrectSme(Address* smeAlias, SmeInfo* sme, int num);
	
	/**
	 * –егистраци€ sme с пустым systemId.
	 */
	TCResult* addCorrectSmeWithEmptySystemId(Address* smeAlias, SmeInfo* sme);

	/**
	 * –егистраци€ sme с некорректными параметрами.
	 */
	TCResult* addIncorrectSme(const SmeInfo& existentSme);

	/**
	 * ”даление зарегистрированного sme.
	 */
	TCResult* deleteExistentSme(const SmeSystemId& smeId);

	/**
	 * ”даление незарегистрированного/несуществующего sme.
	 */
	TCResult* deleteNonExistentSme();
	
	/**
	 * Disable зарегистрированного sme.
	 */
	//TCResult* disableExistentSme(SmeInfo* sme);
	
	/**
	 * Disable незарегистрированного/несуществующего sme.
	 */
	//TCResult* disableNonExistentSme();
	
	/**
	 * Enable зарегистрированного sme.
	 */
	//TCResult* enableExistentSme(SmeInfo* sme);
	
	/**
	 * Enable незарегистрированного/несуществующего sme.
	 */
	//TCResult* enableNonExistentSme();

	/**
	 * ѕолучение зарегистрированного sme.
	 */
	TCResult* getExistentSme(const SmeInfo& sme, SmeProxy* proxy);
	
	/**
	 * ѕолучение незарегистрированного/несуществующего sme.
	 */
	TCResult* getNonExistentSme(const SmeSystemId& smeId, int num);

	/**
	 * »терирование по списку зарегистрированных sme.
	 */
	TCResult* iterateSme();

	/**
	 * ¬ыборка sme происходит равномерно.
	 */
	//TCResult* selectSme(int num);

	TCResult* registerCorrectSmeProxy(const SmeSystemId& systemId, SmeProxy** proxy);

protected:
	virtual Category& getLog();

private:
	SmeManager* smeMan;
	SmeRegistry* smeReg;
	
	void setupRandomCorrectSmeInfo(SmeInfo* sme);
	vector<int> compareSmeInfo(const SmeInfo& sme1, const SmeInfo& sme2);
	void debugSme(const char* tc, const Address& smeAlias, const SmeInfo& sme);
	void addSme(const char* tc, int num, const Address* smeAlias, const SmeInfo* sme);
};

}
}
}

#endif /* TEST_SMEMAN_SME_MANAGER_TEST_CASES */

