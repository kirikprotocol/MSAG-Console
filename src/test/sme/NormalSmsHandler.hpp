#ifndef TEST_SME_NORMAL_SMS_HANDLER
#define TEST_SME_NORMAL_SMS_HANDLER

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::smpp::PduSubmitSm;
using smsc::smpp::PduDeliverySm;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::test::core::PduRegistry;
using smsc::test::core::DeliveryMonitor;

/**
 * —ообщени€ правильно доставл€ютс€ от одного sme другому.
 * @author bryz
 */
class NormalSmsHandler : public BaseTestCases, public PduHandler
{
public:
	NormalSmsHandler(SmppFixture* _fixture)
	: fixture(_fixture), chkList(_fixture->chkList) {}
	
	virtual ~NormalSmsHandler() {}

	virtual void processPdu(PduDeliverySm& pdu, time_t recvTime)
	{
		__unreachable__("use processPdu(PduDeliverySm&, const Address, time_t)");
	}

	virtual void processPdu(PduDeliverySm& pdu, const Address origAddr,
		time_t recvTime);

protected:
	SmppFixture* fixture;
	CheckList* chkList;
	
	virtual Category& getLog();
	void compareMsgText(PduSubmitSm& origPdu, PduDeliverySm& pdu);
	void updateDeliveryReceiptMonitor(DeliveryMonitor* monitor,
		PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime,
		uint32_t smeTimeout);
};

}
}
}

#endif /* TEST_SME_NORMAL_SMS_HANDLER */
