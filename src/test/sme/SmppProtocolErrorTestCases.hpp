#ifndef TEST_SME_SMPP_PROTOCOL_ERROR_TEST_CASES
#define TEST_SME_SMPP_PROTOCOL_ERROR_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "test/smpp/TestSmppSession.hpp"
#include "SmppFixture.hpp"
#include "smpp/smpp.h"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sms::Address;
using smsc::sme::SmeConfig;
using smsc::sme::SmppPduEventListener;
using smsc::test::smpp::TestSmppSession;
using smsc::test::util::TestCase;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::core::synchronization::Event;
using namespace smsc::smpp; //constants, pdu

/**
 * ������� ����� ���������.
 */
class SmppProtocolErrorScenario : public SmppPduEventListener
{
protected:
	TestCase* tc;
	bool isOk;
	TestSmppSession sess;
	const SmeConfig cfg;
	const Address smeAddr;
	Event event;
	bool complete;
	CheckList* chkList;
	
public:
	SmppProtocolErrorScenario(const SmeConfig& conf, const Address& addr,
		CheckList* _chkList)
	: tc(NULL), isOk(true), sess(this), cfg(conf), smeAddr(addr),
		complete(false), chkList(_chkList) {}
	virtual ~SmppProtocolErrorScenario() {}

	void connect();
	void close();
	void sendPdu(SmppHeader* pdu);

	bool checkComplete(int timeout);
	void setComplete(bool val);
	
	void checkBindResp(PduBindTRXResp* pdu);
	void checkUnbindResp(PduUnbindResp* pdu);

	SmppHeader* createPdu(uint32_t commandId);
	SmppHeader* setupBindPdu(PduBindTRX& pdu);
	SmppHeader* setupUnbindPdu(PduUnbind& pdu);
	SmppHeader* setupSubmitSmPdu(PduSubmitSm& pdu);
	SmppHeader* setupDeliverySmRespPdu(PduDeliverySmResp& pdu, uint32_t seqNum);
	SmppHeader* setupGenericNackPdu(PduGenericNack& pdu, uint32_t seqNum);
	
	virtual void execute() = NULL;
	virtual void handleEvent(SmppHeader* pdu) = NULL;
	virtual void handleError(int errorCode) = NULL;
};

/**
 * ���� ����� �������� ��� test cases ����������� ��� ������������ ���������
 * �������� smpp ���������.
 * @author bryz
 */
class SmppProtocolErrorTestCases : public BaseTestCases
{
public:
	SmppProtocolErrorTestCases(const SmeConfig& conf, const Address& addr,
		CheckList* chkList);
	virtual ~SmppProtocolErrorTestCases() {}

	/**
	 * ��������������� ���������� � SC � �������� ������������ bind pdu.
	 */
	void invalidBindScenario(int num);

	/**
	 * ��������������� ���������� � SC, ���������� bind pdu � ��������
	 * ������������ pdu.
	 */
	void invalidPduScenario(int num);

	/**
	 * �������� ���������� submit_sm � ����������� sequence_number.
	 */
	void equalSequenceNumbersScenario();

	/**
	 * �������� submit_sm ����� unbind.
	 */
	void submitAfterUnbindScenario(int num);

	/**
	 * �������� ������������ pdu � ������� ������.
	 * �������� ������ �����������, �������� ���������� - �������� SC.
	 */
	void nullPduScenario(int num);

protected:
	const SmeConfig cfg;
	const Address smeAddr;
	CheckList* chkList;

	virtual Category& getLog();
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_ERROR_TEST_CASES */

