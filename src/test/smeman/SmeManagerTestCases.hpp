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
using smsc::test::util::CheckList;
using smsc::test::util::BaseTestCases;
using namespace smsc::smeman; //SmeManager, SmeSystemId, SmeInfo, SmeProxy

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
	SmeManagerTestCases(SmeManager* smeMan, SmeRegistry* smeReg,
		CheckList* chkList);

	virtual ~SmeManagerTestCases() {}

	/**
	 * –егистраци€ sme с корректными параметрами.
	 */
	void addCorrectSme(Address* smeAddr, SmeInfo* sme, int num);
	
	/**
	 * –егистраци€ sme с пустым systemId.
	 */
	void addCorrectSmeWithEmptySystemId(Address* smeAddr, SmeInfo* sme);

	/**
	 * –егистраци€ sme с некорректными параметрами.
	 */
	void addIncorrectSme(const SmeInfo& existentSme);

	/**
	 * ”даление зарегистрированного sme.
	 */
	void deleteExistentSme(const SmeSystemId& smeId);

	/**
	 * ”даление незарегистрированного/несуществующего sme.
	 */
	void deleteNonExistentSme();
	
	/**
	 * Disable зарегистрированного sme.
	 */
	//void disableExistentSme(SmeInfo* sme);
	
	/**
	 * Disable незарегистрированного/несуществующего sme.
	 */
	//void disableNonExistentSme();
	
	/**
	 * Enable зарегистрированного sme.
	 */
	//void enableExistentSme(SmeInfo* sme);
	
	/**
	 * Enable незарегистрированного/несуществующего sme.
	 */
	//void enableNonExistentSme();

	/**
	 * ѕолучение зарегистрированного sme.
	 */
	void getExistentSme(const SmeInfo& sme, SmeProxy* proxy);
	
	/**
	 * ѕолучение незарегистрированного/несуществующего sme.
	 */
	void getNonExistentSme(const SmeSystemId& smeId, int num);

	/**
	 * »терирование по списку зарегистрированных sme.
	 */
	void iterateSme();

	/**
	 * ¬ыборка sme происходит равномерно.
	 */
	//void selectSme(int num);

	void registerCorrectSmeProxy(const SmeSystemId& systemId, SmeProxy** proxy);

	static void setupRandomCorrectSmeInfo(SmeInfo* sme);

protected:
	virtual Category& getLog();

private:
	SmeManager* smeMan;
	SmeRegistry* smeReg;
	CheckList* chkList;
	
	vector<int> compareSmeInfo(const SmeInfo& sme1, const SmeInfo& sme2);
	void debugSme(const char* tc, const Address& smeAddr, const SmeInfo& sme);
	void addSme(const char* tc, int num, const Address* smeAddr, const SmeInfo* sme);
};

}
}
}

#endif /* TEST_SMEMAN_SME_MANAGER_TEST_CASES */

