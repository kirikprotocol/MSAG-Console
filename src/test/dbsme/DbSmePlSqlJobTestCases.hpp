#ifndef TEST_DBSME_PLSQL_JOB_TEST_CASES
#define TEST_DBSME_PLSQL_JOB_TEST_CASES

#include "DbSmeJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

class DbSmePlSqlJobTestCases : public DbSmeJobTestCases
{
	DbSmeTestRecord* newDefInput();
	const string processProcedureJobOutput(const string& text,
		DbSmeTestRecord* rec);
	const string processFunctionJobOutput(const string& text,
		DbSmeTestRecord* rec);
	const string processExceptionJobOutput(const string& text,
		DbSmeTestRecord* rec);

public:
	DbSmePlSqlJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	
	const DateFormatter& getDateFormatter();
	DbSmeTestRecord* createDefaultsProcedureJobInput();
	DbSmeTestRecord* createProcedureJobInput();
	DbSmeTestRecord* createDefaultsFunctionJobInput();
	DbSmeTestRecord* createFunctionJobInput();
	DbSmeTestRecord* createExceptionJobInput();
	virtual const string processJobFirstOutput(const string& text,
		DbSmeTestRecord* rec);
};

}
}
}

#endif /* TEST_DBSME_PLSQL_JOB_TEST_CASES */

