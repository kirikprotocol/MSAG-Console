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

//max values
const int MAX_SYSTEM_ID_LENGTH = 15;
const int MAX_PASSWORD_LENGTH = 8;
const int MAX_SYSTEM_TYPE_LENGTH = 12;
const int MAX_ADDRESS_RANGE_LENGTH = 40;

/**
 * ���� ����� �������� test cases ����������� ��� ������������ ����������
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
	 * ����������� sme � ����������� �����������.
	 */
	TCResult* addCorrectSme(SmeInfo* info, int num);
	
	/**
	 * ����������� sme � ������������� �����������.
	 */
	TCResult* addIncorrectSme(const SmeInfo& existentSme);

	/**
	 * �������� ������������������� sme.
	 */
	TCResult* deleteExistentSme(const SmeSystemId& systemId);

	/**
	 * �������� ���������������������/��������������� sme.
	 */
	TCResult* deleteNonExistentSme();
	
	/**
	 * Disable ������������������� sme.
	 */
	TCResult* disableExistentSme(SmeInfo* info);
	
	/**
	 * Disable ���������������������/��������������� sme.
	 */
	TCResult* disableNonExistentSme();
	
	/**
	 * Enable ������������������� sme.
	 */
	TCResult* enableExistentSme(SmeInfo* info);
	
	/**
	 * Enable ���������������������/��������������� sme.
	 */
	TCResult* enableNonExistentSme();

	/**
	 * ��������� ������������������� sme.
	 */
	TCResult* getExistentSme(const SmeInfo& info, SmeProxy* proxy);
	
	/**
	 * ��������� ���������������������/��������������� sme.
	 */
	TCResult* getNonExistentSme(const SmeSystemId& systemId, int num);

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

