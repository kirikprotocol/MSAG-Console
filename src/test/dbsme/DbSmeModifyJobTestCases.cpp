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
	rec->setDefInput(newDefInput2());
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
	static const DateFormatter dateFmt("dd-MM-yyyy HH:mm:ss");
	static const FloatFormatter fltFmt(6);
	static const DoubleFormatter dblFmt(6, false);
	static const DoubleFormatter ldblFmt(6, false);
	if (dbSmeReg->getRecord(rec->getId()))
	{
		__tc__("processDbSmeRes.insert.duplicateKey"); __tc_ok__;
		__cfg_str__(dbSmeRespDsFailure);
		return dbSmeRespDsFailure;
	}
	else if (rec->getJob() == "InsertJob1")
	{
		ostringstream os;
		bool res = true;
		__tc__("processDbSmeRes.insert.ok"); __tc_ok__;
		os << lineSeparator << "InsertJob1:" << lineSeparator;
		os << "string: " << getOutputString(rec, NULL, res) << lineSeparator;
		os << "date: " << getOutputDate(rec, NULL, dateFmt, res) << lineSeparator;
		os << "float: " << getOutputFloat(rec, NULL, fltFmt, res) << lineSeparator;
		os << "double: " << getOutputDouble(rec, NULL, dblFmt, res) << lineSeparator;
		os << "long-double: " << getOutputLongDouble(rec, NULL, ldblFmt, res) << lineSeparator;
		os << "int8: " << (int) getOutputInt8(rec, NULL, res) << lineSeparator;
		os << "int16: " << (int) getOutputInt16(rec, NULL, res) << lineSeparator;
		os << "int32: " << (int) getOutputInt32(rec, NULL, res) << lineSeparator;
		os << "int64: " << (int) getOutputInt64(rec, NULL, res) << lineSeparator;
		os << "id: " << rec->getId() << lineSeparator;
		os << "rows-affected: 1" << lineSeparator;
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
		os << lineSeparator << "InsertJob2:" << lineSeparator;
		os << "string: " << getOutputString(rec, NULL, res) << lineSeparator;
		os << "date: " << getOutputDate(rec, NULL, dateFmt, res) << lineSeparator;
		os << "float: " << getOutputFloat(rec, NULL, fltFmt, res) << lineSeparator;
		os << "double: " << getOutputDouble(rec, NULL, dblFmt, res) << lineSeparator;
		os << "long-double: " << getOutputLongDouble(rec, NULL, ldblFmt, res) << lineSeparator;
		os << "uint8: " << (int) getOutputUint8(rec, NULL, res) << lineSeparator;
		os << "uint16: " << (int) getOutputUint16(rec, NULL, res) << lineSeparator;
		os << "uint32: " << (int) getOutputUint32(rec, NULL, res) << lineSeparator;
		os << "uint64: " << (int) getOutputUint64(rec, NULL, res) << lineSeparator;
		os << "id: " << rec->getId() << lineSeparator;
		os << "rows-affected: 1" << lineSeparator;
		__require__(res);
		dbSmeReg->putRecord(rec);
		return os.str();
	}
	else if (rec->getJob() == "InsertJob3")
	{
		ostringstream os;
		bool res = true;
		__tc__("processDbSmeRes.insert.ok"); __tc_ok__;
		os << lineSeparator << "InsertJob3:" << lineSeparator;
		os << "id: " << rec->getId() << lineSeparator;
		os << "rows-affected: 1" << lineSeparator;
		__require__(res);
		dbSmeReg->putRecord(rec);
		return os.str();
	}
	__unreachable__("Invalid insert job");
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
		__require__(rec->checkId());
		__tc__("processDbSmeRes.update.ok"); __tc_ok__;
		__tc__("processDbSmeRes.input.jobName"); __tc_ok__;
		__tc__("processDbSmeRes.input.toAddress"); __tc_ok__;
		__tc__("processDbSmeRes.input.fromAddress"); __tc_ok__;
		bool res = true;
		ostringstream os;
		os << lineSeparator << "UpdateJob1:" << lineSeparator;
		os << "job-name: " << getOutputJobName(rec) << lineSeparator;
		os << "from-address: " << getOutputFromAddress(rec) << lineSeparator;
		os << "to-address: " << getOutputToAddress(rec) << lineSeparator;
		os << "id: " << rec->getId() << lineSeparator;
		int rowsAffected = 0;
		DbSmeTestRecord* r = dbSmeReg->getRecord(rec->getId());
		if (r)
		{
			rowsAffected = 1;
			r->setString("job " + rec->getJob() +
				" from " + getOutputFromAddress(rec) +
				" to " + getOutputToAddress(rec));
		}
		__tc__("processDbSmeRes.update.recordsAffected"); __tc_ok__;
		os << "rows-affected: " << rowsAffected << lineSeparator;
		__require__(res);
		return os.str();
	}
	else if (rec->getJob() == "UpdateJob2")
	{
		if (dbSmeReg->size() > 1)
		{
			__tc__("processDbSmeRes.update.duplicateKey"); __tc_ok__;
			__cfg_str__(dbSmeRespDsFailure);
			return dbSmeRespDsFailure;
		}
		else
		{
			__tc__("processDbSmeRes.update.ok"); __tc_ok__;
			ostringstream os;
			os << lineSeparator << "UpdateJob2:" << lineSeparator;
			os << "id:" << rec->getId() << lineSeparator;
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
			os << "rows-affected: " << dbSmeReg->size() << lineSeparator;
			return os.str();
		}
	}
	__unreachable__("Invalid update job");
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
	os << lineSeparator << "DeleteJob:" << lineSeparator;
	__tc__("processDbSmeRes.delete.recordsAffected"); __tc_ok__;
	os << "rows-affected: " << dbSmeReg->size() << lineSeparator;
	dbSmeReg->clear();
	return os.str();
}

}
}
}

