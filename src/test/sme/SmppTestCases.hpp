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
using namespace smsc::test::util; //BaseTestCases, CheckList
using namespace smsc::test::core; //SmeRegistry, PduRegistry, ...

/**
 * Ётот класс содержит все test cases необходимые дл€ тестировани€ sme.
 * @author bryz
 */
class SmppTestCases : BaseTestCases
{
public:
	SmppTestCases(const SmeConfig& config, const SmeSystemId& systemId,
		const Address& smeAddr, const SmeRegistry* smeReg,
		const AliasRegistry* aliasReg, const RouteRegistry* routeReg,
		CheckList* chkList); //throws Exception
	
	virtual ~SmppTestCases();

	/**
	 * “ест кейсы дл€ smpp receiver.
	 */
	SmppReceiverTestCases& getReceiver();

	/**
	 * “ест кейсы дл€ smpp transmitter.
	 */
	SmppTransmitterTestCases& getTransmitter();
	
	/**
	 * Bind sme зарегистрированной в smsc.
	 */
	void bindCorrectSme(int num);

	/**
	 * Bind sme с неправильными параметрами.
	 */
	void bindIncorrectSme(int num);
	
	/**
	 * ¬се подтверждений доставки, нотификации и sms доставл€ютс€ и не тер€ютс€.
	 */
	void checkMissingPdu();
	
	/**
	 * Unbind дл€ sme.
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
