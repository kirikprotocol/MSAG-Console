#include "DbSmeSelectJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using namespace std;
using namespace smsc::test::util;

DbSmeTestRecord* DbSmeSelectJobTestCases::createSelectNullsJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("SelectJob1");
	return rec;
}

DbSmeTestRecord* DbSmeSelectJobTestCases::createSelectValuesJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("SelectJob2");
	return rec;
}

DbSmeTestRecord* DbSmeSelectJobTestCases::createSelectNoDefaultsJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("SelectJob2");
	return rec;
}

void DbSmeSelectJobTestCases::writeSelectJobRecord(ostream& os,
	DbSmeTestRecord* rec, DbSmeTestRecord* defOutput, time_t now)
{
	static const DateFormatter df1("d/M/yy h:m:s t");
	static const DateFormatter df2("dd.MM.yyyy hh-mm-ss t");
	static const DateFormatter df3("d-MMM-yy H m s");
	static const DateFormatter df4("dd MMMM yyyy HH.mm.ss");

	bool res = true;
	os << endl << "SelectJob:" << endl;
	defOutput->setDateType(DT_NOW);
	defOutput->setDate(now);
	os << getOutputDate(rec, defOutput, df1, res) << endl;
	defOutput->setDateType(DT_TODAY);
	defOutput->setDate(getDate(DT_TODAY));
	os << getOutputDate(rec, defOutput, df2, res) << endl;
	defOutput->setDateType(DT_YESTERDAY);
	defOutput->setDate(getDate(DT_YESTERDAY));
	os << getOutputDate(rec, defOutput, df3, res) << endl;
	defOutput->setDateType(DT_TOMORROW);
	defOutput->setDate(getDate(DT_TOMORROW));
	os << getOutputDate(rec, defOutput, df4, res) << endl;

	os << getOutputInt8(rec, defOutput, res) << endl;
	os << getOutputInt16(rec, defOutput, res) << endl;
	os << getOutputInt32(rec, defOutput, res) << endl;
	os << getOutputInt64(rec, defOutput, res) << endl;
	os << getOutputFloat(rec, defOutput, res) << endl;
	os << getOutputDouble(rec, defOutput, res) << endl;
	os << getOutputLongDouble(rec, defOutput, res) << endl;
	os << getOutputString(rec, defOutput, res) << endl;
	__require__(res);
}

const string DbSmeSelectJobTestCases::processSelectNullsJobOutput(
	const string& text)
{
	__require__(dbSmeReg);
	DbSmeTestRecord defOutput;
	defOutput.setInt8(80);
	defOutput.setInt16(160);
	defOutput.setInt32(320);
	defOutput.setInt64(640);
	defOutput.setFloat(160.16);
	defOutput.setDouble(320.32);
	defOutput.setLongDouble(640.64);
	defOutput.setString("Empty");
	//defOutput.setDate(...);
	__cfg_int__(timeCheckAccuracy);
	for (time_t t = time(NULL); t >= time(NULL) - timeCheckAccuracy; t--)
	{
		ostringstream os;
		DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
		int count = 0;
		while (DbSmeTestRecord* r = it->next())
		{
			__require__(r->checkId());
			if (!r->checkDate() &&
				!(r->getDefInput() && r->getDefInput()->checkDate()))
			{
				count++;
				writeSelectJobRecord(os, r, &defOutput, t);
			}
		}
		delete it;
		if (!count)
		{
			//нет записей удовлетворяющих условиям
			writeSelectJobRecord(os, NULL, &defOutput, t);
		}
		string expected = os.str();
		if (!expected.compare(0, text.length(), text))
		{
			return expected;
		}
	}
	return "";
}

const string DbSmeSelectJobTestCases::processSelectValuesJobOutput()
{
	__require__(dbSmeReg);
	DbSmeTestRecord defOutput;
	defOutput.setInt8(80);
	defOutput.setInt16(160);
	defOutput.setInt32(320);
	defOutput.setInt64(640);
	defOutput.setFloat(160.16);
	defOutput.setDouble(320.32);
	defOutput.setLongDouble(640.64);
	defOutput.setString("Empty");
	//defOutput.setDate(...);
	ostringstream os;
	DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
	int count = 0;
	while (DbSmeTestRecord* r = it->next())
	{
		__require__(r->checkId());
		if (r->checkDate() ||
			(r->getDefInput() && r->getDefInput()->checkDate()))
		{
			count++;
			writeSelectJobRecord(os, r, &defOutput, 0);
		}
	}
	delete it;
	if (!count)
	{
		//нет записей удовлетворяющих условиям
		writeSelectJobRecord(os, NULL, &defOutput, 0);
	}
	return os.str();
}

const string DbSmeSelectJobTestCases::processSelectNoDefaultsJobOutput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream os;
	bool res = true;
	static const DateFormatter df("dd.MM.yyyy hh-mm-ss t");
	DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
	int count = 0;
	while (DbSmeTestRecord* r = it->next())
	{
		__require__(r->checkId());
		count++;
		os << endl << "SelectJob:" << endl;
		os << getOutputDate(r, NULL, df, res) << endl;
		os << getOutputInt8(r, NULL, res) << endl;
		os << getOutputInt16(r, NULL, res) << endl;
		os << getOutputInt32(r, NULL, res) << endl;
		os << getOutputInt64(r, NULL, res) << endl;
		os << getOutputFloat(r, NULL, res) << endl;
		os << getOutputDouble(r, NULL, res) << endl;
		os << getOutputLongDouble(r, NULL, res) << endl;
		os << getOutputString(r, NULL, res) << endl;
		if (!res)
		{
			break;
		}
	}
	delete it;
	if (!res)
	{
		__tc__("processDbSmeRes.select.nullsNoDefaults"); __tc_ok__;
		return "No default";
	}
	else if	(!count)
	{
		__tc__("processDbSmeRes.select.noRecordNoDefaults"); __tc_ok__;
		return "No records, no defaults";
	}
	return os.str();
}

const string DbSmeSelectJobTestCases::processJobFirstOutput(const string& text,
	DbSmeTestRecord* rec)
{
	if (rec->getJob() == "SelectJob1")
	{
		return processSelectNullsJobOutput(text);
	}
	else if (rec->getJob() == "SelectJob2")
	{
		return processSelectValuesJobOutput();
	}
	else if (rec->getJob() == "SelectJob3")
	{
		return processSelectNoDefaultsJobOutput();
	}
	return "";
}

}
}
}

