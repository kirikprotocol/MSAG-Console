#include "DbSmeTestCases.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include <float.h>

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
using namespace smsc::test::smpp;
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::DataCoding;

DbSmeTestCases::DbSmeTestCases(SmppFixture* fixture, DbSmeRegistry* _dbSmeReg)
: SmeAcknowledgementHandler(fixture, TestConfig::getStrParam("dbSmeServiceType"),
	TestConfig::getIntParam("dbSmeProtocolId")), dbSmeReg(_dbSmeReg),
	dateFormatTc(dbSmeReg, fixture->chkList),
	otherFormatTc(dbSmeReg, fixture->chkList),
	insertTc(dbSmeReg, fixture->chkList),
	updateTc(dbSmeReg, fixture->chkList),
	deleteTc(dbSmeReg, fixture->chkList),
	selectTc(dbSmeReg, fixture->chkList),
	plsqlTc(dbSmeReg, fixture->chkList)
{
	__cfg_addr__(dbSmeAddr);
	__cfg_addr__(dbSmeInvalidAddr);
	__cfg_addr__(dbSmeAlias);
	addSmeAddr(dbSmeAddr);
	addSmeAddr(dbSmeInvalidAddr);
	addSmeAlias(dbSmeAlias);
	addSmeAlias(dbSmeInvalidAddr);
	//__require__(dbSmeReg);
	handlers["DateFormatJob1"] = &dateFormatTc;
	handlers["DateFormatJob2"] = &dateFormatTc;
	handlers["DateFormatJob3"] = &dateFormatTc;
	handlers["DateFormatJob4"] = &dateFormatTc;
	handlers["DateFormatJob5"] = &dateFormatTc;
	handlers["OtherFormatJob"] = &otherFormatTc;
	handlers["InsertJob1"] = &insertTc;
	handlers["InsertJob2"] = &insertTc;
	handlers["InsertJob3"] = &insertTc;
	handlers["UpdateJob1"] = &updateTc;
	handlers["UpdateJob2"] = &updateTc;
	handlers["DeleteJob"] = &deleteTc;
	handlers["SelectJob1"] = &selectTc;
	handlers["SelectJob2"] = &selectTc;
	handlers["SelectJob3"] = &selectTc;
	handlers["ProcedureJob"] = &plsqlTc;
	handlers["FunctionJob"] = &plsqlTc;
	handlers["ExceptionJob"] = &plsqlTc;
}

Category& DbSmeTestCases::getLog()
{
	static Category& log = Logger::getCategory("DbSmeTestCases");
	return log;
}

const Address DbSmeTestCases::getFromAddress()
{
	//dbSmeInfo.wantAlias = true
	return fixture->aliasReg->findAliasByAddress(fixture->smeAddr);
}

const Address DbSmeTestCases::getToAddress()
{
	__cfg_addr__(dbSmeAddr);
	return dbSmeAddr;
}

#define __delim__ \
	delim[rand0(delimLen-1)]

