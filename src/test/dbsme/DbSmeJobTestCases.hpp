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
using namespace smsc::test::util;

#define __set_input_float__(type, typeName) \
void setInput##typeName(DbSmeTestRecord* rec, type val) { \
	__decl_tc__; \
	__tc__("submitDbSmeCmd.correct.input.float"); __tc_ok__; \
	rec->set##typeName(val); \
}

#define __set_input_int__(type, size) \
void setInputInt##size(DbSmeTestRecord* rec, type val) { \
	__decl_tc__; \
	__tc__("submitDbSmeCmd.correct.input.int"); __tc_ok__; \
	rec->setInt##size(val); \
}

#define __set_input_uint__(type, size) \
void setInputUint##size(DbSmeTestRecord* rec, type val) { \
	__decl_tc__; \
	__tc__("submitDbSmeCmd.correct.input.uint"); __tc_ok__; \
	rec->setInt##size(val); \
}

#define __set_random_int__(typeName, minVal, maxVal) \
void setRandomInput##typeName(DbSmeTestRecord* rec) { \
	setInput##typeName(rec, rand2(0, maxVal)); \
}
	
#define __set_random_float__(typeName, len, precision) \
void setRandomInput##typeName(DbSmeTestRecord* rec) { \
	setInput##typeName(rec, randDouble(len, precision)); \
}

#define __get_output__(type, typeName, tcType) \
type getOutput##typeName(const DbSmeTestRecord* rec, \
	const DbSmeTestRecord* defOutput, bool& res) { \
	__decl_tc__; \
	if (rec) { \
		if (rec->check##typeName()) { \
			__tc__("processDbSmeRes.input." tcType); __tc_ok__; \
			return rec->get##typeName(); \
		} \
		if (rec->getDefInput() && rec->getDefInput()->check##typeName()) { \
			__tc__("processDbSmeRes.defaultInput." tcType); __tc_ok__; \
			return rec->getDefInput()->get##typeName(); \
		} \
	} \
	if (defOutput && defOutput->check##typeName()) { \
		__tc__("processDbSmeRes.select.defaultOutput." tcType); __tc_ok__; \
		return defOutput->get##typeName(); \
	} \
	res = false; \
	return 0; \
}

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
	__set_input_int__(int, 8)
	__set_input_int__(int, 16)
	__set_input_int__(int, 32)
	__set_input_int__(int, 64)
	__set_input_uint__(int, 8)
	__set_input_uint__(int, 16)
	__set_input_uint__(int, 32)
	__set_input_uint__(int, 64)
	__set_input_float__(double, Float)
	__set_input_float__(double, Double)
	__set_input_float__(double, LongDouble)
	void setInputString(DbSmeTestRecord* rec, const string& val);
	void setInputQuotedString(DbSmeTestRecord* rec, const string& val);
	void setInputDate(DbSmeTestRecord* rec, time_t val);
	//random input
	__set_random_int__(Int8, -127, 127)
	__set_random_int__(Int16, -32767, 32767)
	__set_random_int__(Int32, INT_MIN + 1, INT_MAX - 1)
	__set_random_int__(Int64, INT_MIN + 1, INT_MAX - 1)
	__set_random_int__(Uint8, 0, 255)
	__set_random_int__(Uint16, 0, 65534)
	__set_random_int__(Uint32, 0, UINT_MAX - 1)
	__set_random_int__(Uint64, 0, UINT_MAX - 1)
	__set_random_float__(Float, 5, 2)
	__set_random_float__(Double, 5, 2)
	__set_random_float__(LongDouble, 5, 2)
	void setRandomInputString(DbSmeTestRecord* rec, bool quotedString);
	void setRandomInputDate(DbSmeTestRecord* rec);
	//output
	__get_output__(int, Int8, "int")
	__get_output__(int, Int16, "int")
	__get_output__(int, Int32, "int")
	__get_output__(int, Int64, "int")
	__get_output__(double, Float, "float")
	__get_output__(double, Double, "float")
	__get_output__(double, LongDouble, "float")
	const string getOutputJobName(const DbSmeTestRecord* rec);
	const string getOutputToAddress(const DbSmeTestRecord* rec);
	const string getOutputFromAddress(const DbSmeTestRecord* rec);
	const string getOutputString(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
	time_t getDate(DateType dtType, time_t now = 0);
	const string getOutputDate(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, const DateFormatter& df, bool& res);
};

}
}
}

#endif /* TEST_DBSME_DBSME_JOB_TEST_CASES */

