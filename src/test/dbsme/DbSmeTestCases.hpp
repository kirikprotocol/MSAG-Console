#ifndef TEST_DBSME_DBSME_TEST_CASES
#define TEST_DBSME_DBSME_TEST_CASES

#include "test/sme/SmppProtocolTestCases.hpp"
#include "DbSmeRegistry.hpp"
#include "DateFormatter.hpp"
#include "DbSmeFormatJobTestCases.hpp"
#include "DbSmeModifyJobTestCases.hpp"
#include "DbSmeSelectJobTestCases.hpp"
#include <string>

namespace smsc {
namespace test {
namespace dbsme {

using std::string;
using log4cpp::Category;
using smsc::sme::SmeConfig;
using smsc::smpp::PduSubmitSm;
using smsc::smpp::PduDeliverySm;
using smsc::test::sme::SmppFixture;
using smsc::test::sme::SmppProtocolTestCases;
using smsc::test::sme::SmeAcknowledgementHandler;
using smsc::test::core::SmeAckMonitor;

/**
 * ���� ����� ��� db sme.
 */
class DbSmeTestCases : public SmppProtocolTestCases,
	public SmeAcknowledgementHandler
{
public:
	DbSmeTestCases(const SmeConfig& config, SmppFixture* fixture,
		DbSmeRegistry* _dbSmeReg);

	virtual ~DbSmeTestCases() {}

	/**
	 * �������� ���������� ������� db sme �� �������� ��������������.
	 */
	void submitCorrectFormatDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * �������� ���������� ������� db sme �� ������� ������.
	 */
	void submitCorrectSelectDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * �������� ���������� ������� db sme �� ���������� ������� � �������.
	 */
	void submitCorrectInsertDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * �������� ���������� ������� db sme �� ��������� �������.
	 */
	void submitCorrectUpdateDbSmeCmd(bool sync, uint8_t dataCoding, int num);

	/**
	 * �������� ���������� ������� db sme �� �������� ���� �������.
	 */
	void submitCorrectDeleteDbSmeCmd(bool sync, uint8_t dataCoding);

	/**
	 * �������� ������������ ������� db sme.
	 */
	void submitIncorrectDbSmeCmd(bool sync, uint8_t dataCoding);

	/**
	 * ��������� �������� ��������� �� db sme.
	 */
	virtual void processSmeAcknowledgement(SmeAckMonitor* monitor,
		PduDeliverySm &pdu);

protected:
	DbSmeRegistry* dbSmeReg;
	DbSmeDateFormatJobTestCases dateFormatTc;
	DbSmeOtherFormatJobTestCases otherFormatTc;
	DbSmeInsertJobTestCases insertTc;
	DbSmeUpdateJobTestCases updateTc;
	DbSmeDeleteJobTestCases deleteTc;
	DbSmeSelectJobTestCases selectTc;
	DbSmeSelectNoDefaultJobTestCases selectNoDefaultTc;

	virtual Category& getLog();
	const string getFromAddress();
	void sendDbSmePdu(DbSmeTestRecord* rec, const DateFormatter* df,
		bool sync, uint8_t dataCoding);
};

}
}
}

#endif /* TEST_DBSME_DBSME_TEST_CASES */

