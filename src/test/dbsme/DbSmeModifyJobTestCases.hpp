#ifndef TEST_DBSME_DBSME_MODIFY_JOB_TEST_CASES
#define TEST_DBSME_DBSME_MODIFY_JOB_TEST_CASES

#include "DbSmeJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

class DbSmeInsertJobTestCases : public DbSmeJobTestCases
{
	DbSmeTestRecord* createDefaultInput();
public:
	DbSmeInsertJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	const DateFormatter& getDateFormatter();
	DbSmeTestRecord* createJobInput(bool params);
	DbSmeTestRecord* createDuplicateKeyJobInput();
	DbSmeTestRecord* createZerosJobInput();
	virtual const string processJobFirstOutput(const string& text, DbSmeTestRecord* rec);
};

class DbSmeUpdateJobTestCases : public DbSmeJobTestCases
{
public:
	DbSmeUpdateJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	const DateFormatter& getDateFormatter();
	DbSmeTestRecord* createJobInput(bool params);
	DbSmeTestRecord* createDuplicateKeyJobInput();
	virtual const string processJobFirstOutput(const string& text, DbSmeTestRecord* rec);
};

class DbSmeDeleteJobTestCases : public DbSmeJobTestCases
{
public:
	DbSmeDeleteJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	DbSmeTestRecord* createJobInput();
	virtual const string processJobFirstOutput(const string& text, DbSmeTestRecord* rec);
};

}
}
}

#endif /* TEST_DBSME_DBSME_MODIFY_JOB_TEST_CASES */

