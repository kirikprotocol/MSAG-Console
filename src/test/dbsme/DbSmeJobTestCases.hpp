#ifndef TEST_DBSME_DBSME_JOB_TEST_CASES
#define TEST_DBSME_DBSME_JOB_TEST_CASES

#include "DbSmeRegistry.hpp"
#include "DateFormatter.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include <string>
#include <ostream>
#include <sstream>

namespace smsc {
namespace test {
namespace dbsme {

using std::string;
using std::ostream;
using std::ostringstream;
using smsc::test::conf::TestConfig;
using smsc::test::core::SmeAckMonitor;
using smsc::test::util::CheckList;

/**
 * Абстрактный класс для всех jobs.
 */
class DbSmeJobTestCases
{
public:
	DbSmeJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList);
	//DbSmeTestRecord* createJobInput();
	void processJobOutput(const string& text, DbSmeTestRecord* rec,
		SmeAckMonitor* monitor);
	virtual const string processJobFirstOutput(const string& text,
		DbSmeTestRecord* rec) = NULL;

protected:
	DbSmeRegistry* dbSmeReg;
	CheckList* chkList;

	//input
	void setInputInt16(DbSmeTestRecord* rec, int val);
	void setRandomInputInt16(DbSmeTestRecord* rec);
	void setInputInt32(DbSmeTestRecord* rec, int val);
	void setRandomInputInt32(DbSmeTestRecord* rec);
	void setInputFloat(DbSmeTestRecord* rec, double val);
	void setRandomInputFloat(DbSmeTestRecord* rec);
	void setInputDouble(DbSmeTestRecord* rec, double val);
	void setRandomInputDouble(DbSmeTestRecord* rec);
	void setInputDate(DbSmeTestRecord* rec, time_t val);
	void setRandomInputDate(DbSmeTestRecord* rec);
	void setInputString(DbSmeTestRecord* rec, const string& val);
	void setInputQuotedString(DbSmeTestRecord* rec, const string& val);
	void setRandomInputString(DbSmeTestRecord* rec, bool quotedString);
	//output
	const string getOutputFromAddress(const DbSmeTestRecord* rec);
	const string getOutputString(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
	time_t getDate(DateType dtType, time_t now = 0);
	const string getOutputDate(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, const DateFormatter& df, bool& res);
	int getOutputInt16(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
	int getOutputInt32(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
	float getOutputFloat(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
	double getOutputDouble(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
};

}
}
}

#endif /* TEST_DBSME_DBSME_JOB_TEST_CASES */

