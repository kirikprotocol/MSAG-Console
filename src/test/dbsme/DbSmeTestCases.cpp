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

DbSmeTestCases::DbSmeTestCases(const SmeConfig& config, SmppFixture* fixture,
	DbSmeRegistry* _dbSmeReg)
: SmppProtocolTestCases(config, fixture), dbSmeReg(_dbSmeReg),
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
	__cfg_addr__(dbSmeAlias);
	AddressValue addrVal;
	dbSmeAlias.getValue(addrVal);
	return addrVal;
}

/**
 * ѕор€док аргументов: id, int16, int32, flt, dbl, str, dt
 */
void DbSmeTestCases::sendDbSmePdu(DbSmeTestRecord* rec,
	const DateFormatter* df, bool sync, uint8_t dataCoding)
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
		//установить from-address и to-address
		rec->setFromAddr(getFromAddress());
		rec->setToAddr(getToAddress());
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
		//создать pdu
		PduSubmitSm* pdu = new PduSubmitSm();
		__cfg_addr__(dbSmeAlias);
		transmitter->setupRandomCorrectSubmitSmPdu(pdu, dbSmeAlias);
		//установить немедленную доставку
		pdu->get_message().set_scheduleDeliveryTime("");
		//текст сообщени€
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

void DbSmeTestCases::submitCorrectFormatDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 4);
	for (; s.check(); s++)
	{
		try
		{
			const DateFormatter* df = NULL;
			DbSmeTestRecord* rec;
			switch (s.value())
			{
				case 1: //DateFormatJob с параметром
					rec = dateFormatTc.createJobInput(rand1(4), true, &df);
					break;
				case 2: //DateFormatJob без параметров
					rec = dateFormatTc.createJobInput(rand1(4), false, &df);
					break;
				case 3: //OtherFormatJob с параметрами
					rec = otherFormatTc.createJobInput(true);
					break;
				case 4: //OtherFormatJob без параметров
					rec = otherFormatTc.createJobInput(false);
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendDbSmePdu(rec, df, sync, dataCoding);
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
	TCSelector s(num, 3);
	for (; s.check(); s++)
	{
		try
		{
			DbSmeTestRecord* rec;
			switch (s.value())
			{
				case 1: //SelectJob с нул€ми
					rec = selectTc.createSelectNullsJobInput();
					break;
				case 2: //SelectJob со значени€ми
					rec = selectTc.createSelectValuesJobInput();
					break;
				case 3: //SelectJob без дефолтных output значений
					rec = selectTc.createSelectNoDefaultsJobInput();
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendDbSmePdu(rec, NULL, sync, dataCoding);
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void DbSmeTestCases::submitCorrectInsertDbSmeCmd(bool sync, uint8_t dataCoding, int num)
{
	__decl_tc__;
	TCSelector s(num, 6);
	for (; s.check(); s++)
	{
		try
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
				case 3: //InsertJob без параметров
					rec = insertTc.createDefaultsJobInput();
					break;
				case 4: //InsertJob с нул€ми
					rec = insertTc.createZerosJobInput();
					break;
				case 5: //InsertJob с нулами
					rec = insertTc.createNullsJobInput();
					break;
				case 6: //InsertJob с дублирующимс€ ключом
					rec = insertTc.createDuplicateKeyJobInput();
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendDbSmePdu(rec, &df, sync, dataCoding);
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
	TCSelector s(num, 2);
	for (; s.check(); s++)
	{
		try
		{
			static const DateFormatter df = updateTc.getDateFormatter();
			DbSmeTestRecord* rec;
			switch (s.value())
			{
				case 1: //UpdateJob с параметрами
					rec = updateTc.createUpdateRecordJobInput();
					break;
				case 2: //UpdateJob с дублирующимс€ ключом
					rec = updateTc.createDuplicateKeyJobInput();
					break;
				default:
					__unreachable__("Invalid num");
			}
			sendDbSmePdu(rec, &df, sync, dataCoding);
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
	try
	{
		DbSmeTestRecord* rec = deleteTc.createDeleteAllJobInput();
		sendDbSmePdu(rec, NULL, sync, dataCoding);
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
	__require__(monitor->pduData->objProps.count("dbSmeRec"));
	DbSmeTestRecord* rec = reinterpret_cast<DbSmeTestRecord*>(monitor->pduData->objProps["dbSmeRec"]);
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
	MutexGuard mguard(dbSmeReg->getMutex());
	if (rec->getJob().find("DateFormatJob") != string::npos)
	{
		dateFormatTc.processJobOutput(text, rec, monitor);
	}
	else if (rec->getJob() == "OtherFormatJob")
	{
		otherFormatTc.processJobOutput(text, rec, monitor);
	}
	else if (rec->getJob() == "InsertJob")
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

