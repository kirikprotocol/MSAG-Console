#ifndef TEST_DBSME_DBSME_TEST_CASES
#define TEST_DBSME_DBSME_TEST_CASES

#include "test/sme/SmppProtocolTestCases.hpp"
#include "DbSmeRegistry.hpp"
#include <string>

namespace smsc {
namespace test {
namespace dbsme {

using std::string;
using log4cpp::Category;
using smsc::sme::SmeConfig;
using smsc::test::sme::SmppFixture;
using smsc::test::sme::SmppProtocolTestCases;
using smsc::test::sme::SmeAcknowledgementHandler;
using smsc::test::core::PduData;
using namespace smsc::smpp; //pdu

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
	 * Отправка правильной команды db sme на выборку данных.
	 */
	void submitCorrectQueryDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка правильной команды db sme на изменение данных.
	 */
	void submitCorrectModifyDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка неправильной команды db sme.
	 */
	void submitIncorrectDbSmeCmd(bool sync, uint8_t dataCoding);

	/**
	 * Получение ответных сообщений от db sme.
	 */
	virtual void processSmeAcknowledgement(PduData* pduData, PduDeliverySm &pdu);

protected:
	DbSmeRegistry* dbSmeReg;

	virtual Category& getLog();
	void sendDbSmePdu(PduSubmitSm* pdu, const DbSmeTestRecord& rec,
		bool sync, uint8_t dataCoding, PduData::IntProps& intProps);
	int16_t getRandomInt16();
	int32_t getRandomInt32();
	float getRandomFloat();
	double getRandomDouble();
	time_t getRandomTime();
	string getRandomWord();
	string getRandomWords();
	void processDateFormatJobAck(const string& text, const DbSmeTestRecord* rec,
		time_t submitTime, int dateJobNum);
	void processOtherFormatJobAck(const string& text, const DbSmeTestRecord* rec);
};

}
}
}

#endif /* TEST_DBSME_DBSME_TEST_CASES */

