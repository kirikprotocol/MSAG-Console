#include "DbSmeModifyJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using namespace std;
using namespace smsc::test::util;

const DateFormatter& DbSmeInsertJobTestCases::getDateFormatter()
{
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	return df;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createDefaultInput()
{
	__decl_tc__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("InsertJob");
	rec->defInput = new DbSmeTestRecord();
	rec->defInput->setInt16(16);
	rec->defInput->setInt32(32);
	rec->defInput->setFloat(16.16);
	rec->defInput->setDouble(32.32);
	rec->defInput->setString("xxx");
	rec->defInput->setDateType(DT_TODAY);
	rec->defInput->setDate(getDate(DT_TODAY));
	__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
	return rec;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createJobInput(bool params)
{
	__require__(dbSmeReg);
	__decl_tc__;
	DbSmeTestRecord* rec = createDefaultInput();
	rec->setId(dbSmeReg->nextId());
	if (params)
	{
		setRandomInputInt16(rec);
		setRandomInputInt32(rec);
		setRandomInputFloat(rec);
		setRandomInputDouble(rec);
		setRandomInputString(rec, rand0(1));
		setRandomInputDate(rec);
	}
	else
	{
		__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
	}
	return rec;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createDuplicateKeyJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	DbSmeTestRecord* rec = createDefaultInput();
	rec->setId(dbSmeReg->getExistentId());
	__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
	return rec;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createZerosJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	DbSmeTestRecord* rec = createDefaultInput();
	rec->setId(dbSmeReg->nextId());
	setInputInt16(rec, 0);
	setInputInt32(rec, 0);
	setInputFloat(rec, 0.0);
	setInputDouble(rec, 0.0);
	setInputQuotedString(rec, "");
	setInputDate(rec, 0);
	return rec;
}

const string DbSmeInsertJobTestCases::processJobFirstOutput(const string& text,
	DbSmeTestRecord* rec)
{
	__require__(rec && rec->checkId());
	__require__(dbSmeReg);
	__decl_tc__;
	if (dbSmeReg->getRecord(rec->getId()))
	{
		__tc__("processDbSmeRes.insert.duplicateKey"); __tc_ok__;
		return "Duplicate key";
		//delete rec;
	}
	else
	{
		ostringstream os;
		bool res = true;
		__tc__("processDbSmeRes.insert.ok"); __tc_ok__;
		static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
		os << endl << "InsertJob:" << endl;
		os << "string: " << getOutputString(rec, NULL, res) << endl;
		os << "date: " << getOutputDate(rec, NULL, df, res) << endl;
		os << "int16: " << getOutputInt16(rec, NULL, res) << endl;
		os << "int32: " << getOutputInt32(rec, NULL, res) << endl;
		os << "float: " << getOutputFloat(rec, NULL, res) << endl;
		os << "double: " << getOutputDouble(rec, NULL, res) << endl;
		os << "from-address: " << getOutputFromAddress(rec) << endl;
		os << "id: " << rec->getId() << endl;
		os << "rows-affected: 1" << endl;
		__require__(res);
		dbSmeReg->putRecord(rec);
		return os.str();
	}
}

const DateFormatter& DbSmeUpdateJobTestCases::getDateFormatter()
{
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	return df;
}

DbSmeTestRecord* DbSmeUpdateJobTestCases::createJobInput(bool params)
{
	__decl_tc__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->defInput = new DbSmeTestRecord();
	rec->defInput->setInt16(17);
	rec->defInput->setInt32(33);
	rec->defInput->setFloat(17.17);
	rec->defInput->setDouble(33.33);
	rec->defInput->setString("zzz");
	rec->defInput->setDateType(DT_TODAY);
	rec->defInput->setDate(getDate(DT_TODAY));
	__tc__("submitDbSmeCmd.correct.job.update"); __tc_ok__;
	rec->setJob("UpdateJob1");
	if (params)
	{
		setRandomInputInt16(rec);
		setRandomInputInt32(rec);
		setRandomInputFloat(rec);
		setRandomInputDouble(rec);
		setRandomInputString(rec, rand0(1));
		setRandomInputDate(rec);
	}
	else
	{
		__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
		__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
	}
	return rec;
}

DbSmeTestRecord* DbSmeUpdateJobTestCases::createDuplicateKeyJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.update"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("UpdateJob2");
	rec->setId(dbSmeReg->getExistentId());
	return rec;
}

const string DbSmeUpdateJobTestCases::processJobFirstOutput(const string& text,
	DbSmeTestRecord* rec)
{
	__require__(dbSmeReg);
	__decl_tc__;
	if (rec->getJob() == "UpdateJob1")
	{
		__tc__("processDbSmeRes.update.ok"); __tc_ok__;
		bool res = true;
		ostringstream os;
		os << endl << "UpdateJob1:" << endl;
		os << "from-address: " << getOutputFromAddress(rec) << endl;
		os << "string: " << getOutputString(rec, NULL, res) << endl;
		static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
		os << "date: " << getOutputDate(rec, NULL, df, res) << endl;
		os << "int16: " << getOutputInt16(rec, NULL, res) << endl;
		os << "int32: " << getOutputInt32(rec, NULL, res) << endl;
		os << "float: " << getOutputFloat(rec, NULL, res) << endl;
		os << "double: " << getOutputDouble(rec, NULL, res) << endl;
		int rowsAffected = 0;
		DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
		while (DbSmeTestRecord* r = it->next())
		{
			__require__(r->checkId());
			int id = r->getId();
			rowsAffected++;
			//обновить запись
			*r = *rec;
			r->setId(id);
		}
		delete it;
		__tc__("processDbSmeRes.update.recordsAffected"); __tc_ok__;
		os << "rows-affected: " << rowsAffected << endl;
		__require__(res);
		return os.str();
	}
	else if (rec->getJob() == "UpdateJob2")
	{
		if (dbSmeReg->size() > 1)
		{
			__tc__("processDbSmeRes.update.duplicateKey"); __tc_ok__;
			return "Duplicate key";
		}
		else
		{
			__tc__("processDbSmeRes.update.ok"); __tc_ok__;
			ostringstream os;
			os << endl << "UpdateJob2:" << endl;
			os << "id:" << rec->getId() << endl;
			DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
			while (DbSmeTestRecord* r = it->next())
			{
				//обновить id
				dbSmeReg->updateRecord(rec->getId(), r);
			}
			delete it;
			__tc__("processDbSmeRes.update.recordsAffected"); __tc_ok__;
			os << "rows-affected: " << dbSmeReg->size() << endl;
			return os.str();
		}
	}
	return "";
}

DbSmeTestRecord* DbSmeDeleteJobTestCases::createJobInput()
{
	__decl_tc__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	__tc__("submitDbSmeCmd.correct.job.delete"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
	rec->setJob("DeleteJob");
	return rec;
}

const string DbSmeDeleteJobTestCases::processJobFirstOutput(const string& text,
	DbSmeTestRecord* rec)
{
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream os;
	__tc__("processDbSmeRes.delete.ok"); __tc_ok__;
	os << endl << "DeleteJob:" << endl;
	__tc__("processDbSmeRes.delete.recordsAffected"); __tc_ok__;
	os << "rows-affected: " << dbSmeReg->size() << endl;
	dbSmeReg->clear();
	return os.str();
}

}
}
}

