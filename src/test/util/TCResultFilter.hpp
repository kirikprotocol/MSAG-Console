#ifndef TEST_UTIL_TC_RESULT_FILTER
#define TEST_UTIL_TC_RESULT_FILTER

#include "Util.hpp"
#include <string>
#include <vector>
#include <map>

namespace smsc {
namespace test {
namespace util {

/**
 * ���� ��� ����������� ���������� test case
 * 
 * @author bryz
 */
class TCResultStack : public std::vector<const TCResult*>
{
public:
	virtual ~TCResultStack();
	bool operator== (const TCResult& result) const;
	bool operator== (const TCResultStack& stack) const;
};

class TCResultStackList : public std::vector<const TCResultStack*>
{
public:
	virtual ~TCResultStackList();
	friend std::ostream& operator<< (std::ostream& os,
		const TCResultStackList& stackList);
};

/**
 * ������ ��� ����������� ���������� test cases
 * 
 * @author bryz
 */
class TCResultFilter
{
public:
	~TCResultFilter();

	/**
	 * �������� ���������� ������������ test case
	 */
	void addResult(const TCResult* result);

	/**
	 * �������� ���������� ������������ test case
	 */
	void addResultStack(const TCResultStack& stack);

	/**
	 * @return ��������������� ����� �����������
	 */
	const TCResultStackList* getResults(const char* tcId);

private:
	typedef std::map<const std::string, TCResultStackList*> TCMap;
	TCMap resmap;
};

}
}
}

#endif /* TEST_UTIL_TC_RESULT_FILTER */