#define __print__(name) \
	if (rec->check##name()) { s << __delim__ << rec->get##name(); }
/**
 * Порядок аргументов: id, int16, int32, flt, dbl, str, dt
 */
const string DbSmeTestCases::getCmdText(DbSmeTestRecord* rec,
	const DateFormatter* df)
{
	__require__(rec && rec->checkJob());
	__decl_tc__;
	static const string delim[] = {" ", "  ", "\n", "\n\n"};
	static const int delimLen = sizeof(delim) / sizeof(*delim);
	//установить from-address и to-address
	rec->setFromAddr(getFromAddress());
	rec->setToAddr(getToAddress());
	ostringstream s;
	if (rand0(1))
	{
		s << __delim__;
	}
	__tc__("submitDbSmeCmd.jobNameMixedCase"); __tc_ok__;
	string job = rec->getJob();
	s << mixedCase(job);
	__print__(Id);
	__print__(Int8);
	__print__(Int16);
	__print__(Int32);
	__print__(Int64);
	__print__(Float);
	__print__(Double);
	__print__(LongDouble);
	__print__(String);
	if (rec->checkQuotedString())
	{
		s << __delim__ << "\"" << rec->getQuotedString() << "\"";
	}
	if (rec->checkDate())
	{
		__require__(df);
		__tc__("submitDbSmeCmd.dateMixedCase"); __tc_ok__;
		string str = df->format(rec->getDate());
		s << __delim__ << mixedCase(str);
	}
	if (rand0(1))
	{
		s << __delim__;
	}
	return s.str();
}

void DbSmeTestCases::sendDbSmePdu(const Address& addr, const string& input,
	PduData::IntProps* intProps, PduData::StrProps* strProps,
	PduData::ObjProps* objProps, bool sync, uint8_t dataCoding)
{
	__decl_tc__;
	try
	{
		//создать pdu
		PduSubmitSm* pdu = new PduSubmitSm();
		//отключить short_message & message_payload
		fixture->transmitter->setupRandomCorrectSubmitSmPdu(pdu, addr,
			false, OPT_ALL & ~OPT_MSG_PAYLOAD);
		//установить немедленную доставку
		pdu->get_message().set_scheduleDeliveryTime("");
		//текст сообщения
		switch (dataCoding)
		{
			case DEFAULT:
				__tc__("submitDbSmeCmd.cmdTextDefault");
				break;
			case UCS2:
				__tc__("submitDbSmeCmd.cmdTextUcs2");
				break;
			default:
				__unreachable__("Invalid data coding");
		}
		int msgLen;
		auto_ptr<char> msg = encode(input, dataCoding, msgLen);
		pdu->get_message().set_shortMessage(msg.get(), msgLen);
		pdu->get_message().set_dataCoding(dataCoding);
		fixture->transmitter->sendSubmitSmPdu(pdu, NULL, sync, intProps, strProps, objProps, PDU_EXT_SME);
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
		//throw;
	}
}

void DbSmeTestCases::sendDbSmePdu(const string& input,
	DbSmeTestRecord* rec, bool sync, uint8_t dataCoding)
{
	__require__(rec);
	__cfg_addr__(dbSmeAlias);
	PduData::StrProps strProps;
	strProps["dbSmeTc.input"] = input;
	PduData::ObjProps objProps;
	objProps["dbSmeTc.rec"] = rec;
	rec->ref();
	sendDbSmePdu(dbSmeAlias, input, NULL, &strProps, &objProps, sync, dataCoding);
}

void DbSmeTestCases::sendDbSmePdu(const string& input,
	const string& output, bool sync, uint8_t dataCoding)
{
	__cfg_addr__(dbSmeAlias);
	sendDbSmePdu(dbSmeAlias, input, output, sync, dataCoding);
}

void DbSmeTestCases::sendDbSmePdu(const Address& addr, const string& input,
	const string& output, bool sync, uint8_t dataCoding)
{
	__cfg_int__(timeCheckAccuracy);
	PduData::StrProps strProps;
	strProps["dbSmeTc.input"] = input;
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(fixture->smeAddr, t);
	const pair<string, uint8_t> p = convert(output, profile.codepage);
	bool valid = t + timeCheckAccuracy <= time(NULL);
	AckText* ack = new AckText(p.first, p.second, valid);
	ack->ref();
	PduData::ObjProps objProps;
	objProps["dbSmeTc.output"] = ack;
	sendDbSmePdu(addr, input, NULL, &strProps, &objProps, sync, dataCoding);
}

void DbSmeTestCases::submitCorrectFormatDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		const DateFormatter* df = NULL;
		DbSmeTestRecord* rec;
		switch (s.value())
		{
			case 1: //DateFormatJob с параметром
				rec = dateFormatTc.createJobInput(rand1(5), true, &df);
				break;
			case 2: //DateFormatJob без параметров
				rec = dateFormatTc.createJobInput(rand1(5), false, &df);
				break;
			case 3: //OtherFormatJob с параметрами
				rec = otherFormatTc.createValuesJobInput();
				break;
			case 4: //OtherFormatJob без параметров
				rec = otherFormatTc.createDefaultsJobInput();
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendDbSmePdu(getCmdText(rec, df), rec, sync, dataCoding);
	}
}

