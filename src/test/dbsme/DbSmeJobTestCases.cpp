#include "DbSmeJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using namespace smsc::test::util;

DbSmeJobTestCases::DbSmeJobTestCases(DbSmeRegistry* _dbSmeReg, CheckList* _chkList)
: dbSmeReg(_dbSmeReg), chkList(_chkList)
{
	//__require__(dbSmeReg);
	__require__(chkList);
}

#define __trace_job__(monitor, match, pos, output, expected) \
	__trace2__("db sme cmd: match = %s, input:\n%s\noutput(%d):\n%s\nexpected:\n%s\n", \
		match ? "true" : "false", monitor->pduData->strProps["input"].c_str(), pos, output.c_str(), expected.c_str())

void DbSmeJobTestCases::processJobOutput(const string& text, DbSmeTestRecord* rec,
	SmeAckMonitor* monitor)
{
	__require__(rec && monitor);
	__require__(dbSmeReg);
	__decl_tc__;
	if (!monitor->pduData->strProps.count("output"))
	{
		monitor->pduData->strProps["output"] =
			processJobFirstOutput(text, rec);
	}
	const string& expected = monitor->pduData->strProps["output"];
	//ошибки при обработке
	if (!expected.length())
	{
		monitor->setReceived();
		return;
	}
	int pos = monitor->pduData->intProps.count("output") ?
		monitor->pduData->intProps["output"] : 0;
	bool match = !expected.compare(pos, text.length(), text);
	__trace_job__(monitor, match, pos, text, expected);
	__tc__("processDbSmeRes.output");
	if (match)
	{
		pos += text.length();
		monitor->pduData->intProps["output"] = pos;
		if (pos == expected.length())
		{
			monitor->setReceived();
			//delete rec;
		}
		//pdu порезана на куски по 200 байт
		else if (text.length() != 200)
		{
			__tc_fail__(1);
		}
	}
	else
	{
		__tc_fail__(2);
		monitor->setReceived();
		//delete rec;
	}
	__tc_ok_cond__;
}

void DbSmeJobTestCases::setInputInt16(DbSmeTestRecord* rec, int val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.int"); __tc_ok__;
	rec->setInt16(val);
}

void DbSmeJobTestCases::setRandomInputInt16(DbSmeTestRecord* rec)
{
	setInputInt16(rec, rand2(-32000, 32000));
}

void DbSmeJobTestCases::setInputInt32(DbSmeTestRecord* rec, int val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.int"); __tc_ok__;
	rec->setInt32(val);
}

void DbSmeJobTestCases::setRandomInputInt32(DbSmeTestRecord* rec)
{
	setInputInt32(rec, rand2(INT_MIN/2, INT_MAX/2));
}

void DbSmeJobTestCases::setInputFloat(DbSmeTestRecord* rec, double val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.float"); __tc_ok__;
	rec->setFloat(val);
}

void DbSmeJobTestCases::setRandomInputFloat(DbSmeTestRecord* rec)
{
	setInputFloat(rec, rand2(-100, 100) + (float) rand0(99) / 100.0);
}

void DbSmeJobTestCases::setInputDouble(DbSmeTestRecord* rec, double val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.float"); __tc_ok__;
	rec->setDouble(val);
}

void DbSmeJobTestCases::setRandomInputDouble(DbSmeTestRecord* rec)
{
	setInputDouble(rec, rand2(-100, 100) + (double) rand0(99) / 100.0);
}

void DbSmeJobTestCases::setInputDate(DbSmeTestRecord* rec, time_t val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.date"); __tc_ok__;
	rec->setDate(val);
}

void DbSmeJobTestCases::setRandomInputDate(DbSmeTestRecord* rec)
{
	static const int year = 365 * 24 * 3600;
	setInputDate(rec, time(NULL) + rand2(-year, year));
}

void DbSmeJobTestCases::setInputString(DbSmeTestRecord* rec, const string& val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.string"); __tc_ok__;
	rec->setString(val);
}

void DbSmeJobTestCases::setInputQuotedString(DbSmeTestRecord* rec, const string& val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.quotedString"); __tc_ok__;
	rec->setQuotedString(val);
}

