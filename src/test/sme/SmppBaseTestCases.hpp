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
 * ���� ����� �������� ������� ���� ����� ����������� ��� ������������ sme.
 * @author bryz
 */
class SmppBaseTestCases : BaseTestCases
{
public:
	SmppBaseTestCases(const SmeConfig& _config, SmppFixture* _fixture)
		: config(_config), fixture(_fixture), chkList(fixture->chkList)
	{ fixture->base = this; }
	
	virtual ~SmppBaseTestCases() {}

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
	SmppFixture* fixture;
	CheckList* chkList;

	void checkMissingPdu(time_t checkTime);

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

