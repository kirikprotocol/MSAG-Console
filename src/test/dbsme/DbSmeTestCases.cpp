#include "DbSmeTestCases.hpp"
#include "DateFormatter.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "test/conf/TestConfig.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using std::ostringstream;
using smsc::sms::Address;
using smsc::sms::AddressValue;
using smsc::core::synchronization::MutexGuard;
using smsc::util::Logger;
using smsc::test::conf::TestConfig;
using smsc::test::smpp::SmppUtil;
using namespace smsc::test::core; //constants, PduData
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet;

Category& DbSmeTestCases::getLog()
{
	static Category& log = Logger::getCategory("DbSmeTestCases");
	return log;
}

const string DbSmeTestCases::getFromAddress()
{
	const Address smeAlias =
		fixture->aliasReg->findAliasByAddress(fixture->smeAddr);
	AddressValue addrVal;
	smeAlias.getValue(addrVal);
	return addrVal;
}

/**
 * Порядок аргументов: id, int16, int32, flt, dbl, str, dt
 */
void DbSmeTestCases::sendDbSmePdu(PduSubmitSm* pdu, DbSmeTestRecord* rec,
	const DateFormatter* df, bool sync, uint8_t dataCoding)
{
	__require__(pdu);
	__decl_tc__;
	try
	{
		switch (dataCoding)
		{
			case DATA_CODING_SMSC_DEFAULT:
				__tc__("submitDbSmeCmd.cmdTextDefault");
				break;
			case DATA_CODING_UCS2:
				__tc__("submitDbSmeCmd.cmdTextUcs2");
				break;
			default:
				__unreachable__("Invalid data coding");
		}
		//установить from-address
		static const string fromAddr = getFromAddress();
		rec->setFromAddr(fromAddr);
		ostringstream s;
		__require__(rec->checkJob());
		s << rec->getJob();
		if (rec->checkId())
		{
			s << " " << rec->getId();
		}
		if (rec->checkInt16())
		{
			s << " " << rec->getInt16();
		}
		if (rec->checkInt32())
		{
			s << " " << rec->getInt32();
		}
		if (rec->checkFloat())
		{
			s << " " << rec->getFloat();
		}
		if (rec->checkDouble())
		{
			s << " " << rec->getDouble();
		}
		if (rec->checkString())
		{
			s << " " << rec->getString();
		}
		if (rec->checkQuotedString())
		{
			s << " \"" << rec->getQuotedString() << "\"";
		}
		if (rec->checkDate())
		{
			__require__(df);
			s << " " << df->format(rec->getDate());
		}
		//установить немедленную доставку
		pdu->get_message().set_scheduleDeliveryTime("");
		//текст сообщения
		const string text = s.str();
		const string encText = encode(text.c_str(), dataCoding);
		pdu->get_message().set_shortMessage(encText.c_str(), encText.length());
		pdu->get_message().set_dataCoding(dataCoding);
		//отправить и зарегистрировать pdu
		PduData::StrProps strProps;
		strProps["input"] = text;
		PduData::ObjProps objProps;
		objProps["dbSmeRec"] = rec;
		rec->ref();
		transmitter->sendSubmitSmPdu(pdu, NULL, sync, NULL, &strProps, &objProps, false);
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

void DbSmeTestCases::setInputInt16(DbSmeTestRecord* rec, int val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.int"); __tc_ok__;
	rec->setInt16(val);
}

void DbSmeTestCases::setRandomInputInt16(DbSmeTestRecord* rec)
{
	setInputInt16(rec, rand2(-32000, 32000));
}

void DbSmeTestCases::setInputInt32(DbSmeTestRecord* rec, int val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.int"); __tc_ok__;
	rec->setInt32(val);
}

void DbSmeTestCases::setRandomInputInt32(DbSmeTestRecord* rec)
{
	setInputInt32(rec, rand2(INT_MIN, INT_MAX));
}

void DbSmeTestCases::setInputFloat(DbSmeTestRecord* rec, double val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.float"); __tc_ok__;
	rec->setFloat(val);
}

void DbSmeTestCases::setRandomInputFloat(DbSmeTestRecord* rec)
{
	setInputFloat(rec, rand2(-100, 100) + (float) rand0(99) / 100.0);
}

void DbSmeTestCases::setInputDouble(DbSmeTestRecord* rec, double val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.float"); __tc_ok__;
	rec->setDouble(val);
}

void DbSmeTestCases::setRandomInputDouble(DbSmeTestRecord* rec)
{
	setInputDouble(rec, rand2(-100, 100) + (double) rand0(99) / 100.0);
}

void DbSmeTestCases::setInputDate(DbSmeTestRecord* rec, time_t val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.date"); __tc_ok__;
	rec->setDate(val);
}

void DbSmeTestCases::setRandomInputDate(DbSmeTestRecord* rec)
{
	static const int year = 365 * 24 * 3600;
	setInputDate(rec, time(NULL) + rand2(-year, year));
}

void DbSmeTestCases::setInputString(DbSmeTestRecord* rec, const string& val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.string"); __tc_ok__;
	rec->setString(val);
}

void DbSmeTestCases::setInputQuotedString(DbSmeTestRecord* rec, const string& val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.quotedString"); __tc_ok__;
	rec->setQuotedString(val);
}

void DbSmeTestCases::setRandomInputString(DbSmeTestRecord* rec, bool quotedString)
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

void DbSmeTestCases::submitCorrectFormatDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
		try
		{
			//constants
			static const string dfJobs[] =
				{"DateFormatJob1",
				"DateFormatJob2",
				"DateFormatJob3",
				"DateFormatJob4"};
			static const DateFormatter dfs[] =
				{DateFormatter("d/M/yy h:m:s t"),
				DateFormatter("dd.MM.yyyy hh-mm-ss t"),
				DateFormatter("d-MMM-yy H m s"),
				DateFormatter("dd MMMM yyyy HH.mm.ss")};
			int dfJobNum = rand0(3);
			const DateFormatter* df = NULL;
			//pdu
			PduSubmitSm* pdu = new PduSubmitSm();
			__cfg_addr__(dbSmeAlias);
			transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
			//rec
			DbSmeTestRecord* rec = new DbSmeTestRecord();
			rec->defInput = new DbSmeTestRecord();
			switch (dfJobNum)
			{
				case 1:
					rec->defInput->setDateType(DT_NOW);
					rec->defInput->setDate(time(NULL));
					break;
				case 2:
					rec->defInput->setDateType(DT_TODAY);
					rec->defInput->setDate(getDate(DT_TODAY));
					break;
				case 3:
					rec->defInput->setDateType(DT_YESTERDAY);
					rec->defInput->setDate(getDate(DT_YESTERDAY));
					break;
				case 4:
					rec->defInput->setDateType(DT_TOMORROW);
					rec->defInput->setDate(getDate(DT_TOMORROW));
					break;
				default:
					__unreachable__("Invalid dfJobNum");
			}
			switch (s.value())
			{
				case 1: //DateFormatJob с параметром
					rec->setJob(dfJobs[dfJobNum]);
					df = dfs + dfJobNum;
					setRandomInputDate(rec);
					break;
				case 2: //DateFormatJob без параметров
					__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
					rec->setJob(dfJobs[dfJobNum]);
					__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
					break;
				case 3: //OtherFormatJob с параметрами
					rec->setJob("OtherFormatJob");
					setRandomInputInt16(rec);
					setRandomInputInt32(rec);
					setRandomInputFloat(rec);
					setRandomInputDouble(rec);
					setRandomInputString(rec, rand0(1));
					break;
				case 4: //OtherFormatJob без параметров
					__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
					rec->setJob("OtherFormatJob");
					__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendDbSmePdu(pdu, rec, df, sync, dataCoding);
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void DbSmeTestCases::submitCorrectSelectDbSmeCmd(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 2);
	for (; s.check(); s++)
	{
		__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			__cfg_addr__(dbSmeAlias);
			transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
			DbSmeTestRecord* rec = new DbSmeTestRecord();
			switch (s.value())
			{
				case 1: //SelectJob
					__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
					rec->setJob("SelectJob");
					break;
				case 2:
					rec->setJob("SelectNoDefaultJob");
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendDbSmePdu(pdu, rec, NULL, sync, dataCoding);
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

DbSmeTestRecord* DbSmeTestCases::getInsertJobDefaultInput()
{
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setInt16(16);
	rec->setInt32(32);
	rec->setFloat(16.16);
	rec->setDouble(32.32);
	rec->setString("xxx");
	rec->setDateType(DT_TODAY);
	rec->setDate(getDate(DT_TODAY));
	return rec;
}

void DbSmeTestCases::submitCorrectInsertDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__require__(dbSmeReg);
	__decl_tc__;
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			__cfg_addr__(dbSmeAlias);
			transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
			DbSmeTestRecord* rec = new DbSmeTestRecord();
			rec->defInput = getInsertJobDefaultInput();
			static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
			switch (s.value())
			{
				case 1: //InsertJob с параметрами
					__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
					rec->setJob("InsertJob");
					rec->setId(dbSmeReg->nextId());
					setRandomInputInt16(rec);
					setRandomInputInt32(rec);
					setRandomInputFloat(rec);
					setRandomInputDouble(rec);
					setRandomInputString(rec, rand0(1));
					setRandomInputDate(rec);
					break;
				case 2: //InsertJob без параметров
					__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
					rec->setJob("InsertJob");
					rec->setId(dbSmeReg->nextId());
					__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
					break;
				case 3: //InsertJob с дублирующимся ключом
					__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
					rec->setJob("InsertJob");
					rec->setId(dbSmeReg->getExistentId());
					__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
					break;
				case 4: //InsertJob с нулями
					__tc__("submitDbSmeCmd.correct.job.insert"); __tc_ok__;
					rec->setJob("InsertJob");
					rec->setId(dbSmeReg->getExistentId());
					setInputInt16(rec, 0);
					setInputInt32(rec, 0);
					setInputFloat(rec, 0.0);
					setInputDouble(rec, 0.0);
					setInputQuotedString(rec, "");
					setInputDate(rec, 0);
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendDbSmePdu(pdu, rec, &df, sync, dataCoding);
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

DbSmeTestRecord* DbSmeTestCases::getUpdateJob1DefaultInput()
{
	DbSmeTestRecord* rec = new DbSmeTestRecord();
	rec->setInt16(17);
	rec->setInt32(33);
	rec->setFloat(17.17);
	rec->setDouble(33.33);
	rec->setString("zzz");
	rec->setDateType(DT_TODAY);
	rec->setDate(getDate(DT_TODAY));
	return rec;
}

void DbSmeTestCases::submitCorrectUpdateDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__require__(dbSmeReg);
	__decl_tc__;
	TCSelector s(num, 3);
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			__cfg_addr__(dbSmeAlias);
			transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
			DbSmeTestRecord* rec = new DbSmeTestRecord();
			static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
			switch (s.value())
			{
				case 1: //UpdateJob с параметрами
					__tc__("submitDbSmeCmd.correct.job.update"); __tc_ok__;
					rec->setJob("UpdateJob1");
					rec->defInput = getUpdateJob1DefaultInput();
					setRandomInputInt16(rec);
					setRandomInputInt32(rec);
					setRandomInputFloat(rec);
					setRandomInputDouble(rec);
					setRandomInputString(rec, rand0(1));
					setRandomInputDate(rec);
					break;
				case 2: //UpdateJob без параметров
					__tc__("submitDbSmeCmd.correct.job.update"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
					rec->setJob("UpdateJob1");
					rec->defInput = getUpdateJob1DefaultInput();
					__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
					break;
				case 3: //UpdateJob с дублирующимся ключом
					__tc__("submitDbSmeCmd.correct.job.update"); __tc_ok__;
					rec->setJob("UpdateJob2");
					//rec->defInput = NULL;
					rec->setId(dbSmeReg->getExistentId());
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendDbSmePdu(pdu, rec, &df, sync, dataCoding);
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void DbSmeTestCases::submitCorrectDeleteDbSmeCmd(bool sync, uint8_t dataCoding)
{
	__require__(dbSmeReg);
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.delete"); __tc_ok__;
	try
	{
		PduSubmitSm* pdu = new PduSubmitSm();
		__cfg_addr__(dbSmeAlias);
		transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
		DbSmeTestRecord* rec = new DbSmeTestRecord();
		__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
		rec->setJob("DeleteJob");
		sendDbSmePdu(pdu, rec, NULL, sync, dataCoding);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void DbSmeTestCases::submitIncorrectDbSmeCmd(bool sync, uint8_t dataCoding)
{
}

const string DbSmeTestCases::getOutputFromAddress(const DbSmeTestRecord* rec)
{
	__require__(rec);
	__decl_tc__;
	__tc__("processDbSmeRes.output.fromAddress"); __tc_ok__;
	return rec->getFromAddr();
}

const string DbSmeTestCases::getOutputString(const DbSmeTestRecord* rec,
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

time_t DbSmeTestCases::getDate(DateType dtType, time_t now)
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

const string DbSmeTestCases::getOutputDate(const DbSmeTestRecord* rec,
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

int DbSmeTestCases::getOutputInt16(const DbSmeTestRecord* rec,
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

int DbSmeTestCases::getOutputInt32(const DbSmeTestRecord* rec,
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
	
float DbSmeTestCases::getOutputFloat(const DbSmeTestRecord* rec,
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

double DbSmeTestCases::getOutputDouble(const DbSmeTestRecord* rec,
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

#define __trace_ack__(monitor, match, pos, output, expected) \
	__trace2__("db sme cmd: match = %s, input:\n%s\noutput(%d):\n%s\nexpected:\n%s\n", \
		match ? "true" : "false", monitor->pduData->strProps["input"].c_str(), pos, output.c_str(), expected.c_str())

void DbSmeTestCases::processDateFormatJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor, int dateJobNum)
{
	__require__(rec);
	__decl_tc__;
	__tc__("processDbSmeRes.select.singleRecord"); __tc_ok__;
	static const DateFormatter df("!@#$%^&*()_+-=|\\:;\'<,>.?/wMMMMMMMMwwyyWyyyyWWyyyyyydyyyyyyyyddhdddhhddddhhhMhhhhMMmMMMmmMMMMmmmMMMMMmmmmMMMMMMtMMMMMMMtt");
	static const string prefix("\nDate: ");
	bool match = false;
	string expected;
	bool res = true;
	if (dateJobNum == 1 && !rec->checkDate())
	{
		__require__(rec->defInput && rec->defInput->checkDate());
		while (rec->defInput->getDate() <= time(NULL))
		{
			expected = prefix + getOutputDate(rec, NULL, df, res) + "\n";
			__require__(res);
			match = text == expected;
			if (match)
			{
				break;
			}
			rec->defInput->setDate(rec->defInput->getDate() + 1);
		}
	}
	else
	{
		switch (dateJobNum)
		{
			case 1: //now
			case 2: //today
			case 3: //yesterday
			case 4: //tomorrow
				break;
			default:
				__unreachable__("Invalid dateJobNum");
		}
		expected = prefix + getOutputDate(rec, NULL, df, res) + "\n";
		__require__(res);
		match = text == expected;
	}
	__tc__("processDbSmeRes.output");
	__trace_ack__(monitor, match, 0, text, expected);
	if (!match)
	{
		__tc_fail__(dateJobNum);
	}
	__tc_ok_cond__;
	monitor->setReceived();
	//delete rec;
}

void DbSmeTestCases::processOtherFormatJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
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
	string expected = os.str();
	bool match = text == expected;
	__trace_ack__(monitor, match, 0, text, expected);
	__tc__("processDbSmeRes.output");
	if (!match)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
	//delete rec;
}

void DbSmeTestCases::processInsertJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(rec && rec->checkId());
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream os;
	if (dbSmeReg->getRecord(rec->getId()))
	{
		__tc__("processDbSmeRes.insert.duplicateKey"); __tc_ok__;
		os << "Duplicate key";
		//delete rec;
	}
	else
	{
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
	}
	string expected = os.str();
	bool match = text == expected;
	__trace_ack__(monitor, match, 0, text, expected);
	__tc__("processDbSmeRes.output");
	if (!match)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
}

void DbSmeTestCases::processUpdateOkJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(rec);
	__require__(dbSmeReg);
	__decl_tc__;
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
	string expected = os.str();
	bool match = text == expected;
	__trace_ack__(monitor, match, 0, text, expected);
	__tc__("processDbSmeRes.output");
	if (!match)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
	//delete rec;
}

void DbSmeTestCases::processUpdateDuplicateJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(rec);
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream os;
	if (dbSmeReg->size() > 1)
	{
		__tc__("processDbSmeRes.update.duplicateKey"); __tc_ok__;
		os << "Duplicate key";
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
	}
	__tc__("processDbSmeRes.output");
	string expected = os.str();
	bool match = text == expected;
	__trace_ack__(monitor, match, 0, text, expected);
	if (!match)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
	//delete rec;
}

void DbSmeTestCases::processDeleteJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream os;
	__tc__("processDbSmeRes.delete.ok"); __tc_ok__;
	os << endl << "DeleteJob:" << endl;
	__tc__("processDbSmeRes.delete.recordsAffected"); __tc_ok__;
	os << "rows-affected: " << dbSmeReg->size() << endl;
	dbSmeReg->clear();
	string expected = os.str();
	bool match = text == expected;
	__trace_ack__(monitor, match, 0, text, expected);
	__tc__("processDbSmeRes.output");
	if (!match)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
	//delete rec;
}

void DbSmeTestCases::writeSelectJobRecord(ostream& os, DbSmeTestRecord* rec,
	DbSmeTestRecord* defOutput, time_t now, bool& res)
{
	static const DateFormatter df1("dd.MM.yyyy hh-mm-ss t");
	static const DateFormatter df2("d-MMM-yy H m s");
	static const DateFormatter df3("dd MMMM yyyy HH.mm.ss");
	static const DateFormatter df4("d/M/yy h:m:s t");

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

void DbSmeTestCases::processSelectJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(dbSmeReg);
	__decl_tc__;
	int pos = monitor->pduData->intProps.count("output") ?
		monitor->pduData->intProps["output"] : 0;
	if (!pos)
	{
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
				writeSelectJobRecord(os, r, &defOutput, t, res);
				__require__(res);
			}
			delete it;
			expected = os.str();
			if (!expected.length())
			{
				writeSelectJobRecord(os, NULL, &defOutput, t, res);
				__require__(res);
			}
			expected = os.str();
			match = !expected.compare(0, text.length(), text);
			if (match)
			{
				monitor->pduData->strProps["output"] = expected;
				monitor->pduData->intProps["output"] = text.length();
				break;
			}
		}
		__tc__("processDbSmeRes.output");
		__trace_ack__(monitor, match, 0, text, expected);
		if (!match)
		{
			__tc_fail__(1);
			monitor->setReceived();
			//delete rec;
		}
		__tc_ok_cond__;
	}
	else
	{
		const string& expected = monitor->pduData->strProps["output"];
		__require__(expected.length());
		bool match = !expected.compare(pos, text.length(), text);
		__trace_ack__(monitor, match, pos, text, expected);
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
		}
		else
		{
			__tc_fail__(1);
			monitor->setReceived();
			//delete rec;
		}
		__tc_ok_cond__;
	}
}

void DbSmeTestCases::processSelectNoDefaultJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(dbSmeReg);
	__decl_tc__;
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	if (!monitor->pduData->strProps.count("output"))
	{
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
		monitor->pduData->strProps["output"] = res ? os.str() : "Invalid format";
	}
	const string& expected = monitor->pduData->strProps["output"];
	int pos = monitor->pduData->intProps.count("output") ?
		monitor->pduData->intProps["output"] : 0;
	bool match = !expected.compare(pos, text.length(), text);
	__trace_ack__(monitor, match, pos, text, expected);
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
	}
	else
	{
		__tc_fail__(1);
		monitor->setReceived();
		//delete rec;
	}
	__tc_ok_cond__;
}

void DbSmeTestCases::processSmeAcknowledgement(SmeAckMonitor* monitor,
	PduDeliverySm &pdu)
{
	if (!dbSmeReg)
	{
		return;
	}
	if (monitor->getFlag() != PDU_REQUIRED_FLAG)
	{
		return;
	}
	__require__(monitor);
	__decl_tc__;
	PduData::ObjProps::const_iterator it = monitor->pduData->objProps.find("dbSmeRec");
	__require__(it != monitor->pduData->objProps.end());
	DbSmeTestRecord* rec = reinterpret_cast<DbSmeTestRecord*>(it->second);
	__require__(rec);
	__tc__("processDbSmeRes.dataCoding");
	if (pdu.get_message().get_dataCoding() != DATA_CODING_SMSC_DEFAULT)
	{
		__tc_fail__(1);
		return;
	}
	__tc_ok_cond__;
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().size_shortMessage(), pdu.get_message().get_dataCoding());
	int jobNum;
	MutexGuard mguard(dbSmeReg->getMutex());
	if (sscanf(rec->getJob().c_str(), "DateFormatJob%u", &jobNum))
	{
		__require__(jobNum >= 1 && jobNum <= 4);
		processDateFormatJobAck(text, rec, monitor, jobNum);
	}
	else if (rec->getJob() == "OtherFormatJob")
	{
		processOtherFormatJobAck(text, rec, monitor);
	}
	else if (rec->getJob() == "InsertJob")
	{
		processInsertJobAck(text, rec, monitor);
	}
	else if (sscanf(rec->getJob().c_str(), "UpdateJob%u", &jobNum))
	{
		switch (jobNum)
		{
			case 1:
				processUpdateOkJobAck(text, rec, monitor);
				break;
			case 2:
				processUpdateDuplicateJobAck(text, rec, monitor);
				break;
			default:
				__unreachable__("Unsupported job");
		}
	}
	else if (rec->getJob() == "DeleteJob")
	{
		processDeleteJobAck(text, rec, monitor);
	}
	else if (rec->getJob() == "SelectJob")
	{
		processSelectJobAck(text, rec, monitor);
	}
	else if (rec->getJob() == "SelectNoDefaultJob")
	{
		processSelectNoDefaultJobAck(text, rec, monitor);
	}
	else
	{
		__unreachable__("Unsupported job");
	}
	//delete rec;
}

}
}
}