void DbSmeTestCases::submitCorrectSelectDbSmeCmd(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 3);
	for (; s.check(); s++)
	{
		DbSmeTestRecord* rec;
		switch (s.value())
		{
			case 1: //SelectJob с нулями
				rec = selectTc.createSelectNullsJobInput();
				break;
			case 2: //SelectJob со значениями
				rec = selectTc.createSelectValuesJobInput();
				break;
			case 3: //SelectJob без дефолтных output значений
				rec = selectTc.createSelectNoDefaultsJobInput();
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendDbSmePdu(getCmdText(rec, NULL), rec, sync, dataCoding);
	}
}

void DbSmeTestCases::submitCorrectInsertDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 7);
	for (; s.check(); s++)
	{
		static const DateFormatter& df = insertTc.getDateFormatter();
		DbSmeTestRecord* rec;
		switch (s.value())
		{
			case 1: //InsertJob с int параметрами
				rec = insertTc.createIntsJobInput();
				break;
			case 2: //InsertJob с uint параметрами
				rec = insertTc.createUintsJobInput();
				break;
			case 3: //InsertJob с int без параметров
				rec = insertTc.createIntDefaultsJobInput();
				break;
			case 4: //InsertJob с uint без параметров
				rec = insertTc.createUintDefaultsJobInput();
				break;
			case 5: //InsertJob с нулями
				rec = insertTc.createZerosJobInput();
				break;
			case 6: //InsertJob с нулами
				rec = insertTc.createNullsJobInput();
				break;
			case 7: //InsertJob с дублирующимся ключом
				rec = insertTc.createDuplicateKeyJobInput();
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendDbSmePdu(getCmdText(rec, &df), rec, sync, dataCoding);
	}
}

void DbSmeTestCases::submitCorrectUpdateDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__require__(dbSmeReg);
	__decl_tc__;
	TCSelector s(num, 2);
	for (; s.check(); s++)
	{
		static const DateFormatter df = updateTc.getDateFormatter();
		DbSmeTestRecord* rec;
		switch (s.value())
		{
			case 1: //UpdateJob с параметрами
				rec = updateTc.createUpdateRecordJobInput();
				break;
			case 2: //UpdateJob с дублирующимся ключом
				rec = updateTc.createDuplicateKeyJobInput();
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendDbSmePdu(getCmdText(rec, &df), rec, sync, dataCoding);
	}
}

void DbSmeTestCases::submitCorrectDeleteDbSmeCmd(bool sync, uint8_t dataCoding)
{
	__require__(dbSmeReg);
	__decl_tc__;
	DbSmeTestRecord* rec = deleteTc.createDeleteAllJobInput();
	sendDbSmePdu(getCmdText(rec, NULL), rec, sync, dataCoding);
}

void DbSmeTestCases::submitCorrectPlSqlDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 5);
	for (; s.check(); s++)
	{
		static const DateFormatter& df = plsqlTc.getDateFormatter();
		DbSmeTestRecord* rec;
		switch (s.value())
		{
			case 1: //ProcedureJob без параметров
				rec = plsqlTc.createDefaultsProcedureJobInput();
				break;
			case 2: //ProcedureJob с параметрами
				rec = plsqlTc.createProcedureJobInput();
				break;
			case 3: //FunctionJob без параметров
				rec = plsqlTc.createDefaultsFunctionJobInput();
				break;
			case 4: //FunctionJob с параметрами
				rec = plsqlTc.createFunctionJobInput();
				break;
			case 5: //ExceptionJob
				rec = plsqlTc.createExceptionJobInput();
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendDbSmePdu(getCmdText(rec, &df), rec, sync, dataCoding);
	}
}

