#include "DbSmeSelectJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using namespace std;
using namespace smsc::test::util;

DbSmeTestRecord* DbSmeSelectJobTestCases::createJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("SelectJob");
	return rec;
}

void DbSmeSelectJobTestCases::writeSelectJobRecord(ostream& os,
	DbSmeTestRecord* rec, DbSmeTestRecord* defOutput, time_t now)
{
	static const DateFormatter df1("dd.MM.yyyy hh-mm-ss t");
	static const DateFormatter df2("d-MMM-yy H m s");
	static const DateFormatter df3("dd MMMM yyyy HH.mm.ss");
	static const DateFormatter df4("d/M/yy h:m:s t");

	bool res = true;
	os << endl << "SelectJob:" << endl;
	os << getOutputInt16(rec, defOutput, res) << endl;
	os << getOutputInt32(rec, defOutput, res) << endl;
	os << getOutputFloat(rec, defOutput, res) << endl;
	os << getOutputDouble(rec, defOutput, res) << endl;
	os << getOutputString(rec, defOutput, res) << endl;
	defOutput->setDateType(DT_TODAY);
	defOutput->setDate(getDate(DT_TODAY));
	os << getOutputDate(rec, defOutput, df1, res) << endl;
	defOutput->setDateType(DT_YESTERDAY);
	defOutput->setDate(getDate(DT_YESTERDAY));
	os << getOutputDate(rec, defOutput, df2, res) << endl;
	defOutput->setDateType(DT_TOMORROW);
	defOutput->setDate(getDate(DT_TOMORROW));
	os << getOutputDate(rec, defOutput, df3, res) << endl;
	defOutput->setDateType(DT_NOW);
	defOutput->setDate(now);
	os << getOutputDate(rec, defOutput, df4, res) << endl;
	__require__(res);
}

const string DbSmeSelectJobTestCases::processJobFirstOutput(const string& text,
	DbSmeTestRecord* rec)
{
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream os;
	bool match;
	string expected;
	DbSmeTestRecord defOutput;
	defOutput.setInt16(160);
	defOutput.setInt32(320);
	defOutput.setFloat(160.16);
	defOutput.setDouble(320.32);
	defOutput.setString("Empty");
	//defOutput.setDate(...);
	__cfg_int__(timeCheckAccuracy);
	for (time_t t = time(NULL); t >= time(NULL) - timeCheckAccuracy; t--)
	{
		bool res = true;
		DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
		while (DbSmeTestRecord* r = it->next())
		{
			__require__(r->checkId());
			writeSelectJobRecord(os, r, &defOutput, t);
		}
		delete it;
		expected = os.str();
		if (!expected.length())
		{
			writeSelectJobRecord(os, NULL, &defOutput, t);
		}
		expected = os.str();
		if (!expected.compare(0, text.length(), text))
		{
			return expected;
		}
	}
	return "";
}

DbSmeTestRecord* DbSmeSelectNoDefaultJobTestCases::createJobInput()
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("SelectNoDefaultJob");
	return rec;
}

const string DbSmeSelectNoDefaultJobTestCases::processJobFirstOutput(
	const string& text, DbSmeTestRecord* rec)
{
	__require__(dbSmeReg);
	__decl_tc__;
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	ostringstream os;
	bool res = true;
	DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
	while (DbSmeTestRecord* r = it->next())
	{
		__require__(r->checkId());
		os << endl << "SelectNoDefaultJob:" << endl;
		os << getOutputInt16(r, NULL, res) << endl;
		os << getOutputInt32(r, NULL, res) << endl;
		os << getOutputFloat(r, NULL, res) << endl;
		os << getOutputDouble(r, NULL, res) << endl;
		os << getOutputDate(r, NULL, df, res) << endl;
		os << getOutputString(r, NULL, res) << endl;
		if (!res)
		{
			break;
		}
	}
	delete it;
	return (res ? os.str() : "Invalid format");
}

}
}
}

