#include "DbSmeTestCases.hpp"
#include "DateFormatter.hpp"
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
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet;

DbSmeTestCases::DbSmeTestCases(const SmeConfig& config, SmppFixture* fixture,
	DbSmeRegistry* _dbSmeReg)
: SmppProtocolTestCases(config, fixture), dbSmeReg(_dbSmeReg),
	simpleTc(dbSmeReg, fixture->chkList),
	dateFormatTc(dbSmeReg, fixture->chkList),
	otherFormatTc(dbSmeReg, fixture->chkList),
	insertTc(dbSmeReg, fixture->chkList),
	updateTc(dbSmeReg, fixture->chkList),
	deleteTc(dbSmeReg, fixture->chkList),
	selectTc(dbSmeReg, fixture->chkList)
{
	fixture->ackHandler = this;
	//__require__(dbSmeReg);
}

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

const string DbSmeTestCases::getToAddress()
{
	__cfg_addr__(dbSmeAddr);
	AddressValue addrVal;
	dbSmeAddr.getValue(addrVal);
	return addrVal;
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
	s << rec->getJob();
	__print__(Id);
	__print__(Int8);
	__print__(Int16);
	__print__(Int32);
	__print__(Int64);
	__print__(Float);
	__print__(Double);
	__print__(LongDouble);
	__print__(String)
	if (rec->checkQuotedString())
	{
		s << __delim__ << "\"" << rec->getQuotedString() << "\"";
	}
	if (rec->checkDate())
	{
		__require__(df);
		s << __delim__ << df->format(rec->getDate());
	}
	if (rand0(1))
	{
		s << __delim__;
	}
	return s.str();
}

void DbSmeTestCases::sendDbSmePdu(const string& input, PduData::IntProps* intProps,
	PduData::StrProps* strProps, PduData::ObjProps* objProps, bool sync,
	uint8_t dataCoding)
{
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
		//создать pdu
		PduSubmitSm* pdu = new PduSubmitSm();
		__cfg_addr__(dbSmeAlias);
		transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
		//установить немедленную доставку
		pdu->get_message().set_scheduleDeliveryTime("");
		//текст сообщения
		const string encText = encode(input.c_str(), dataCoding);
		pdu->get_message().set_shortMessage(encText.c_str(), encText.length());
		pdu->get_message().set_dataCoding(dataCoding);
		transmitter->sendSubmitSmPdu(pdu, NULL, sync, intProps, strProps, objProps, false);
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
	PduData::StrProps strProps;
	strProps["input"] = input;
	PduData::ObjProps objProps;
	objProps["dbSmeRec"] = rec;
	rec->ref();
	sendDbSmePdu(input, NULL, &strProps, &objProps, sync, dataCoding);
}

void DbSmeTestCases::sendDbSmePdu(const string& input,
	const string& output, bool sync, uint8_t dataCoding)
{
	PduData::StrProps strProps;
	strProps["input"] = input;
	strProps["output"] = output;
	sendDbSmePdu(input, NULL, &strProps, NULL, sync, dataCoding);
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

void DbSmeTestCases::submitIncorrectDateFormatDbSmeCmd(bool sync,
	uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 19);
	for (; s.check(); s++)
	{
		string input;
		string output = "Invalid date format";
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
	for (; s.check(); s++)
	{
		string input;
		string output = "Invalid number format";
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
	TCSelector s(num, 3);
	for (; s.check(); s++)
	{
		string input, output;
		switch (s.value())
		{
			case 1: //отсутствуют обязательные параметры
				{
					__tc__("submitDbSmeCmd.incorrect.invalidJob"); __tc_ok__;
					auto_ptr<char> tmp = rand_char(20);
					input = tmp.get();
					output = "Invalid job name";
				}
				break;
			case 2: //отсутствуют обязательные параметры
				__tc__("submitDbSmeCmd.incorrect.missingParams"); __tc_ok__;
				input = "InsertJob1";
				output = "Required parameter is missing";
				break;
			case 3: //присутствуют лишние параметры
				__tc__("submitDbSmeCmd.incorrect.extraParams"); __tc_ok__;
				input = "DateFormatJob1 1/1/02 12:0:0 AM str";
				output = "Extra parameters found";
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendDbSmePdu(input, output, sync, dataCoding);
	}
}

bool DbSmeTestCases::checkPdu(PduDeliverySm &pdu)
{
	__decl_tc__;
	__tc__("processDbSmeRes.dataCoding");
	if (pdu.get_message().get_dataCoding() != DATA_CODING_SMSC_DEFAULT)
	{
		__tc_fail__(1);
		return false;
	}
	__tc_ok_cond__;
	__tc__("processDbSmeRes.serviceType");
	__cfg_str__(dbSmeServiceType);
	if (dbSmeServiceType != pdu.get_message().get_serviceType())
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	__tc__("processDbSmeRes.protocolId");
	__cfg_int__(dbSmeProtocolId);
	if (pdu.get_message().get_protocolId() != dbSmeProtocolId)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	return true;
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
	if (!checkPdu(pdu))
	{
		return;
	}
	const string text = decode(pdu.get_message().get_shortMessage(),
		pdu.get_message().size_shortMessage(), pdu.get_message().get_dataCoding());
	MutexGuard mguard(dbSmeReg->getMutex());
	if (!monitor->pduData->objProps.count("dbSmeRec"))
	{
		simpleTc.processJobOutput(text, NULL, monitor);
		return;
	}
	DbSmeTestRecord* rec = reinterpret_cast<DbSmeTestRecord*>(
		monitor->pduData->objProps["dbSmeRec"]);
	__require__(rec);
	if (rec->getJob().find("DateFormatJob") != string::npos)
	{
		dateFormatTc.processJobOutput(text, rec, monitor);
	}
	else if (rec->getJob() == "OtherFormatJob")
	{
		otherFormatTc.processJobOutput(text, rec, monitor);
	}
	else if (rec->getJob().find("InsertJob") != string::npos)
	{
		insertTc.processJobOutput(text, rec, monitor);
	}
	else if (rec->getJob().find("UpdateJob") != string::npos)
	{
		updateTc.processJobOutput(text, rec, monitor);
	}
	else if (rec->getJob() == "DeleteJob")
	{
		deleteTc.processJobOutput(text, rec, monitor);
	}
	else if (rec->getJob().find("SelectJob") != string::npos)
	{
		selectTc.processJobOutput(text, rec, monitor);
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

