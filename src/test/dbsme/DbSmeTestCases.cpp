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
 * Порядок аргументов: 	int16, int32, flt, dbl, dt, str
 */
void DbSmeTestCases::sendDbSmePdu(PduSubmitSm* pdu, DbSmeTestRecord* rec,
	const DateFormatter* df, bool sync, uint8_t dataCoding)
{
	__require__(pdu);
	__decl_tc__;
	try
	{
		//установить немедленную доставку
		pdu->get_message().set_scheduleDeliveryTime("");
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
		string encText = encode(s.str().c_str(), dataCoding);
		pdu->get_message().set_shortMessage(encText.c_str(), encText.length());
		pdu->get_message().set_dataCoding(dataCoding);
		//отправить и зарегистрировать pdu
		PduData::ObjProps objProps;
		objProps["dbSmeRec"] = rec;
		transmitter->sendSubmitSmPdu(pdu, NULL, sync, NULL, NULL, &objProps, false);
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
	__tc__("submitDbSmeCmd.correct.input.int");
	__tc_ok__;
	rec->setInt16(val);
}

void DbSmeTestCases::setRandomInputInt16(DbSmeTestRecord* rec)
{
	setInputInt16(rec, rand2(-32000, 32000));
}

void DbSmeTestCases::setInputInt32(DbSmeTestRecord* rec, int val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.int");
	__tc_ok__;
	rec->setInt32(val);
}

void DbSmeTestCases::setRandomInputInt32(DbSmeTestRecord* rec)
{
	setInputInt32(rec, rand2(INT_MIN, INT_MAX));
}

void DbSmeTestCases::setInputFloat(DbSmeTestRecord* rec, double val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.float");
	__tc_ok__;
	rec->setFloat(val);
}

void DbSmeTestCases::setRandomInputFloat(DbSmeTestRecord* rec)
{
	setInputFloat(rec, rand2(-100, 100) + (float) rand0(99) / 100.0);
}

void DbSmeTestCases::setInputDouble(DbSmeTestRecord* rec, double val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.float");
	__tc_ok__;
	rec->setDouble(val);
}

void DbSmeTestCases::setRandomInputDouble(DbSmeTestRecord* rec)
{
	setInputDouble(rec, rand2(-100, 100) + (double) rand0(99) / 100.0);
}

void DbSmeTestCases::setInputDate(DbSmeTestRecord* rec, time_t val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.date");
	__tc_ok__;
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
	__tc__("submitDbSmeCmd.correct.input.string");
	__tc_ok__;
	rec->setString(val);
}

void DbSmeTestCases::setInputQuotedString(DbSmeTestRecord* rec, const string& val)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.input.quotedString");
	__tc_ok__;
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
			PduSubmitSm* pdu = new PduSubmitSm();
			__cfg_addr__(dbSmeAlias);
			transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
			DbSmeTestRecord* rec = new DbSmeTestRecord();
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

void DbSmeTestCases::submitCorrectSelectDbSmeCmd(bool sync, uint8_t dataCoding)
{
	__decl_tc__;
	__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
	try
	{
		PduSubmitSm* pdu = new PduSubmitSm();
		__cfg_addr__(dbSmeAlias);
		transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
		DbSmeTestRecord* rec = new DbSmeTestRecord();
		__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
		rec->setJob("SelectJob");
		sendDbSmePdu(pdu, rec, NULL, sync, dataCoding);
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
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
					__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
					break;
				case 3: //UpdateJob с дублирующимся ключом
					__tc__("submitDbSmeCmd.correct.job.update"); __tc_ok__;
					rec->setJob("UpdateJob2");
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
	__decl_tc__;
	__tc__("processDbSmeRes.output.fromAddress"); __tc_ok__;
	return rec->getFromAddr();
}

const string DbSmeTestCases::getOutputString(const DbSmeTestRecord* rec)
{
	__decl_tc__;
	if (!rec)
	{
		__tc__("processDbSmeRes.select.defaultOutput.string"); __tc_ok__;
		return "Empty";
	}
	else if (rec->checkString())
	{
		__tc__("processDbSmeRes.input.string"); __tc_ok__;
		return rec->getString();
	}
	else if (rec->checkQuotedString())
	{
		__tc__("processDbSmeRes.input.string"); __tc_ok__;
		return rec->getQuotedString();
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.string"); __tc_ok__;
		return "xxx";
	}
}

const string DbSmeTestCases::getOutputDate(const DbSmeTestRecord* rec,
	const DateFormatter& df, int addDays)
{
	__decl_tc__;
	__tc__("processDbSmeRes.output.dateFormat"); __tc_ok__;
	if (!rec)
	{
		time_t _t = time(NULL);
		tm lt;
		localtime_r(&_t, &lt);
		lt.tm_sec = 0;
		lt.tm_min = 0;
		lt.tm_hour = 0;
		switch (addDays)
		{
			case 0: //today
				__tc__("processDbSmeRes.select.defaultOutput.today"); __tc_ok__;
				return df.format(mktime(&lt));
			case -1: //yesterday
				__tc__("processDbSmeRes.select.defaultOutput.yesterday"); __tc_ok__;
				lt.tm_mday -= 1;
				return df.format(mktime(&lt));
			case 1: //tomorrow
				__tc__("processDbSmeRes.select.defaultOutput.tomorrow"); __tc_ok__;
				lt.tm_mday += 1;
				return df.format(mktime(&lt));
			default:
				__unreachable__("Invalid addDays");
		}
	}
	else if (rec->checkDate())
	{
		__tc__("processDbSmeRes.input.date"); __tc_ok__;
		return df.format(rec->getDate());
	}
	else
	{
		time_t _t = time(NULL);
		tm lt;
		localtime_r(&_t, &lt);
		lt.tm_sec = 0;
		lt.tm_min = 0;
		lt.tm_hour = 0;
		switch (addDays)
		{
			case 0: //today
				__tc__("processDbSmeRes.defaultInput.today"); __tc_ok__;
				return df.format(mktime(&lt));
			case -1: //yesterday
				__tc__("processDbSmeRes.defaultInput.yesterday"); __tc_ok__;
				lt.tm_mday -= 1;
				return df.format(mktime(&lt));
			case 1: //tomorrow
				__tc__("processDbSmeRes.defaultInput.tomorrow"); __tc_ok__;
				lt.tm_mday += 1;
				return df.format(mktime(&lt));
			default:
				__unreachable__("Invalid addDays");
		}
	}
}

int DbSmeTestCases::getOutputInt16(const DbSmeTestRecord* rec)
{
	__decl_tc__;
	if (!rec)
	{
		__tc__("processDbSmeRes.select.defaultOutput.int"); __tc_ok__;
		return 160;
	}
	else if (rec->checkInt16())
	{
		__tc__("processDbSmeRes.input.int"); __tc_ok__;
		return rec->getInt16();
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.int"); __tc_ok__;
		return 16;
	}
}

int DbSmeTestCases::getOutputInt32(const DbSmeTestRecord* rec)
{
	__decl_tc__;
	if (!rec)
	{
		__tc__("processDbSmeRes.select.defaultOutput.int"); __tc_ok__;
		return 320;
	}
	else if (rec->checkInt32())
	{
		__tc__("processDbSmeRes.input.int"); __tc_ok__;
		return rec->getInt32();
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.int"); __tc_ok__;
		return 32;
	}
}
	
float DbSmeTestCases::getOutputFloat(const DbSmeTestRecord* rec)
{
	__decl_tc__;
	if (!rec)
	{
		__tc__("processDbSmeRes.select.defaultOutput.float"); __tc_ok__;
		return 160.16;
	}
	else if (rec->checkFloat())
	{
		__tc__("processDbSmeRes.input.float"); __tc_ok__;
		return rec->getFloat();
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.float"); __tc_ok__;
		return 16.16;
	}
}

double DbSmeTestCases::getOutputDouble(const DbSmeTestRecord* rec)
{
	__decl_tc__;
	if (!rec)
	{
		__tc__("processDbSmeRes.select.defaultOutput.float"); __tc_ok__;
		return 320.32;
	}
	else if (rec->checkDouble())
	{
		__tc__("processDbSmeRes.input.float"); __tc_ok__;
		return rec->getDouble();
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.float"); __tc_ok__;
		return 32.32;
	}
}

#define __trace_ack__(job, match, text, output) \
	__trace2__(job ": match = %s, text = %s, expected = %s", \
		match ? "true" : "false", text.c_str(), output.c_str())

void DbSmeTestCases::processDateFormatJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor, int dateJobNum)
{
	__require__(rec);
	__decl_tc__;
	__tc__("processDbSmeRes.output.select.singleRecord"); __tc_ok__;
	static const DateFormatter df("w ww  W.WW..d,dd,,ddd/M//MM-MMM--MMMM	MMMMM		y yy yyy yyyy yyyyy h hh hhh m mm mmm t tt");
	static const string prefix("Date: ");
	if (rec->checkDate())
	{
		__tc__("processDbSmeRes.output");
		string output = prefix + getOutputDate(rec, df, 0);
		bool match = text == output;
		__trace_ack__("DateFormatJob", match, text, output);
		if (!match)
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
	else
	{
		bool match = false;
		string output;
		switch (dateJobNum)
		{
			case 1: //now
				__tc__("processDbSmeRes.defaultInput.now"); __tc_ok__;
				__require__(monitor->startTime <= time(NULL));
				for (time_t t = monitor->startTime; t <= time(NULL); t++)
				{
					output = prefix + df.format(t);
					match = text == output;
					__trace_ack__("DateFormatJob", match, text, output);
					if (match)
					{
						break;
					}
				}
				break;
			case 2: //today
				output = prefix + getOutputDate(rec, df, 0);
				match = text == output;
				__trace_ack__("DateFormatJob", match, text, output);
				break;
			case 3: //yesterday
				output = prefix + getOutputDate(rec, df, -1);
				match = text == output;
				__trace_ack__("DateFormatJob", match, text, output);
				break;
			case 4: //tomorrow
				output = prefix + getOutputDate(rec, df, 1);
				match = text == output;
				__trace_ack__("DateFormatJob", match, text, output);
				break;
			default:
				__unreachable__("Invalid dateJobNum");
		}
		__tc__("processDbSmeRes.output");
		if (!match)
		{
			__tc_fail__(dateJobNum);
		}
		__tc_ok_cond__;
	}
	monitor->setReceived();
	delete rec;
}

void DbSmeTestCases::processOtherFormatJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(rec);
	__decl_tc__;
	__tc__("processDbSmeRes.select.singleRecord"); __tc_ok__;
	ostringstream res;
	res << "from-address: '" << getOutputFromAddress(rec) << "';" << endl;
	string str = getOutputString(rec);
	res << "string: \"" << str << "\"," << endl;
	__tc__("processDbSmeRes.output.string.left"); __tc_ok__;
	__tc__("processDbSmeRes.output.string.right"); __tc_ok__;
	__tc__("processDbSmeRes.output.string.center"); __tc_ok__;
	if (str.length() < 30)
	{
		res << "string_right: " << string(30 - str.length(), ' ') << str << "," << endl;
		int tmp = (30 - str.length()) / 2;
		res << "string_center: " << string(tmp, ' ') << str <<
			string(30 - tmp - str.length(), ' ') << "," << endl;
	}
	else
	{
		res << "string_right: " << str << "," << endl;
		res << "string_center: " << str << ",\n" << endl;
	}
	res << "string_left: " << str << ";" << endl;
	res << "int16: (--$" << getOutputInt16(rec) << "--)," << endl;
	res << "int32: (++$" << getOutputInt32(rec) << "++)," << endl;
	res << "float: (**$" << getOutputFloat(rec) << "**)," << endl;
	res << "double: (::$" << getOutputDouble(rec) << "::);";
	string output = res.str();
	bool match = text == output;
	__trace_ack__("OtherFormatJob", match, text, output);
	__tc__("processDbSmeRes.output");
	if (!match)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
	delete rec;
}

void DbSmeTestCases::processInsertJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(rec);
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream res;
	__require__(rec->checkId());
	if (dbSmeReg->getRecord(rec->getId()))
	{
		__tc__("processDbSmeRes.insert.duplicateKey"); __tc_ok__;
		res << "Duplicate key";
		delete rec;
	}
	else
	{
		__tc__("processDbSmeRes.insert.ok"); __tc_ok__;
		ostringstream res;
		res << "InsertJob:" << endl;
		res << "from-address: " << getOutputFromAddress(rec) << endl;
		res << "string: " << getOutputString(rec) << endl;
		static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
		res << "date: " << getOutputDate(rec, df, 0) << endl;
		res << "int16: " << getOutputInt16(rec) << endl;
		res << "int32: " << getOutputInt32(rec) << endl;
		res << "float: " << getOutputFloat(rec) << endl;
		res << "double: " << getOutputDouble(rec) << endl;
		res << "rows-affected: 1";
		dbSmeReg->putRecord(rec);
	}
	string output = res.str();
	bool match = text == output;
	__trace_ack__("InsertJob", match, text, output);
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
	ostringstream res;
	res << "UpdateJob1:" << endl;
	res << "from-address: " << getOutputFromAddress(rec) << endl;
	res << "string: " << getOutputString(rec) << endl;
	static const DateFormatter df("dd-MM-yyyy HH:mm:ss");
	res << "date: " << getOutputDate(rec, df, 0) << endl;
	res << "int16: " << getOutputInt16(rec) << endl;
	res << "int32: " << getOutputInt32(rec) << endl;
	res << "float: " << getOutputFloat(rec) << endl;
	res << "double: " << getOutputDouble(rec) << endl;
	int rowsAffected = 0;
	DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
	while (DbSmeTestRecord* r = it->next())
	{
		__require__(r->checkId());
		int id = r->getId();
		if (id % 5 == 0)
		{
			rowsAffected++;
			//обновить запись
			*r = *rec;
			r->setId(id);
		}
	}
	delete it;
	__tc__("processDbSmeRes.update.recordsAffected"); __tc_ok__;
	res << "rows-affected: " << rowsAffected;
	string output = res.str();
	bool match = text == output;
	__trace_ack__("UpdateJob1", match, text, output);
	__tc__("processDbSmeRes.output");
	if (!match)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
	delete rec;
}

