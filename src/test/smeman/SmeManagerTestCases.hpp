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
	SmeManagerTestCases(SmeManager* smeMan, SmeRegistry* smeReg,
		CheckList* chkList);

	virtual ~SmeManagerTestCases() {}

	/**
	 * ����������� sme � ����������� �����������.
	 */
	void addCorrectSme(Address* smeAddr, SmeInfo* sme, int num);
	
	/**
	 * ����������� sme � ������ systemId.
	 */
	void addCorrectSmeWithEmptySystemId(Address* smeAddr, SmeInfo* sme);

	/**
	 * ����������� sme � ������������� �����������.
	 */
	void addIncorrectSme(const SmeInfo& existentSme);

	/**
	 * �������� ������������������� sme.
	 */
	void deleteExistentSme(const SmeSystemId& smeId);

	/**
	 * �������� ���������������������/��������������� sme.
	 */
	void deleteNonExistentSme();
	
	/**
	 * Disable ������������������� sme.
	 */
	//void disableExistentSme(SmeInfo* sme);
	
	/**
	 * Disable ���������������������/��������������� sme.
	 */
	//void disableNonExistentSme();
	
	/**
	 * Enable ������������������� sme.
	 */
	//void enableExistentSme(SmeInfo* sme);
	
	/**
	 * Enable ���������������������/��������������� sme.
	 */
	//void enableNonExistentSme();

	/**
	 * ��������� ������������������� sme.
	 */
	void getExistentSme(const SmeInfo& sme, SmeProxy* proxy);
	
	/**
	 * ��������� ���������������������/��������������� sme.
	 */
	void getNonExistentSme(const SmeSystemId& smeId, int num);

	/**
	 * ������������ �� ������ ������������������ sme.
	 */
	void iterateSme();

	/**
	 * ������� sme ���������� ����������.
	 */
	//void selectSme(int num);

	void registerCorrectSmeProxy(const SmeSystemId& systemId, SmeProxy** proxy);

protected:
	virtual Category& getLog();

private:
	SmeManager* smeMan;
	SmeRegistry* smeReg;
	CheckList* chkList;
	
	void setupRandomCorrectSmeInfo(SmeInfo* sme);
	vector<int> compareSmeInfo(const SmeInfo& sme1, const SmeInfo& sme2);
	void debugSme(const char* tc, const Address& smeAddr, const SmeInfo& sme);
	void addSme(const char* tc, int num, const Address* smeAddr, const SmeInfo* sme);
};

}
}
}

#endif /* TEST_SMEMAN_SME_MANAGER_TEST_CASES */

