#ifndef TEST_DBSME_DBSME_TEST_CASES
#define TEST_DBSME_DBSME_TEST_CASES

#include "test/sme/SmppProtocolTestCases.hpp"
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
	DbSmeTestCases(const SmeConfig& config, SmppFixture* fixture)
		: SmppProtocolTestCases(config, fixture)
	{
		fixture->ackHandler = this;
	}

	virtual ~DbSmeTestCases() {}

	/**
	 * Отправка правильной команды db sme.
	 */
	void submitCorrectDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * Отправка неправильной команды db sme.
	 */
	void submitIncorrectDbSmeCmd(bool sync, uint8_t dataCoding);

	/**
	 * Получение ответных сообщений от db sme.
	 */
	virtual void processSmeAcknowledgement(PduData* pduData, PduDeliverySm &pdu);

protected:
	virtual Category& getLog();
};

}
}
}

#endif /* TEST_DBSME_DBSME_TEST_CASES */