void DbSmeJobTestCases::setRandomInputString(DbSmeTestRecord* rec, bool quotedString)
{
	__decl_tc__;
	int len = rand1(100);
	char str[len + 1];
	rand_char(len, str);
	if (quotedString)
	{
		for (int i = 0; i < len / 5; i++)
		{
			str[rand0(len - 1)] = ' ';
		}
		setInputQuotedString(rec, str);
	}
	else
	{
		setInputString(rec, str);
	}
}

const string DbSmeJobTestCases::getOutputFromAddress(const DbSmeTestRecord* rec)
{
	__require__(rec);
	__decl_tc__;
	__tc__("processDbSmeRes.output.fromAddress"); __tc_ok__;
	return rec->getFromAddr();
}

const string DbSmeJobTestCases::getOutputString(const DbSmeTestRecord* rec,
	const DbSmeTestRecord* defOutput, bool& res)
{
	__decl_tc__;
	//res = true;
	if (rec)
	{
		if (rec->checkString())
		{
			__tc__("processDbSmeRes.input.string"); __tc_ok__;
			return rec->getString();
		}
		if (rec->checkQuotedString())
		{
			__tc__("processDbSmeRes.input.string"); __tc_ok__;
			return rec->getQuotedString();
		}
		if (rec->defInput)
		{
			if (rec->defInput->checkString())
			{
				__tc__("processDbSmeRes.defaultInput.string"); __tc_ok__;
				return rec->defInput->getString();
			}
			if (rec->defInput->checkQuotedString())
			{
				__tc__("processDbSmeRes.defaultInput.string"); __tc_ok__;
				return rec->defInput->getQuotedString();
			}
		}
	}
	if (defOutput)
	{
		if (defOutput->checkString())
		{
			__tc__("processDbSmeRes.select.defaultOutput.string"); __tc_ok__;
			return defOutput->getString();
		}
		if (defOutput->checkQuotedString())
		{
			__tc__("processDbSmeRes.select.defaultOutput.string"); __tc_ok__;
			return defOutput->getQuotedString();
		}
	}
	res = false;
	return "";
}

time_t DbSmeJobTestCases::getDate(DateType dtType, time_t now)
{
	if (dtType == DT_NOW)
	{
		__require__(now);
		return now;
	}
	time_t _t = time(NULL);
	tm lt;
	localtime_r(&_t, &lt);
	lt.tm_sec = 0;
	lt.tm_min = 0;
	lt.tm_hour = 0;
	switch (dtType)
	{
		case DT_TODAY:
			break;
		case DT_YESTERDAY:
			lt.tm_mday -= 1;
			break;
		case DT_TOMORROW:
			lt.tm_mday += 1;
			break;
		default:
			__unreachable__("Invalid dtType");

	}
	return mktime(&lt);
}

const string DbSmeJobTestCases::getOutputDate(const DbSmeTestRecord* rec,
	const DbSmeTestRecord* defOutput, const DateFormatter& df, bool& res)
{
	__decl_tc__;
	//res = true;
	if (rec)
	{
		__tc__("processDbSmeRes.output.dateFormat"); __tc_ok__;
		if (rec->checkDate())
		{
			__tc__("processDbSmeRes.input.date"); __tc_ok__;
			return df.format(rec->getDate());
		}
		if (rec->defInput && rec->defInput->checkDate())
		{
			__require__(rec->defInput->checkDateType());
			switch (rec->defInput->getDateType())
			{
				case DT_NOW:
					__tc__("processDbSmeRes.defaultInput.now"); __tc_ok__;
					break;
				case DT_TODAY:
					__tc__("processDbSmeRes.defaultInput.today"); __tc_ok__;
					break;
				case DT_YESTERDAY:
					__tc__("processDbSmeRes.defaultInput.yesterday"); __tc_ok__;
					break;
				case DT_TOMORROW:
					__tc__("processDbSmeRes.defaultInput.tomorrow"); __tc_ok__;
					break;
				default:
					__unreachable__("Invalid dtType");
			}
			return df.format(rec->defInput->getDate());
		}
	}
	if (defOutput && defOutput->checkDate())
	{
		__tc__("processDbSmeRes.output.dateFormat"); __tc_ok__;
		__require__(defOutput->checkDateType());
		switch (defOutput->getDateType())
		{
			case DT_NOW:
				__tc__("processDbSmeRes.select.defaultOutput.now"); __tc_ok__;
				break;
			case DT_TODAY:
				__tc__("processDbSmeRes.select.defaultOutput.today"); __tc_ok__;
				break;
			case DT_YESTERDAY:
				__tc__("processDbSmeRes.select.defaultOutput.yesterday"); __tc_ok__;
				break;
			case DT_TOMORROW:
				__tc__("processDbSmeRes.select.defaultOutput.tomorrow"); __tc_ok__;
				break;
			default:
				__unreachable__("Invalid dtType");
		}
		return df.format(defOutput->getDate());
	}
	res = false;
	return "";
}

