#ifndef TEST_SME_SMPP_BASE_TEST_CASES
#define TEST_SME_SMPP_BASE_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sme::SmeConfig;
using smsc::sme::SmppPduEventListener;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;

/**
 * Этот класс содержит базовые тест кейсы необходимые для тестирования sme.
 * @author bryz
 */
class SmppBaseTestCases : BaseTestCases
{
public:
	SmppBaseTestCases(const SmeConfig& _config, SmppFixture* _fixture)
		: config(_config), fixture(_fixture), chkList(fixture->chkList) {}
	
	virtual ~SmppBaseTestCases() {}

	/**
	 * Bind sme зарегистрированной в smsc.
	 */
	bool bindCorrectSme(int num);

	/**
	 * Bind sme с неправильными параметрами.
	 */
	void bindIncorrectSme(int num);
	
	/**
	 * Все подтверждений доставки, нотификации и sms доставляются и не теряются.
	 */
	void checkMissingPdu();
	
	/**
	 * Unbind для sme.
	 */
	void unbind();

protected:
	virtual Category& getLog();

private:
	const SmeConfig config;
	SmppFixture* fixture;
	CheckList* chkList;

	void checkSubmitTime(time_t checkTime);
	void checkWaitTime(time_t checkTime);
	void checkValidTime(time_t checkTime);

	struct FakeReceiver : public SmppPduEventListener
	{
		virtual void handleEvent(SmppHeader *pdu) {}
		virtual void handleError(int errorCode) {}
	};
};

}
}
}

#endif /* TEST_SME_SMPP_BASE_TEST_CASES */

