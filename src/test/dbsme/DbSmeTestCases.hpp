#ifndef TEST_DBSME_DBSME_TEST_CASES
#define TEST_DBSME_DBSME_TEST_CASES

#include "test/sme/SmppProtocolTestCases.hpp"
#include "DbSmeRegistry.hpp"
#include "DateFormatter.hpp"
#include <string>
#include <ostream>

namespace smsc {
namespace test {
namespace dbsme {

using std::string;
using std::ostream;
using log4cpp::Category;
using smsc::sme::SmeConfig;
using smsc::smpp::PduSubmitSm;
using smsc::smpp::PduDeliverySm;
using smsc::test::sme::SmppFixture;
using smsc::test::sme::SmppProtocolTestCases;
using smsc::test::sme::SmeAcknowledgementHandler;
using smsc::test::core::SmeAckMonitor;

/**
 * Тест кейсы для db sme.
 */
class DbSmeTestCases : public SmppProtocolTestCases,
	public SmeAcknowledgementHandler
{
public:
	DbSmeTestCases(const SmeConfig& config, SmppFixture* fixture,
		DbSmeRegistry* _dbSmeReg)
		: SmppProtocolTestCases(config, fixture), dbSmeReg(_dbSmeReg)
	{
		fixture->ackHandler = this;
		//__require__(dbSmeReg);
	}

	virtual ~DbSmeTestCases() {}

	/**
	 * Отправка правильной команды db sme на проверку форматирования.
	 */
	void submitCorrectFormatDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка правильной команды db sme на выборку данных.
	 */
	void submitCorrectSelectDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка правильной команды db sme на добавление записей в таблицу.
	 */
	void submitCorrectInsertDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка правильной команды db sme на изменение записей.
	 */
	void submitCorrectUpdateDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка правильной команды db sme на удаление всех записей.
	 */
	void submitCorrectDeleteDbSmeCmd(bool sync, uint8_t dataCoding);

	/**
	 * Отправка неправильной команды db sme.
	 */
	void submitIncorrectDbSmeCmd(bool sync, uint8_t dataCoding);

	/**
	 * Получение ответных сообщений от db sme.
	 */
	virtual void processSmeAcknowledgement(SmeAckMonitor* monitor,
		PduDeliverySm &pdu);

protected:
	DbSmeRegistry* dbSmeReg;

	virtual Category& getLog();
	const string getFromAddress();
	void sendDbSmePdu(PduSubmitSm* pdu, DbSmeTestRecord* rec,
		const DateFormatter* df, bool sync, uint8_t dataCoding);

	void setInputInt16(DbSmeTestRecord* rec, int val);
	void setRandomInputInt16(DbSmeTestRecord* rec);
	void setInputInt32(DbSmeTestRecord* rec, int val);
	void setRandomInputInt32(DbSmeTestRecord* rec);
	void setInputFloat(DbSmeTestRecord* rec, double val);
	void setRandomInputFloat(DbSmeTestRecord* rec);
	void setInputDouble(DbSmeTestRecord* rec, double val);
	void setRandomInputDouble(DbSmeTestRecord* rec);
	void setInputDate(DbSmeTestRecord* rec, time_t val);
	void setRandomInputDate(DbSmeTestRecord* rec);
	void setInputString(DbSmeTestRecord* rec, const string& val);
	void setInputQuotedString(DbSmeTestRecord* rec, const string& val);
	void setRandomInputString(DbSmeTestRecord* rec, bool quotedString);

	DbSmeTestRecord* getInsertJobDefaultInput();
	DbSmeTestRecord* getUpdateJob1DefaultInput();

	const string getOutputFromAddress(const DbSmeTestRecord* rec);
	const string getOutputString(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
	time_t getDate(DateType dtType, time_t now = 0);
	const string getOutputDate(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, const DateFormatter& df, bool& res);
	int getOutputInt16(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
	int getOutputInt32(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
	float getOutputFloat(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);
	double getOutputDouble(const DbSmeTestRecord* rec,
		const DbSmeTestRecord* defOutput, bool& res);

	void processDateFormatJobAck(const string& text, DbSmeTestRecord* rec,
		SmeAckMonitor* monitor, int dateJobNum);
	void processOtherFormatJobAck(const string& text, DbSmeTestRecord* rec,
		SmeAckMonitor* monitor);
	void processInsertJobAck(const string& text, DbSmeTestRecord* rec,
		SmeAckMonitor* monitor);
	void processUpdateOkJobAck(const string& text, DbSmeTestRecord* rec,
		SmeAckMonitor* monitor);
	void processUpdateDuplicateJobAck(const string& text,
		DbSmeTestRecord* rec, SmeAckMonitor* monitor);
	void processDeleteJobAck(const string& text, DbSmeTestRecord* rec,
		SmeAckMonitor* monitor);
	void writeSelectJobRecord(ostream& os, DbSmeTestRecord* rec,
		DbSmeTestRecord* defOutput, time_t now, bool& res);
	void processSelectJobAck(const string& text, DbSmeTestRecord* rec,
		SmeAckMonitor* monitor);
	void processSelectNoDefaultJobAck(const string& text,
		DbSmeTestRecord* rec, SmeAckMonitor* monitor);
};

}
}
}

#endif /* TEST_DBSME_DBSME_TEST_CASES */

