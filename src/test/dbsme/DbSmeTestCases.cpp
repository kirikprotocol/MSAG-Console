#include "DbSmeTestCases.hpp"
#include "DateFormatter.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "test/conf/TestConfig.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using smsc::util::Logger;
using smsc::test::conf::TestConfig;
using smsc::test::smpp::SmppUtil;
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet;

Category& DbSmeTestCases::getLog()
{
	static Category& log = Logger::getCategory("DbSmeTestCases");
	return log;
}
/**
 * Порядок аргументов: 	int16, int32, flt, dbl, dt, str
 */
void DbSmeTestCases::sendDbSmePdu(PduSubmitSm* pdu,
	const DbSmeTestRecord& rec, bool sync, uint8_t dataCoding,
	PduData::IntProps& intProps)
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
		ostringstream s;
		s << rec.job;
		if (rec.int16)
		{
			s << " " << (int) rec.int16;
		}
		if (rec.int32)
		{
			s << " " << (int) rec.int32;
		}
		if (rec.flt != 0.0)
		{
			s << " " << rec.flt;
		}
		if (rec.dbl != 0.0)
		{
			s << " " << rec.dbl;
		}
		if (rec.dt)
		{
			DateFormatter df(rec.dateFormat);
			s << " " << df.format(rec.dt);
		}
		if (rec.str.length())
		{
			if (rec.quotedString)
			{
				s << " \"" << rec.str << "\"";
			}
			else
			{
				s << " " << rec.str;
			}
		}
		string encText = encode(s.str().c_str(), dataCoding);
		pdu->get_message().set_shortMessage(encText.c_str(), encText.length());
		pdu->get_message().set_dataCoding(dataCoding);
		//зарегистрировать запись
		if (dbSmeReg)
		{
			dbSmeReg->putRecord(rec);
		}
		//отправить и зарегистрировать pdu
		transmitter->sendSubmitSmPdu(pdu, NULL, sync, &intProps, NULL, false);
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		//error();
		throw;
	}
}

int16_t DbSmeTestCases::getRandomInt16()
{
	return rand2(-32000, 32000);
}

int32_t DbSmeTestCases::getRandomInt32()
{
	return rand2(INT_MIN, INT_MAX);
}

float DbSmeTestCases::getRandomFloat()
{
	return rand2(-100, 100) + (float) rand0(99) / 100.0;
}

double DbSmeTestCases::getRandomDouble()
{
	return rand2(-100, 100) + (double) rand0(99) / 100.0;
}

time_t DbSmeTestCases::getRandomTime()
{
	static const int year = 365 * 24 * 3600;
	return time(NULL) + rand2(-year, year);
}

string DbSmeTestCases::getRandomWord()
{
	int len = rand1(150);
	auto_ptr<char> str = rand_char(len);
	return str.get();
}

string DbSmeTestCases::getRandomWords()
{
	int len = rand1(150);
	char str[len + 1];
	rand_char(len, str);
	for (int i = 0; i < len / 5; i++)
	{
		str[rand0(len - 1)] = ' ';
	}
	return str;
}

