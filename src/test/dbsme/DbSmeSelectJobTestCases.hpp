#ifndef TEST_DBSME_DBSME_SELECT_JOB_TEST_CASES
#define TEST_DBSME_DBSME_SELECT_JOB_TEST_CASES

#include "DbSmeJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

class DbSmeSelectJobTestCases : public DbSmeJobTestCases
{
	void writeSelectJobRecord(ostream& os, DbSmeTestRecord* rec,
		DbSmeTestRecord* defOutput, time_t now);
	const string processSelectNullsJobOutput(const string& text);
	const string processSelectValuesJobOutput();
	const string processSelectNoDefaultsJobOutput();
public:
	DbSmeSelectJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	DbSmeTestRecord* createSelectNullsJobInput();
	DbSmeTestRecord* createSelectValuesJobInput();
	DbSmeTestRecord* createSelectNoDefaultsJobInput();
	virtual const string processJobFirstOutput(const string& text,
		DbSmeTestRecord* rec);
};

}
}
}

#endif /* TEST_DBSME_DBSME_SELECT_JOB_TEST_CASES */