void DbSmeTestCases::submitIncorrectDateFormatDbSmeCmd(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 19);
	__cfg_str__(dbSmeRespInputParse);
	string output = dbSmeRespInputParse;
	for (; s.check(); s++)
	{
		string input;
		switch (s.value())
		{
			//DateFormatJob1
			case 1: //неправильное число (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidDay"); __tc_ok__;
				input = "DateFormatJob1 0/1/02 1:00:00 AM";
				break;
			case 2: //неправильное число (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidDay"); __tc_ok__;
				input = "DateFormatJob1 32/1/02 1:0:0 AM";
				break;
			case 3: //неправильный формат месяца (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMonth"); __tc_ok__;
				input = "DateFormatJob1 1/Jan/02 1:0:0 AM";
				break;
			case 4: //неправильный номер месяца (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMonth"); __tc_ok__;
				input = "DateFormatJob1 1/0/02 1:0:0 AM";
				break;
			case 5: //неправильный номер месяца (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMonth"); __tc_ok__;
				input = "DateFormatJob1 1/13/02 1:0:0 AM";
				break;
			case 6: //неправильный год (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidYear"); __tc_ok__;
				input = "DateFormatJob1 1/1/2002 1:0:0 AM";
				break;
			case 7: //неправильный час (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidHour"); __tc_ok__;
				input = "DateFormatJob1 1/1/02 0:0:0 AM";
				break;
			case 8: //неправильный час (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidHour"); __tc_ok__;
				input = "DateFormatJob1 1/1/02 13:0:0 AM";
				break;
			case 9: //неправильный час (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidHour"); __tc_ok__;
				input = "DateFormatJob1 1/1/02 0:0:0 PM";
				break;
			case 10: //неправильный час (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidHour"); __tc_ok__;
				input = "DateFormatJob1 1/1/02 13:0:0 PM";
				break;
			case 11: //неправильная минута (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMinute"); __tc_ok__;
				input = "DateFormatJob1 1/1/02 1:60:0 AM";
				break;
			case 12: //неправильная секунда (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidSecond"); __tc_ok__;
				input = "DateFormatJob1 1/1/02 1:0:60 AM";
				break;
			case 13: //неправильный am/pm (формат даты "d/M/yy h:m:s t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidIndicator"); __tc_ok__;
				input = "DateFormatJob1 1/1/02 1:0:0 A";
				break;
			//DateFormatJob2
			case 14: //неправильный год (формат даты "dd.MM.yyyy hh-mm-ss t")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidYear"); __tc_ok__;
				input = "DateFormatJob2 01.01.02 01-00-00 AM";
				break;
			//DateFormatJob3
			case 15: //неправильный месяц (формат даты "d-MMM-yy H m s")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMonth"); __tc_ok__;
				input = "DateFormatJob3 1-01-02 0 0 0";
				break;
			case 16: //неправильный месяц (формат даты "d-MMM-yy H m s")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMonth"); __tc_ok__;
				input = "DateFormatJob3 1-January-02 0 0 0";
				break;
			case 17: //неправильный час (формат даты "d-MMM-yy H m s")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidHour"); __tc_ok__;
				input = "DateFormatJob3 1-Jan-02 24 0 0";
				break;
			//DateFormatJob4
			case 18: //неправильный месяц (формат даты "dd MMMM yyyy HH.mm.ss")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMonth"); __tc_ok__;
				input = "DateFormatJob4 01 01 2002 00.00.00";
				break;
			case 19: //неправильный месяц (формат даты "dd MMMM yyyy HH.mm.ss")
				__tc__("submitDbSmeCmd.incorrect.dateFormat.invalidMonth"); __tc_ok__;
				input = "DateFormatJob4 01 Jan 2002 00.00.00";
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendDbSmePdu(input, output, sync, dataCoding);
	}
}

