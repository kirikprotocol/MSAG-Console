#include "DbSmeModifyJobTestCases.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using namespace std;
using namespace smsc::test::util;

DbSmeTestRecord* DbSmeInsertJobTestCases::newDefInput1()
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
	rec->setDateType(DT_TODAY);
	rec->setDate(getDate(DT_TODAY));
	return rec;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::newDefInput2()
{
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setInt8(7);
	rec->setInt16(15);
	rec->setInt32(31);
	rec->setInt64(63);
	rec->setFloat(15.15);
	rec->setDouble(31.31);
	rec->setLongDouble(63.63);
	rec->setString("yyy");
	rec->setDateType(DT_TODAY);
	rec->setDate(getDate(DT_TODAY));
	return rec;
}

const DateFormatter& DbSmeInsertJobTestCases::getDateFormatter()
{
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	return df;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createIntsJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("InsertJob1");
	rec->setId(dbSmeReg->nextId());
	rec->setDefInput(newDefInput1());
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

DbSmeTestRecord* DbSmeInsertJobTestCases::createUintsJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("InsertJob2");
	rec->setId(dbSmeReg->nextId());
	rec->setDefInput(newDefInput2());
	setRandomInputUint8(rec);
	setRandomInputUint16(rec);
	setRandomInputUint32(rec);
	setRandomInputUint64(rec);
	setRandomInputFloat(rec);
	setRandomInputDouble(rec);
	setRandomInputLongDouble(rec);
	setRandomInputString(rec, rand0(1));
	setRandomInputDate(rec);
	return rec;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createIntDefaultsJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("InsertJob1");
	rec->setId(dbSmeReg->nextId());
	rec->setDefInput(newDefInput1());
	__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
	return rec;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createUintDefaultsJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("InsertJob2");
	rec->setId(dbSmeReg->nextId());
	rec->setDefInput(newDefInput1());
	__tc__("submitDbSmeCmd.correct.defaultInput.uint"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
	return rec;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createZerosJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("InsertJob1");
	rec->setId(dbSmeReg->nextId());
	rec->setDefInput(newDefInput1());
	setInputInt8(rec, 0);
	setInputInt16(rec, 0);
	setInputInt32(rec, 0);
	setInputInt64(rec, 0);
	setInputFloat(rec, 0.0);
	setInputDouble(rec, 0.0);
	setInputLongDouble(rec, 0.0);
	setInputQuotedString(rec, "");
	setInputDate(rec, 0);
	return rec;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createNullsJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("InsertJob3");
	rec->setId(dbSmeReg->nextId());
	return rec;
}

DbSmeTestRecord* DbSmeInsertJobTestCases::createDuplicateKeyJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("InsertJob3");
	rec->setId(dbSmeReg->getExistentId());
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
	}
	else if (rec->getJob() == "InsertJob1")
	{
		ostringstream os;
		bool res = true;
		__tc__("processDbSmeRes.insert.ok"); __tc_ok__;
		static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
		os << endl << "InsertJob1:" << endl;
		os << "string: " << getOutputString(rec, NULL, res) << endl;
		os << "date: " << getOutputDate(rec, NULL, df, res) << endl;
		os << "float: " << getOutputFloat(rec, NULL, res) << endl;
		os << "double: " << getOutputDouble(rec, NULL, res) << endl;
		os << "long-double: " << getOutputLongDouble(rec, NULL, res) << endl;
		os << "int8: " << getOutputInt8(rec, NULL, res) << endl;
		os << "int16: " << getOutputInt16(rec, NULL, res) << endl;
		os << "int32: " << getOutputInt32(rec, NULL, res) << endl;
		os << "int64: " << getOutputInt64(rec, NULL, res) << endl;
		os << "id: " << rec->getId() << endl;
		os << "rows-affected: 1" << endl;
		__require__(res);
		dbSmeReg->putRecord(rec);
		return os.str();
	}
	else if (rec->getJob() == "InsertJob2")
	{
		ostringstream os;
		bool res = true;
		__tc__("processDbSmeRes.insert.ok"); __tc_ok__;
		static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
		os << endl << "InsertJob2:" << endl;
		os << "string: " << getOutputString(rec, NULL, res) << endl;
		os << "date: " << getOutputDate(rec, NULL, df, res) << endl;
		os << "float: " << getOutputFloat(rec, NULL, res) << endl;
		os << "double: " << getOutputDouble(rec, NULL, res) << endl;
		os << "long-double: " << getOutputLongDouble(rec, NULL, res) << endl;
		os << "uint8: " << getOutputInt8(rec, NULL, res) << endl;
		os << "uint16: " << getOutputInt16(rec, NULL, res) << endl;
		os << "uint32: " << getOutputInt32(rec, NULL, res) << endl;
		os << "uint64: " << getOutputInt64(rec, NULL, res) << endl;
		os << "id: " << rec->getId() << endl;
		os << "rows-affected: 1" << endl;
		__require__(res);
		dbSmeReg->putRecord(rec);
		return os.str();
	}
	else if (rec->getJob() == "InsertJob3")
	{
		ostringstream os;
		bool res = true;
		__tc__("processDbSmeRes.insert.ok"); __tc_ok__;
		os << endl << "InsertJob3:" << endl;
		os << "id: " << rec->getId() << endl;
		os << "rows-affected: 1" << endl;
		__require__(res);
		dbSmeReg->putRecord(rec);
		return os.str();
	}
	return "";
}

const DateFormatter& DbSmeUpdateJobTestCases::getDateFormatter()
{
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	return df;
}

DbSmeTestRecord* DbSmeUpdateJobTestCases::createUpdateRecordJobInput()
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.update"); __tc_ok__;
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setJob("UpdateJob1");
	rec->setId(dbSmeReg->getExistentId());
	__tc__("submitDbSmeCmd.correct.input.jobName"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.input.toAddress"); __tc_ok__;
	__tc__("submitDbSmeCmd.correct.input.fromAddress"); __tc_ok__;
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
		__tc__("processDbSmeRes.input.jobName"); __tc_ok__;
		__tc__("processDbSmeRes.input.toAddress"); __tc_ok__;
		__tc__("processDbSmeRes.input.fromAddress"); __tc_ok__;
		bool res = true;
		ostringstream os;
		os << endl << "UpdateJob1:" << endl;
		os << "job-name: " << getOutputJobName(rec) << endl;
		os << "from-address: " << getOutputFromAddress(rec) << endl;
		os << "to-address: " << getOutputToAddress(rec) << endl;
		int rowsAffected = 0;
		__require__(rec->checkId());
		DbSmeTestRecord* r = dbSmeReg->getRecord(rec->getId());
		if (r)
		{
			rowsAffected = 1;
			r->setString("job " + rec->getJob() +
				" from " + rec->getFromAddr() +
				" to " + rec->getToAddr());
		}
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
				if (rec->getId() != r->getId())
				{
					dbSmeReg->updateRecord(rec->getId(), r);
				}
			}
			delete it;
			__tc__("processDbSmeRes.update.recordsAffected"); __tc_ok__;
			os << "rows-affected: " << dbSmeReg->size() << endl;
			return os.str();
		}
	}
	return "";
}

DbSmeTestRecord* DbSmeDeleteJobTestCases::createDeleteAllJobInput()
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

