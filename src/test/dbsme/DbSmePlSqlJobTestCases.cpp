#include "DbSmePlSqlJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

DbSmeTestRecord* DbSmePlSqlJobTestCases::newDefInput()
{
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setInt8(8);
	rec->setInt16(16);
	rec->setInt32(32);
	rec->setInt64(64);
	rec->setFloat(16.16);
	rec->setDouble(32.32);
	rec->setLongDouble(64.64);
	rec->setString("xxx");
	rec->setDate(time(NULL));
	return rec;
}

const DateFormatter& DbSmePlSqlJobTestCases::getDateFormatter()
{
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	return df;
}

DbSmeTestRecord* DbSmePlSqlJobTestCases::createDefaultsProcedureJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.procedure"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("ProcedureJob");
	rec->setDefInput(newDefInput());
	__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
	return rec;
}

DbSmeTestRecord* DbSmePlSqlJobTestCases::createProcedureJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.procedure"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("ProcedureJob");
	rec->setDefInput(newDefInput());
	setRandomInputInt8(rec);
	setRandomInputInt16(rec);
	setRandomInputInt32(rec);
	setRandomInputInt64(rec);
	setRandomInputFloat(rec);
	setRandomInputDouble(rec);
	setRandomInputLongDouble(rec);
	setRandomInputString(rec, rand0(1));
	setRandomInputDate(rec);
	return rec;
}

DbSmeTestRecord* DbSmePlSqlJobTestCases::createDefaultsFunctionJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.function"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("FunctionJob");
	rec->setDefInput(newDefInput());
	__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
	return rec;
}

DbSmeTestRecord* DbSmePlSqlJobTestCases::createFunctionJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.function"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("FunctionJob");
	rec->setDefInput(newDefInput());
	setRandomInputInt8(rec);
	setRandomInputInt16(rec);
	setRandomInputInt32(rec);
	setRandomInputInt64(rec);
	setRandomInputFloat(rec);
	setRandomInputDouble(rec);
	setRandomInputLongDouble(rec);
	setRandomInputString(rec, rand0(1));
	setRandomInputDate(rec);
	return rec;
}

DbSmeTestRecord* DbSmePlSqlJobTestCases::createExceptionJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.function"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("ExceptionJob");
	rec->setId(rand1(2));
	return rec;
}

#define __plsql__(name, val) \
	if (rec->check##name()) { \
		rec->set##name(rec->get##name() + val); \
	} else if (rec->checkDefInput() && rec->getDefInput()->check##name()) { \
		rec->set##name(rec->getDefInput()->get##name() + val); \
	}

const string DbSmePlSqlJobTestCases::processProcedureJobOutput(const string& text,
	DbSmeTestRecord* rec)
{
	__require__(rec);
	__decl_tc__;
	__tc__("processDbSmeRes.procedure"); __tc_ok__;
	
	__plsql__(Int8, 1);
	__plsql__(Int16, 1);
	__plsql__(Int32, 1);
	__plsql__(Int64, 1);
	__plsql__(Float, 1.1);
	__plsql__(Double, 1.1);
	__plsql__(LongDouble, 1.1);
	__plsql__(String, "!!!");
	__plsql__(QuotedString, "!!!");
	__plsql__(Date, 3600);
	
	static const FloatFormatter ff;
	static const DoubleFormatter df1(3, true);
	static const DoubleFormatter df2(2, false);
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	
	ostringstream s;
	bool res = true;
	s << "ProcedureJob" << lineSeparator;
	s << "float: " << getOutputFloat(rec, NULL, ff, res) << lineSeparator;
	s << "double: " << getOutputDouble(rec, NULL, df1, res) << lineSeparator;
	s << "long-double: " << getOutputLongDouble(rec, NULL, df2, res) << lineSeparator;
	s << "date: " << getOutputDate(rec, NULL, df, res) << lineSeparator;
	s << "string: " << getOutputString(rec, NULL, res) << lineSeparator;
	s << "int8: " << getOutputInt8(rec, NULL, res) << lineSeparator;
	s << "int16: " << getOutputInt16(rec, NULL, res) << lineSeparator;
	s << "int32: " << getOutputInt32(rec, NULL, res) << lineSeparator;
    s << "int64: " << getOutputInt64(rec, NULL, res) << lineSeparator;
	return s.str();
}

const string DbSmePlSqlJobTestCases::processFunctionJobOutput(const string& text,
	DbSmeTestRecord* rec)
{
	__require__(rec);
	__decl_tc__;
	__tc__("processDbSmeRes.function"); __tc_ok__;

	__plsql__(Int8, 1);
	__plsql__(Int16, 1);
	__plsql__(Int32, 1);
	__plsql__(Int64, 1);
	__plsql__(Float, 1.1);
	__plsql__(Double, 1.1);
	__plsql__(LongDouble, 1.1);
	__plsql__(String, "!!!");
	__plsql__(QuotedString, "!!!");
	__plsql__(Date, 3600);
	
	static const FloatFormatter ff;
	static const DoubleFormatter df1(3, true);
	static const DoubleFormatter df2(2, false);
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	
	ostringstream s;
	bool res = true;
	s << "FunctionJob" << lineSeparator;
	s << "return: 5" << lineSeparator;
	s << "date: " << getOutputDate(rec, NULL, df, res) << lineSeparator;
	s << "string: " << getOutputString(rec, NULL, res) << lineSeparator;
	s << "uint8: " << getOutputUint8(rec, NULL, res) << lineSeparator;
	s << "uint16: " << getOutputUint16(rec, NULL, res) << lineSeparator;
	s << "uint32: " << getOutputUint32(rec, NULL, res) << lineSeparator;
    s << "uint64: " << getOutputUint64(rec, NULL, res) << lineSeparator;
	s << "float: " << getOutputFloat(rec, NULL, ff, res) << lineSeparator;
	s << "double: " << getOutputDouble(rec, NULL, df1, res) << lineSeparator;
	s << "long-double: " << getOutputLongDouble(rec, NULL, df2, res) << lineSeparator;
	return s.str();
}

const string DbSmePlSqlJobTestCases::processExceptionJobOutput(const string& text,
	DbSmeTestRecord* rec)
{
	__decl_tc__;
	__tc__("processDbSmeRes.exception"); __tc_ok__;
	__cfg_str__(dbSmeRespDsFailure);
	return dbSmeRespDsFailure;
}

const string DbSmePlSqlJobTestCases::processJobFirstOutput(const string& text,
	DbSmeTestRecord* rec)
{
	if (rec->getJob() == "ProcedureJob")
	{
		return processProcedureJobOutput(text, rec);
	}
	else if (rec->getJob() == "FunctionJob")
	{
		return processFunctionJobOutput(text, rec);
	}
	else if (rec->getJob() == "ExceptionJob")
	{
		return processExceptionJobOutput(text, rec);
	}
	__unreachable__("Invalid pl/sql job");
}

}
}
}

