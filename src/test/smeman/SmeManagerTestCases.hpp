#ifndef TEST_SMEMAN_SME_MANAGER_TEST_CASES
#define TEST_SMEMAN_SME_MANAGER_TEST_CASES

#include "smeman/smeman.h"
#include "test/util/Util.hpp"
#include "test/util/BaseTestCases.hpp"
#include <map>
#include <vector>

namespace smsc {
namespace test {
namespace smeman {

using std::ostream;
using std::map;
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
 * ���� ����� �������� test cases ����������� ��� ������������ ����������
 * SME manager.
 * ���� ����� registerSmeProxy, unregisterSmeProxy, selectSmeProxy
 * �� ���������������� � �������������� �� ����������� ����� �� �������
 * smsc.
 * 
 * @author bryz
 */
class SmeManagerTestCases : BaseTestCases
{
public:
	SmeManagerTestCases(SmeManager* smeMan);

	virtual ~SmeManagerTestCases() {}

	/**
	 * ����������� sme � ����������� �����������.
	 */
	TCResult* addCorrectSme(SmeInfo* sme, int num);
	
	/**
	 * ����������� sme � ������ systemId.
	 */
	TCResult* addCorrectSmeWithEmptySystemId(SmeInfo* sme);

	/**
	 * ����������� sme � ������������� �����������.
	 */
	TCResult* addIncorrectSme(const SmeInfo& existentSme);

	/**
	 * �������� ������������������� sme.
	 */
	TCResult* deleteExistentSme(const SmeSystemId& smeId);

	/**
	 * �������� ���������������������/��������������� sme.
	 */
	TCResult* deleteNonExistentSme();
	
	/**
	 * Disable ������������������� sme.
	 */
	//TCResult* disableExistentSme(SmeInfo* sme);
	
	/**
	 * Disable ���������������������/��������������� sme.
	 */
	//TCResult* disableNonExistentSme();
	
	/**
	 * Enable ������������������� sme.
	 */
	//TCResult* enableExistentSme(SmeInfo* sme);
	
	/**
	 * Enable ���������������������/��������������� sme.
	 */
	//TCResult* enableNonExistentSme();

	/**
	 * ��������� ������������������� sme.
	 */
	TCResult* getExistentSme(const SmeInfo& sme, SmeProxy* proxy);
	
	/**
	 * ��������� ���������������������/��������������� sme.
	 */
	TCResult* getNonExistentSme(const SmeSystemId& smeId, int num);

	/**
	 * ������������ �� ������ ������������������ sme.
	 */
	TCResult* iterateSme(const vector<SmeInfo*> sme);

	/**
	 * ������� sme ���������� ����������.
	 */
	TCResult* selectSme(const vector<SmeInfo*>& sme, int num);

	TCResult* registerCorrectSmeProxy(const SmeSystemId& systemId, SmeProxy** proxy);

protected:
	virtual Category& getLog();

private:
	SmeManager* smeMan;
	
	void setupRandomCorrectSmeInfo(SmeInfo* sme);
	vector<int> compareSmeInfo(const SmeInfo& sme1, const SmeInfo& sme2);
	void checkSelectSmeStat(const vector<SmeInfo*>& sme,
		const map<uint32_t, int>& statMap, TCResult* res);
	void debugSme(const char* tc, SmeInfo& sme);
};

}
}
}

#endif /* TEST_SMEMAN_SME_MANAGER_TEST_CASES */