int DbSmeJobTestCases::getOutputInt16(const DbSmeTestRecord* rec,
	const DbSmeTestRecord* defOutput, bool& res)
{
	__decl_tc__;
	//res = true;
	if (rec)
	{
		if (rec->checkInt16())
		{
			__tc__("processDbSmeRes.input.int"); __tc_ok__;
			return rec->getInt16();
		}
		if (rec->defInput && rec->defInput->checkInt16())
		{
			__tc__("processDbSmeRes.defaultInput.int"); __tc_ok__;
			return rec->defInput->getInt16();
		}
	}
	if (defOutput && defOutput->checkInt16())
	{
		__tc__("processDbSmeRes.select.defaultOutput.int"); __tc_ok__;
		return defOutput->getInt16();
	}
	res = false;
	return 0;
}

int DbSmeJobTestCases::getOutputInt32(const DbSmeTestRecord* rec,
	const DbSmeTestRecord* defOutput, bool& res)
{
	__decl_tc__;
	//res = true;
	if (rec)
	{
		if (rec->checkInt32())
		{
			__tc__("processDbSmeRes.input.int"); __tc_ok__;
			return rec->getInt32();
		}
		if (rec->defInput && rec->defInput->checkInt32())
		{
			__tc__("processDbSmeRes.defaultInput.int"); __tc_ok__;
			return rec->defInput->getInt32();
		}
	}
	if (defOutput && defOutput->checkInt32())
	{
		__tc__("processDbSmeRes.select.defaultOutput.int"); __tc_ok__;
		return defOutput->getInt32();
	}
	res = false;
	return 0;
}
	
float DbSmeJobTestCases::getOutputFloat(const DbSmeTestRecord* rec,
	const DbSmeTestRecord* defOutput, bool& res)
{
	__decl_tc__;
	//res = true;
	if (rec)
	{
		if (rec->checkFloat())
		{
			__tc__("processDbSmeRes.input.float"); __tc_ok__;
			return rec->getFloat();
		}
		if (rec->defInput && rec->defInput->checkFloat())
		{
			__tc__("processDbSmeRes.defaultInput.float"); __tc_ok__;
			return rec->defInput->getFloat();
		}
	}
	if (defOutput && defOutput->checkFloat())
	{
		__tc__("processDbSmeRes.select.defaultOutput.float"); __tc_ok__;
		return defOutput->getFloat();
	}
	res = false;
	return 0.0;
}

double DbSmeJobTestCases::getOutputDouble(const DbSmeTestRecord* rec,
	const DbSmeTestRecord* defOutput, bool& res)
{
	__decl_tc__;
	//res = true;
	if (rec)
	{
		if (rec->checkDouble())
		{
			__tc__("processDbSmeRes.input.float"); __tc_ok__;
			return rec->getDouble();
		}
		if (rec->defInput && rec->defInput->checkDouble())
		{
			__tc__("processDbSmeRes.defaultInput.float"); __tc_ok__;
			return rec->defInput->getDouble();
		}
	}
	if (defOutput && defOutput->checkDouble())
	{
		__tc__("processDbSmeRes.select.defaultOutput.float"); __tc_ok__;
		return defOutput->getDouble();
	}
	res = false;
	return 0.0;
}

}
}
}

