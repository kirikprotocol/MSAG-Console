#ifndef TEST_DBSME_DBSME_TEST_CASES
#define TEST_DBSME_DBSME_TEST_CASES

#include "test/sme/SmppProfilerTestCases.hpp"
#include "DbSmeRegistry.hpp"
#include "DbSmeFormatJobTestCases.hpp"
#include "DbSmeModifyJobTestCases.hpp"
#include "DbSmeSelectJobTestCases.hpp"
#include <string>

namespace smsc {
namespace test {
namespace dbsme {

using std::string;
using log4cpp::Category;
using smsc::sms::Address;
using smsc::sme::SmeConfig;
using smsc::smpp::PduSubmitSm;
using smsc::smpp::PduDeliverySm;
using smsc::test::sme::SmppFixture;
using smsc::test::sme::SmppProfilerTestCases;
using smsc::test::sme::SmeAcknowledgementHandler;
using smsc::test::core::SmeAckMonitor;
using smsc::test::core::PduData;

/**
 * Тест кейсы для db sme.
 */
class DbSmeTestCases : public SmppProfilerTestCases
{
public:
	DbSmeTestCases(const SmeConfig& config, SmppFixture* fixture,
		DbSmeRegistry* _dbSmeReg);

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
	 * Отправка команды db sme с неправильным форматом даты.
	 */
	void submitIncorrectDateFormatDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка команды db sme с неправильным форматом чисел.
	 */
	void submitIncorrectNumberFormatDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка команды db sme с неправильными параметрами.
	 */
	void submitIncorrectParamsDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Получение ответных сообщений от db sme.
	 */
	virtual void processSmeAcknowledgement(SmeAckMonitor* monitor,
		PduDeliverySm &pdu, time_t recvTime);

protected:
	DbSmeRegistry* dbSmeReg;
	DbSmeDateFormatJobTestCases dateFormatTc;
	DbSmeOtherFormatJobTestCases otherFormatTc;
	DbSmeInsertJobTestCases insertTc;
	DbSmeUpdateJobTestCases updateTc;
	DbSmeDeleteJobTestCases deleteTc;
	DbSmeSelectJobTestCases selectTc;

	virtual Category& getLog();
	const string getFromAddress();
	const string getToAddress();
	const string getCmdText(DbSmeTestRecord* rec, const DateFormatter* df);
	void sendDbSmePdu(const Address& addr, const string& input,
		PduData::IntProps* intProps, PduData::StrProps* strProps,
		PduData::ObjProps* objProps, bool sync, uint8_t dataCoding);
	void sendDbSmePdu(const string& text, DbSmeTestRecord* rec,
		bool sync, uint8_t dataCoding);
	void sendDbSmePdu(const string& text, const string& output,
		bool sync, uint8_t dataCoding);
	void sendDbSmePdu(const Address& addr, const string& input,
		const string& output, bool sync, uint8_t dataCoding);
	const string processJobFirstOutput(const string& text, DbSmeTestRecord* rec);
	AckText* getExpectedResponse(SmeAckMonitor* monitor, PduDeliverySm &pdu,
		const string& text);
};

}
}
}

#endif /* TEST_DBSME_DBSME_TEST_CASES */