void DbSmeTestCases::submitIncorrectNumberFormatDbSmeCmd(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 41);
	__cfg_str__(dbSmeRespInputParse);
	string output = dbSmeRespInputParse;
	for (; s.check(); s++)
	{
		string input;
		char tmp[64];
		switch (s.value())
		{
			//OtherFormatJob
			case 1: //недопустимое значение int8
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 130";
				break;
			case 2: //недопустимое значение int8
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob -130";
				break;
			case 3: //недопустимое значение int8
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 1.1";
				break;
			case 4: //недопустимое значение int8
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 1e1";
				break;

			case 5: //недопустимое значение int16
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 33000";
				break;
			case 6: //недопустимое значение int16
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 -33000";
				break;
			case 7: //недопустимое значение int16
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 1.1";
				break;
			case 8: //недопустимое значение int16
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 1e1";
				break;

			case 9: //недопустимое значение int32
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 0 2200100100";
				break;
			case 10: //недопустимое значение int32
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 0 -2200100100";
				break;
			case 11: //недопустимое значение int32
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 0 1.1";
				break;
			case 12: //недопустимое значение int32
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 0 1e1";
				break;

			case 13: //недопустимое значение int64
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 10100100100100100100";
				break;
			case 14: //недопустимое значение int64
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 -10100100100100100100";
				break;
			case 15: //недопустимое значение int64
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 1.1";
				break;
			case 16: //недопустимое значение int64
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.int"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 1e1";
				break;
			
			//InsertJob2
			case 17: //недопустимое значение uint8
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 260";
				break;
			case 18: //недопустимое значение uint8
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 -1";
				break;
			case 19: //недопустимое значение uint8
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 1.1";
				break;
			case 20: //недопустимое значение uint8
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 1e1";
				break;

			case 21: //недопустимое значение uint16
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 66000";
				break;
			case 22: //недопустимое значение uint16
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 -1";
				break;
			case 23: //недопустимое значение uint16
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 1.1";
				break;
			case 24: //недопустимое значение uint16
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 1e1";
				break;

			case 25: //недопустимое значение uint32
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 0 4400100100";
				break;
			case 26: //недопустимое значение uint32
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 0 -1";
				break;
			case 27: //недопустимое значение uint32
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 0 1.1";
				break;
			case 28: //недопустимое значение uint32
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 0 1e1";
				break;

			case 29: //недопустимое значение uint64
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 0 0 20100100100100100100";
				break;
			case 30: //недопустимое значение uint64
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 0 0 -1";
				break;
			case 31: //недопустимое значение uint64
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 0 0 1.1";
				break;
			case 32: //недопустимое значение uint64
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.uint"); __tc_ok__;
				input = "InsertJob2 0 0 0 0 1e1";
				break;

			//OtherFormatJob
			case 33: //недопустимое значение float (infinity)
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.float"); __tc_ok__;
				sprintf(tmp, "OtherFormatJob 0 0 0 0 1e%d", FLT_MAX_10_EXP + 1);
				input = tmp;
				break;
			case 34: //недопустимое значение float
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.float"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 0 1g+1";
				break;
			case 35: //недопустимое значение float
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.float"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 0 1,1";
				break;
				
			case 36: //недопустимое значение double (infinity)
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.float"); __tc_ok__;
				sprintf(tmp, "OtherFormatJob 0 0 0 0 0.0 1e%d", DBL_MAX_10_EXP + 1);
				input = tmp;
				break;
			case 37: //недопустимое значение double
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.float"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 0 0.0 1g+1";
				break;
			case 38: //недопустимое значение double
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.float"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 0 0.0 1,1";
				break;

			case 39: //недопустимое значение long double (infinity)
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.float"); __tc_ok__;
				sprintf(tmp, "OtherFormatJob 0 0 0 0 0.0 0.0 1e%d", LDBL_MAX_10_EXP + 1);
				input = tmp;
				break;
			case 40: //недопустимое значение long double
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.float"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 0 0.0 0.0 1g+1";
				break;
			case 41: //недопустимое значение long double
				__tc__("submitDbSmeCmd.incorrect.invalidNumber.float"); __tc_ok__;
				input = "OtherFormatJob 0 0 0 0 0.0 0.0 1,1";
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendDbSmePdu(input, output, sync, dataCoding);
	}
}

void DbSmeTestCases::submitIncorrectParamsDbSmeCmd(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		__cfg_addr__(dbSmeAlias);
		__cfg_addr__(dbSmeInvalidAddr);
		string input, output;
		Address addr = dbSmeAlias;
		switch (s.value())
		{
			case 1: //неправильный адрес
				{
					__tc__("submitDbSmeCmd.incorrect.invalidAddr"); __tc_ok__;
					auto_ptr<char> tmp = rand_char(20);
					input = tmp.get();
					addr = dbSmeInvalidAddr;
					__cfg_str__(dbSmeRespProviderNoFound);
					output = dbSmeRespProviderNoFound;
				}
				break;
			case 2: //неправильная задача
				{
					__tc__("submitDbSmeCmd.incorrect.invalidJob"); __tc_ok__;
					auto_ptr<char> tmp = rand_char(20);
					input = tmp.get();
					__cfg_str__(dbSmeRespJobNotFound);
					output = dbSmeRespJobNotFound;
				}
				break;
			case 3: //отсутствуют обязательные параметры
				{
					__tc__("submitDbSmeCmd.incorrect.missingParams"); __tc_ok__;
					input = "InsertJob1";
					__cfg_str__(dbSmeRespInputParse);
					output = dbSmeRespInputParse;
				}
				break;
			case 4: //присутствуют лишние параметры
				{
					__tc__("submitDbSmeCmd.incorrect.extraParams"); __tc_ok__;
					input = "DateFormatJob1 1/1/02 12:0:0 AM str";
					__cfg_str__(dbSmeRespInputParse);
					output = dbSmeRespInputParse;
				}
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendDbSmePdu(addr, input, output, sync, dataCoding);
	}
}

