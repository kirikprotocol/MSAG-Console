#ifndef TEST_UTIL_TC_RESULT_FILTER
#define TEST_UTIL_TC_RESULT_FILTER

#include "Util.hpp"
#include <vector>
#include <map>

namespace smsc {
namespace test {
namespace util {

class CheckList;

/**
 * ���� ��� ����������� ���������� test case
 * 
 * @author bryz
 */
class TCResultStack : public std::vector<TCResult*>
{
public:
	~TCResultStack();
	bool operator== (const TCResult& result) const;
	bool operator== (const TCResultStack& stack) const;
};

/**
 * ������ ��� ����������� ���������� test cases
 * 
 * @author bryz
 */
class TCResultFilter
{
public:
	//friend void CheckList::writeResult(const TCResultFilter&);
	friend class CheckList;

	struct TCValue
	{
		bool used;
		std::string description;
		std::vector<TCResultStack*> tcStacks;
		
		TCValue(const std::string& _description)
			: description(_description), used(false) {}
	};
	typedef std::map<std::string, TCValue*> TCMap;

	~TCResultFilter();

	/**
	 * ������������� ������������ ����� test case id � ��������� ��������� test
	 * case.
	 */
	void registerTC(const std::string& tcId, const std::string& tcDescription);

	/**
	 * �������� ���������� ������������ test case
	 */
	void addResult(TCResult& result);

	/**
	 * �������� ���������� ������������ test case
	 */
	void addResultStack(TCResultStack& stack);

private:
	TCMap resmap;

	TCValue& getTCValue(const std::string& tcId);
};

}
}
}

#endif /* TEST_UTIL_TC_RESULT_FILTER */
