#ifndef TEST_UTIL_CHECK_LIST
#define TEST_UTIL_CHECK_LIST

#include "Util.hpp"
#include "TCResultFilter.hpp"
#include <fstream>
#include <map>
#include <string>

namespace smsc {
namespace test {
namespace util {

/**
 * ��������������� ����� ��� ������ ����������� � checklist.
 */
class CheckList
{
public:
	static const std::string UNIT_TEST;
	static const std::string SYSTEM_TEST;
	static const std::string STANDARDS_TEST;

	typedef std::map<std::string, CheckList*> CheckListMap;
	
	/**
	 * @return ���������� checklist
	 */
	static CheckList& getCheckList(const std::string& name);

	CheckList(const std::string& name);
	~CheckList();

	/**
	 * ������ ����� ������/������� � checklist.
	 */
	void startNewGroup(const std::string& groupName);

	/**
	 * �������� ���������� ������������ (test case) � ������� ������/�������
	 */
	void writeResult(const std::string& testCaseDesc, TCResult& result);

	/**
	 * �������� ���������� ������������ (������ test case) � ������� 
	 * ������/�������
	 */
	void writeResult(TCResultFilter& resultFilter);

private:
	static CheckListMap* clists;
	std::string name;
	std::ofstream os;
	int counter;
};

}
}
}

#endif /* TEST_UTIL_CHECK_LIST */