AckText* DbSmeTestCases::getExpectedResponse(SmeAckMonitor* monitor,
	PduDeliverySm& pdu, const string& text, time_t recvTime)
{
	__require__(monitor->pduData->objProps.count("dbSmeTc.rec"));
	__cfg_int__(timeCheckAccuracy);
	MutexGuard mguard(dbSmeReg->getMutex());
	DbSmeTestRecord* rec = dynamic_cast<DbSmeTestRecord*>(
		monitor->pduData->objProps["dbSmeTc.rec"]);
	HandlersMap::iterator it = handlers.find(rec->getJob());
	__require__(it != handlers.end());
	const string expected = it->second->processJobFirstOutput(text, rec);
	Address addr;
	SmppUtil::convert(pdu.get_message().get_dest(), &addr);
	time_t t;
	const Profile& profile = fixture->profileReg->getProfile(addr, t);
	const pair<string, uint8_t> p = convert(expected, profile.codepage);
	bool valid = t + timeCheckAccuracy <= recvTime;
	__trace2__("getExpectedResponse(): codepage = %d, valid = %s, text:\n%s",
		(int) p.second, valid ? "true" : "false", p.first.c_str());
	return new AckText(p.first, p.second, valid);
}

#define __check__(errorCode, field, value) \
	if (value != pdu.get_message().get_##field()) { __tc_fail__(errorCode); }

void DbSmeTestCases::processSmeAcknowledgement(SmeAckMonitor* monitor,
	PduDeliverySm &pdu, time_t recvTime)
{
	__decl_tc__;
	if (!dbSmeReg)
	{
		return;
	}
	__require__(monitor);
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().size_shortMessage(), pdu.get_message().get_dataCoding());
	__trace__("getExpectedResponse(): before");
	if (!monitor->pduData->objProps.count("dbSmeTc.output"))
	{
		AckText* ack = getExpectedResponse(monitor, pdu, text, recvTime);
		ack->ref();
		monitor->pduData->objProps["dbSmeTc.output"] = ack;
	}
	AckText* ack =
		dynamic_cast<AckText*>(monitor->pduData->objProps["dbSmeTc.output"]);
	__require__(ack);
	__trace2__("getExpectedResponse(): after, ack = %p", ack);
	if (!ack->valid)
	{
		monitor->setReceived();
		return;
	}
	__tc__("processDbSmeRes.checkFields");
	SmppOptional opt;
	opt.set_userMessageReference(pdu.get_optional().get_userMessageReference());
	__tc_fail2__(SmppUtil::compareOptional(opt, pdu.get_optional()), 0);
	__tc_ok_cond__;
	__tc__("processDbSmeRes.output");
	__check__(1, dataCoding, ack->dataCoding);
	if (text.length() > getMaxChars(ack->dataCoding))
	{
		__tc_fail__(2);
	}
	bool check;
	int pos = findPos(text, ack->text, getMaxChars(ack->dataCoding), check);
	__trace2__("db sme cmd: pos = %d, input:\n%s\noutput:\n%s\nexpected:\n%s\n",
		pos, monitor->pduData->strProps["dbSmeTc.input"].c_str(), text.c_str(), ack->text.c_str());
	if (pos == string::npos)
	{
		__tc_fail__(3);
		monitor->setReceived();
	}
	else
	{
		__tc_ok__;
		ack->text.erase(pos, text.length());
		if (!ack->text.length())
		{
			monitor->setReceived();
		}
		else
		{
			__tc__("processDbSmeRes.longOutput");
			if (!check)
			{
				__trace2__("text length = %d", text.length());
				__tc_fail__(1);
			}
			__tc_ok_cond__;
		}
	}
}

}
}
}

