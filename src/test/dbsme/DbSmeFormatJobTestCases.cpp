#include "DbSmeFormatJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using namespace std;
using namespace smsc::test::util;

DbSmeTestRecord* DbSmeDateFormatJobTestCases::createJobInput(
	int jobNum, bool params, const DateFormatter** df)
{
	__decl_tc__;
	static const DateFormatter df1("d/M/yy h:m:s t");
	static const DateFormatter df2("dd.MM.yyyy hh-mm-ss t");
	static const DateFormatter df3("d-MMM-yy H m s");
	static const DateFormatter df4("dd MMMM yyyy HH.mm.ss");
	
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->defInput = new DbSmeTestRecord();
	switch (jobNum)
	{
		case 1:
			rec->setJob("DateFormatJob1");
			rec->defInput->setDateType(DT_NOW);
			rec->defInput->setDate(time(NULL));
			*df = &df1;
			break;
		case 2:
			rec->setJob("DateFormatJob2");
			rec->defInput->setDateType(DT_TODAY);
			rec->defInput->setDate(getDate(DT_TODAY));
			*df = &df2;
			break;
		case 3:
			rec->setJob("DateFormatJob3");
			rec->defInput->setDateType(DT_YESTERDAY);
			rec->defInput->setDate(getDate(DT_YESTERDAY));
			*df = &df3;
			break;
		case 4:
			rec->setJob("DateFormatJob4");
			rec->defInput->setDateType(DT_TOMORROW);
			rec->defInput->setDate(getDate(DT_TOMORROW));
			*df = &df4;
			break;
		default:
			__unreachable__("Invalid jobNum");
	}
	__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
	if (params)
	{
		setRandomInputDate(rec);
	}
	else
	{
		__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
	}
	return rec;
}

const string DbSmeDateFormatJobTestCases::processJobFirstOutput(const string& text,
	DbSmeTestRecord* rec)
{
	__decl_tc__;
	__tc__("processDbSmeRes.select.singleRecord"); __tc_ok__;
	static const DateFormatter df("!@#$%^&*( )_+-=|\\:;\'<,>.?/wMMMMMMMMwwyyWyyyyWWyyyyyydyyyyyyyyddhdddhhddddhhhMhhhhMMmMMMmmMMMMmmmMMMMMmmmmMMMMMMtMMMMMMMtt");
	static const string prefix("\nDate: ");
	int jobNum;
	if (!sscanf(rec->getJob().c_str(), "DateFormatJob%d", &jobNum))
	{
		return "";
	}
	bool match = false;
	string expected;
	bool res = true;
	if (jobNum == 1 && !rec->checkDate())
	{
		__require__(rec->defInput && rec->defInput->checkDate());
		while (rec->defInput->getDate() <= time(NULL))
		{
			expected = prefix + getOutputDate(rec, NULL, df, res) + "\n";
			__require__(res);
			if (!expected.compare(0, text.length(), text))
			{
				return expected;
			}
			rec->defInput->setDate(rec->defInput->getDate() + 1);
		}
	}
	else
	{
		switch (jobNum)
		{
			case 1: //now
			case 2: //today
			case 3: //yesterday
			case 4: //tomorrow
				break;
			default:
				__unreachable__("Invalid jobNum");
		}
		expected = prefix + getOutputDate(rec, NULL, df, res) + "\n";
		__require__(res);
		return expected;
	}
	return "";
}

DbSmeTestRecord* DbSmeOtherFormatJobTestCases::createJobInput(
	bool params)
{
	__decl_tc__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("OtherFormatJob");
	rec->defInput = new DbSmeTestRecord();
	rec->defInput->setInt16(16);
	rec->defInput->setInt32(32);
	rec->defInput->setFloat(16.16);
	rec->defInput->setDouble(32.32);
	rec->defInput->setString("xxx");
	__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
	if (params)
	{
		setRandomInputInt16(rec);
		setRandomInputInt32(rec);
		setRandomInputFloat(rec);
		setRandomInputDouble(rec);
		setRandomInputString(rec, rand0(1));
	}
	else
	{
		__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
	}
	return rec;
}

const string DbSmeOtherFormatJobTestCases::processJobFirstOutput(const string& text,
	DbSmeTestRecord* rec)
{
	__require__(rec);
	__decl_tc__;
	__tc__("processDbSmeRes.select.singleRecord"); __tc_ok__;
	bool res = true;
	ostringstream os;
	os << endl << "from-address: '" << getOutputFromAddress(rec) << "';" << endl;
	string str = getOutputString(rec, NULL, res);
	os << "string: \"" << str << "\"," << endl;
	__tc__("processDbSmeRes.output.string.left"); __tc_ok__;
	__tc__("processDbSmeRes.output.string.right"); __tc_ok__;
	__tc__("processDbSmeRes.output.string.center"); __tc_ok__;
	if (str.length() < 30)
	{
		os << "string_right : " << string(30 - str.length(), ' ') << str << "," << endl;
		int tmp = (30 - str.length()) / 2;
		os << "string_center: " << string(tmp, ' ') << str <<
			string(30 - tmp - str.length(), ' ') << "," << endl;
		os << "string_left  : " << str << string(30 - str.length(), ' ') << ";" << endl;
	}
	else
	{
		os << "string_right : " << str << "," << endl;
		os << "string_center: " << str << "," << endl;
		os << "string_left  : " << str << ";" << endl;
	}
	os << "int16: (--$" << getOutputInt16(rec, NULL, res) << "--)," << endl;
	os << "int32: (++$" << getOutputInt32(rec, NULL, res) << "++)," << endl;
	os << "float: (**$" << getOutputFloat(rec, NULL, res) << "**)," << endl;
	os << "double: (::$" << getOutputDouble(rec, NULL, res) << "::);" << endl;
	__require__(res);
	return os.str();
}

}
}
}