void DbSmeTestCases::submitCorrectQueryDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 5);
	for (; s.check(); s++)
	{
		__tc__("submitDbSmeCmd.correct.job.select"); __tc_ok__;
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			__cfg_addr__(dbSmeAlias);
			transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
			string text;
			int cmdType;
			PduData::IntProps intProps;
			DbSmeTestRecord rec(dbSmeReg ? dbSmeReg->nextId() : 0);
			intProps["recId"] = rec.id;
			static const string dateFormatJobs[] =
				{"DateFormatJob1", "DateFormatJob2", "DateFormatJob3", "DateFormatJob4"};
			static const string dateFormats[] =
				{"d/M/yy h:m:s t", "dd.MM.yyyy hh-mm-ss t", "d-MMM-yy H m s", "dd MMMM yyyy HH.mm.ss"};
			int dateJobNum = rand0(3);
			switch (s.value())
			{
				case 1: //DateFormatJob с параметром
					__tc__("submitDbSmeCmd.correct.input.date"); __tc_ok__;
					rec.job = dateFormatJobs[dateJobNum];
					rec.dateFormat = dateFormats[dateJobNum];
					rec.dt = getRandomTime();
					break;
				case 2: //DateFormatJob без параметров
					__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.date"); __tc_ok__;
					rec.job = dateFormatJobs[dateJobNum];
					//rec.dt = ...;
					break;
				case 3:
					__tc__("submitDbSmeCmd.correct.input.int"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.input.float"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.input.string"); __tc_ok__;
					rec.job = "OtherFormatJob";
					rec.int16 = getRandomInt16();
					rec.int32 = getRandomInt32();
					rec.flt = getRandomFloat();
					rec.dbl = getRandomDouble();
					//rec.quotedString = false;
					rec.str = getRandomWord();
					break;
				case 4:
					__tc__("submitDbSmeCmd.correct.input.int"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.input.float"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.input.quotedString"); __tc_ok__;
					rec.job = "OtherFormatJob";
					rec.int16 = getRandomInt16();
					rec.int32 = getRandomInt32();
					rec.flt = getRandomFloat();
					rec.dbl = getRandomDouble();
					rec.quotedString = true;
					rec.str = getRandomWords();
					break;
				case 5:
					__tc__("submitDbSmeCmd.correct.input.noParams"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.int"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.float"); __tc_ok__;
					__tc__("submitDbSmeCmd.correct.defaultInput.string"); __tc_ok__;
					rec.job = "OtherFormatJob";
					//rec.int16 = ...;
					//rec.int32 = ...;
					//rec.flt = ...;
					//rec.dbl = ...;
					//rec.str = ...;
					break;
				/*
				case 6:
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			sendDbSmePdu(pdu, rec, sync, dataCoding, intProps);
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void DbSmeTestCases::submitCorrectModifyDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
}

void DbSmeTestCases::submitIncorrectDbSmeCmd(bool sync, uint8_t dataCoding)
{
}

void DbSmeTestCases::processDateFormatJobAck(const string& text,
	const DbSmeTestRecord* rec, PduData* pduData, int dateJobNum)
{
	__decl_tc__;
	__tc__("processDbSmeRes.output.select.singleRecord"); __tc_ok__;
	static const DateFormatter df("w ww  W.WW..d,dd,,ddd/M//MM-MMM--MMMM	MMMMM		y yy yyy yyyy yyyyy h hh hhh m mm mmm t tt");
	static const string prefix("Date: ");
	if (rec->dt)
	{
		__tc__("processDbSmeRes.input.date"); __tc_ok__;
		__tc__("processDbSmeRes.output.dateFormat");
		if (text != (prefix + df.format(rec->dt)))
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
	else
	{
		bool match = false;
		time_t _t = time(NULL);
		tm lt;
		localtime_r(&_t, &lt);
		lt.tm_sec = 0;
		lt.tm_min = 0;
		lt.tm_hour = 0;
		switch (dateJobNum)
		{
			case 1: //now
				__tc__("processDbSmeRes.defaultInput.now"); __tc_ok__;
				__require__(pduData->submitTime <= time(NULL));
				for (time_t t = pduData->submitTime; t <= time(NULL); t++)
				{
					if (match = text == (prefix + df.format(t)))
					{
						break;
					}
				}
				break;
			case 2: //today
				__tc__("processDbSmeRes.defaultInput.today"); __tc_ok__;
				match = text == (prefix + df.format(mktime(&lt)));
				break;
			case 3: //yesterday
				__tc__("processDbSmeRes.defaultInput.yesterday"); __tc_ok__;
				lt.tm_mday -= 1;
				match = text == (prefix + df.format(mktime(&lt)));
				break;
			case 4: //tomorrow
				__tc__("processDbSmeRes.defaultInput.tomorrow"); __tc_ok__;
				lt.tm_mday += 1;
				match = text == (prefix + df.format(mktime(&lt)));
				break;
			default:
				__unreachable__("Invalid dateJobNum");
		}
		__tc__("processDbSmeRes.output.dateFormat");
		if (!match)
		{
			__tc_fail__(dateJobNum);
		}
		__tc_ok_cond__;
	}
}

void DbSmeTestCases::processOtherFormatJobAck(const string& text,
	const DbSmeTestRecord* rec, PduData* pduData)
{
	__decl_tc__;
	__tc__("processDbSmeRes.select.singleRecord"); __tc_ok__;
	ostringstream res;
	__tc__("processDbSmeRes.output.fromAddress"); __tc_ok__;
	//получать алиас адреса отправителя исходной pdu
	__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
	PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
	Address srcAddr;
	SmppUtil::convert(pdu->get_message().get_source(), &srcAddr);
	const Address srcAlias =
		fixture->aliasReg->findAliasByAddress(srcAddr);
	AddressValue addrVal;
	srcAlias.getValue(addrVal);
	res << "from-address: '" << addrVal << "';" << endl;
	string str;
	if (rec->str.length())
	{
		__tc__("processDbSmeRes.input.string"); __tc_ok__;
		str =  rec->str;
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.string"); __tc_ok__;
		str = "xxx";
	}
	res << "string: \"" << str << "\"," << endl;
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
	if (rec->int16)
	{
		__tc__("processDbSmeRes.input.int"); __tc_ok__;
		res << "int16: (--$" << (int) rec->int16 << "--)," << endl;
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.int"); __tc_ok__;
		res << "int16: (--$16--)," << endl;
	}
	if (rec->int32)
	{
		__tc__("processDbSmeRes.input.int"); __tc_ok__;
		res << "int32: (++$" << (int) rec->int32 << "++)," << endl;
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.int"); __tc_ok__;
		res << "int32: (++$32++)," << endl;
	}
	if (rec->flt != 0.0)
	{
		__tc__("processDbSmeRes.input.float"); __tc_ok__;
		res << "float: (**$" << rec->flt << "**)," << endl;
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.float"); __tc_ok__;
		res << "float: (**$16.16**)," << endl;
	}
	if (rec->dbl != 0.0)
	{
		__tc__("processDbSmeRes.input.float"); __tc_ok__;
		res << "double: (::$" << rec->dbl << "::);";
	}
	else
	{
		__tc__("processDbSmeRes.defaultInput.float"); __tc_ok__;
		res << "double: (::$32.32::);";
	}
	__tc__("processDbSmeRes.output");
	if (text != res.str())
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

void DbSmeTestCases::processSmeAcknowledgement(PduData* pduData, PduDeliverySm &pdu)
{
	if (!dbSmeReg)
	{
		return;
	}
	__require__(pduData);
	__decl_tc__;
	PduData::IntProps::const_iterator it = pduData->intProps.find("recId");
	__require__(it != pduData->intProps.end());
	int id = it->second;
	DbSmeTestRecord* rec = dbSmeReg->getRecord(id);
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
	int dateJobNum;
	if (sscanf(rec->job.c_str(), "DateFormatJob%u", &dateJobNum))
	{
		__require__(dateJobNum >= 1 && dateJobNum <= 4);
		processDateFormatJobAck(text, rec, pduData, dateJobNum);
	}
	else if (rec->job == "OtherFormatJob")
	{
		processOtherFormatJobAck(text, rec, pduData);
	}
	else
	{
		__unreachable__("Unsupported job");
	}
	bool res = dbSmeReg->removeRecord(id);
	__require__(res);
}

}
}
}

