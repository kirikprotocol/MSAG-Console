#ifndef TEST_SME_SMPP_TEST_CASES
#define TEST_SME_SMPP_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "SmppTransmitterTestCases.hpp"
#include "SmppReceiverTestCases.hpp"
#include "SmppResponseSender.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sme::SmppSession;
using smsc::sme::SmeConfig;
using namespace smsc::test::util; //BaseTestCases, CheckList
using namespace smsc::test::core; //SmeRegistry, PduRegistry, ...

/**
 * ���� ����� �������� ��� test cases ����������� ��� ������������ sme.
 * @author bryz
 */
class SmppTestCases : BaseTestCases
{
public:
	SmppTestCases(const SmeConfig& config, const SmeSystemId& systemId,
		const Address& smeAddr, SmppResponseSender* respSender,
		const SmeRegistry* smeReg, const AliasRegistry* aliasReg,
		const RouteRegistry* routeReg, CheckList* chkList); //throws Exception
	
	virtual ~SmppTestCases();

	/**
	 * ���� ����� ��� smpp receiver.
	 */
	SmppReceiverTestCases& getReceiver();

	/**
	 * ���� ����� ��� smpp transmitter.
	 */
	SmppTransmitterTestCases& getTransmitter();
	
	/**
	 * Bind sme ������������������ � smsc.
	 */
	bool bindCorrectSme(int num);

	/**
	 * Bind sme � ������������� �����������.
	 */
	void bindIncorrectSme(int num);
	
	/**
	 * ��� ������������� ��������, ����������� � sms ������������ � �� ��������.
	 */
	void checkMissingPdu();
	
	/**
	 * Unbind ��� sme.
	 */
	void unbind();

protected:
	virtual Category& getLog();

private:
	const SmeConfig config;
	const SmeSystemId systemId;
	const Address smeAddr;
	//external
	const SmeRegistry* smeReg;
	PduRegistry* pduReg;
	const AliasRegistry* aliasReg;
	const RouteRegistry* routeReg;
	//internal
	SmppSession* session;
	RouteChecker* routeChecker;
	SmppPduChecker* pduChecker;
	SmppReceiverTestCases* receiver;
	SmppTransmitterTestCases* transmitter;
	CheckList* chkList;

	void checkSubmitTime(time_t checkTime);
	void checkWaitTime(time_t checkTime);
	void checkValidTime(time_t checkTime);
};

}
}
}

#endif /* TEST_SME_SMPP_TEST_CASES */
