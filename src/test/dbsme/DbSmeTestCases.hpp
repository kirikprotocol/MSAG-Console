#ifndef TEST_DBSME_DBSME_TEST_CASES
#define TEST_DBSME_DBSME_TEST_CASES

#include "test/sme/SmppProtocolTestCases.hpp"
#include "DbSmeRegistry.hpp"
#include "DateFormatter.hpp"
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
		DbSmeRegistry* _dbSmeReg)
		: SmppProtocolTestCases(config, fixture), dbSmeReg(_dbSmeReg)
	{
		fixture->ackHandler = this;
		//__require__(dbSmeReg);
	}

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

	const string getOutputFromAddress(const DbSmeTestRecord* rec);
	const string getOutputString(const DbSmeTestRecord* rec);
	const string getOutputDate(const DbSmeTestRecord* rec,
		const DateFormatter& df, int addDays);
	int getOutputInt16(const DbSmeTestRecord* rec);
	int getOutputInt32(const DbSmeTestRecord* rec);
	float getOutputFloat(const DbSmeTestRecord* rec);
	double getOutputDouble(const DbSmeTestRecord* rec);

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
	void processSelectJobAck(const string& text, DbSmeTestRecord* rec,
		SmeAckMonitor* monitor);
	void processSelectNoDefaultJobAck(const string& text,
		DbSmeTestRecord* rec, SmeAckMonitor* monitor);
};

}
}
}

#endif /* TEST_DBSME_DBSME_TEST_CASES */

