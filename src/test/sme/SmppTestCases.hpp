#ifndef TEST_SME_SMPP_TEST_CASES
#define TEST_SME_SMPP_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "SmppTransmitterTestCases.hpp"
#include "SmppReceiverTestCases.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sme::SmppSession;
using smsc::sme::SmeConfig;
using namespace smsc::test::util; //TCResult, BaseTestCases
using namespace smsc::test::core; //SmeRegistry, PduRegistry, ...

//implemented
const char* const TC_BIND_REGISTERED_SME = "bindRegisteredSme";
const char* const TC_BIND_NON_REGISTERED_SME = "bindNonRegisteredSme";
const char* const TC_UNBIND_BOUNDED = "unbindBounded";
const char* const TC_UNBIND_NON_BOUNDED = "unbindNonBounded";

/**
 * ���� ����� �������� ��� test cases ����������� ��� ������������ sme.
 * @author bryz
 */
class SmppTestCases : BaseTestCases
{
public:
	SmppTestCases(const SmeConfig& config, const SmeSystemId& _systemId,
		const Address& addr, const SmeRegistry* _smeReg,
		const AliasRegistry* _aliasReg, const RouteRegistry* _routeReg,
		ResultHandler* handler); //throws Exception
	
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
	 * Bind ��� sme ������������������ � smsc.
	 */
	TCResult* bindRegisteredSme(int num);

	/**
	 * Bind ��� sme �������������������� � smsc.
	 */
	TCResult* bindNonRegisteredSme(int num);

	/**
	 * �������� ����������� ������������� ��������, ����������� � sms �� ������ sme.
	 */
	TCResult* processInvalidSms();
	
	/**
	 * Unbind ��� sme ����������� � smsc.
	 */
	TCResult* unbindBounded();

	/**
	 * Unbind ��� sme ������������� � smsc.
	 */
	TCResult* unbindNonBounded();

protected:
	virtual Category& getLog();

private:
	const SmeSystemId systemId;
	const Address& smeAddr;
	//external
	const SmeRegistry* smeReg;
	PduRegistry* pduReg;
	const AliasRegistry* aliasReg;
	const RouteRegistry* routeReg;
	ResultHandler* resultHandler;
	//internal
	SmppSession* session;
	RouteChecker* routeChecker;
	SmppResponsePduChecker* responseChecker;
	SmppReceiverTestCases* receiver;
	SmppTransmitterTestCases* transmitter;
};

}
}
}

#endif /* TEST_SME_SMPP_TEST_CASES */