void DbSmeTestCases::processUpdateDuplicateJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(rec);
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream res;
	if (dbSmeReg->size() > 1)
	{
		__tc__("processDbSmeRes.update.duplicateKey"); __tc_ok__;
		res << "Duplicate key";
	}
	else
	{
		__tc__("processDbSmeRes.update.ok"); __tc_ok__;
		ostringstream res;
		res << "UpdateJob2:" << endl;
		DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
		while (DbSmeTestRecord* r = it->next())
		{
			//обновить запись
			r->setId(rec->getId());
		}
		delete it;
		__tc__("processDbSmeRes.update.recordsAffected"); __tc_ok__;
		res << "rows-affected: " << dbSmeReg->size();
	}
	__tc__("processDbSmeRes.output");
	string output = res.str();
	bool match = text == output;
	__trace_ack__("UpdateJob2", match, text, output);
	if (!match)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
	delete rec;
}

void DbSmeTestCases::processDeleteJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream res;
	__tc__("processDbSmeRes.delete.ok"); __tc_ok__;
	res << "DeleteJob:" << endl;
	__tc__("processDbSmeRes.delete.recordsAffected"); __tc_ok__;
	res << "rows-affected: " << dbSmeReg->size();
	dbSmeReg->clear();
	string output = res.str();
	bool match = text == output;
	__trace_ack__("DeleteJob", match, text, output);
	__tc__("processDbSmeRes.output");
	if (!match)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	monitor->setReceived();
	delete rec;
}

