#ifndef TEST_DBSME_DBSME_MODIFY_JOB_TEST_CASES
#define TEST_DBSME_DBSME_MODIFY_JOB_TEST_CASES

#include "DbSmeJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

class DbSmeInsertJobTestCases : public DbSmeJobTestCases
{
	DbSmeTestRecord* newDefInput1();
	DbSmeTestRecord* newDefInput2();
public:
	DbSmeInsertJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	const DateFormatter& getDateFormatter();
	DbSmeTestRecord* createIntsJobInput();
	DbSmeTestRecord* createUintsJobInput();
	DbSmeTestRecord* createIntDefaultsJobInput();
	DbSmeTestRecord* createUintDefaultsJobInput();
	DbSmeTestRecord* createZerosJobInput();
	DbSmeTestRecord* createNullsJobInput();
	DbSmeTestRecord* createDuplicateKeyJobInput();
	virtual const string processJobFirstOutput(const string& text,
		DbSmeTestRecord* rec);
};

class DbSmeUpdateJobTestCases : public DbSmeJobTestCases
{
public:
	DbSmeUpdateJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	const DateFormatter& getDateFormatter();
	DbSmeTestRecord* createUpdateRecordJobInput();
	DbSmeTestRecord* createDuplicateKeyJobInput();
	virtual const string processJobFirstOutput(const string& text,
		DbSmeTestRecord* rec);
};

class DbSmeDeleteJobTestCases : public DbSmeJobTestCases
{
public:
	DbSmeDeleteJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	DbSmeTestRecord* createDeleteAllJobInput();
	virtual const string processJobFirstOutput(const string& text,
		DbSmeTestRecord* rec);
};

}
}
}

#endif /* TEST_DBSME_DBSME_MODIFY_JOB_TEST_CASES */

