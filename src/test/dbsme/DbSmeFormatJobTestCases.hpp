#ifndef TEST_DBSME_DBSME_FORMAT_JOB_TEST_CASES
#define TEST_DBSME_DBSME_FORMAT_JOB_TEST_CASES

#include "DbSmeJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

class DbSmeDateFormatJobTestCases : public DbSmeJobTestCases
{
public:
	DbSmeDateFormatJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	DbSmeTestRecord* createJobInput(int jobNum, bool params,
		const DateFormatter** df);
	virtual const string processJobFirstOutput(const string& text, DbSmeTestRecord* rec);
};

class DbSmeOtherFormatJobTestCases : public DbSmeJobTestCases
{
	DbSmeTestRecord* newDefInput();
public:
	DbSmeOtherFormatJobTestCases(DbSmeRegistry* dbSmeReg, CheckList* chkList)
		: DbSmeJobTestCases(dbSmeReg, chkList) {}
	DbSmeTestRecord* createValuesJobInput();
	DbSmeTestRecord* createDefaultsJobInput();
	virtual const string processJobFirstOutput(const string& text, DbSmeTestRecord* rec);
};

}
}
}

#endif /* TEST_DBSME_DBSME_FORMAT_JOB_TEST_CASES */