void DbSmeTestCases::processSelectJobAck(const string& text,
	DbSmeTestRecord* rec, SmeAckMonitor* monitor)
{
	__require__(dbSmeReg);
	__decl_tc__;
	ostringstream res;
	static const DateFormatter df1("dd.MM.yyyy hh-mm-ss t");
	static const DateFormatter df2("d-MMM-yy H m s");
	static const DateFormatter df3("dd MMMM yyyy HH.mm.ss");
	static const DateFormatter df4("d/M/yy h:m:s t");
	if (!monitor->pduData->strProps.count("output"))
	{
		DbSmeRegistry::DbSmeTestRecordIterator* it = dbSmeReg->getRecords();
		int i = 0;
		while (DbSmeTestRecord* r = it->next())
		{
			__require__(r->checkId());
			int id = r->getId();
			if (id % 5 == 0)
			{
				if (i++)
				{
					res << endl;
				}
				res << "SelectJob:" << endl;
				res << getOutputInt16(r) << endl;
				res << getOutputInt32(r) << endl;
				res << getOutputFloat(r) << endl;
				res << getOutputDouble(r) << endl;
				res << getOutputString(r) << endl;
				res << getOutputDate(r, df1, 0) << endl;
				res << getOutputDate(r, df2, -1) << endl;
				res << getOutputDate(r, df3, 1) << endl;
				res << getOutputDate(r, df4, 0);
			}
		}
		delete it;
		monitor->pduData->strProps["output"] = res.str();
	}
	const string& output = monitor->pduData->strProps["output"];
	if (!output.length())
	{
		res << "SelectJob:" << endl;
		res << getOutputInt16(NULL) << endl;
		res << getOutputInt32(NULL) << endl;
		res << getOutputFloat(NULL) << endl;
		res << getOutputDouble(NULL) << endl;
		res << getOutputString(NULL) << endl;
		res << getOutputDate(NULL, df1, 0) << endl;
		res << getOutputDate(NULL, df2, -1) << endl;
		res << getOutputDate(NULL, df3, 1) << endl;
		string tmp = res.str();
		__tc__("processDbSmeRes.select.defaultOutput.now"); __tc_ok__;
		__require__(monitor->startTime <= time(NULL));
		bool match;
		for (time_t t = monitor->startTime; t <= time(NULL); t++)
		{
			string output = tmp + df4.format(t);
			match = text == output;
			__trace_ack__("SelectJob", match, text, output);
			if (match)
			{
				break;
			}
		}
		__tc__("processDbSmeRes.output");
		if (!match)
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
	else
	{
		int pos = monitor->pduData->intProps.count("output") ?
			monitor->pduData->intProps["output"] : 0;
		bool match = !output.compare(pos, text.length(), text);
		__trace2__("SelectJob: match = %s, pos = %d, text = %s, expected = %s",
			match ? "true" : "false", pos, text.c_str(), output.c_str());
		__tc__("processDbSmeRes.output");
		if (!match)
		{
			__tc_fail__(1);
			monitor->setReceived();
			delete rec;
		}
		__tc_ok_cond__;
		if (pos + text.length() == output.length())
		{
			monitor->setReceived();
			delete rec;
		}
		else
		{
			monitor->pduData->intProps["output"] = pos + text.length();
		}
	}
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
	else
	{
		__unreachable__("Unsupported job");
	}
	//delete rec;
}

}
}
}

