#ifndef TEST_SME_NORMAL_SMS_HANDLER
#define TEST_SME_NORMAL_SMS_HANDLER

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"
#include "util/debug.h"
#include <vector>

namespace smsc {
namespace test {
namespace sme {

using std::vector;
using log4cpp::Category;
using smsc::smpp::SmppHeader;
using smsc::smeman::SmeInfo;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::test::core::PduFlag;
using smsc::test::core::PduRegistry;
using smsc::test::core::DeliveryMonitor;

/**
 * —ообщени€ правильно доставл€ютс€ от одного sme другому.
 * @author bryz
 */
class NormalSmsHandler : public BaseTestCases, public PduHandler
{
public:
	NormalSmsHandler(SmppFixture* fixture);
	
	virtual ~NormalSmsHandler() {}

	virtual void processPdu(SmppHeader* header, time_t recvTime)
	{
		__unreachable__("Use processPdu(SmppHeader*, const Address, time_t)");
	}

	virtual void processPdu(SmppHeader* header, const Address& origAddr,
		time_t recvTime);

protected:
	SmppFixture* fixture;
	CheckList* chkList;

	virtual Category& getLog();
	vector<int> checkRoute(SmppHeader* header1, SmppHeader* header2) const;

	void checkNotMapMsgText(DeliveryMonitor* monitor, SmppHeader* header);

	PduFlag checkSegmentedMapMsgText(DeliveryMonitor* monitor,
		SmppHeader* header, RespPduFlag respFlag, uint8_t dataCoding,
		SmsMsg* msg, int concatRefNum, int concatMaxNum, int concatSeqNum);
	PduFlag checkSimpleMapMsgText(DeliveryMonitor* monitor, SmppHeader* header,
		RespPduFlag respFlag, uint8_t dataCoding, SmsMsg* msg);
	PduFlag checkMapMsgText(DeliveryMonitor* monitor, SmppHeader* header,
		RespPduFlag respFlag);

	void registerIntermediateNotificationMonitor(const DeliveryMonitor* monitor,
		PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime,
		time_t respTime);
	void registerDeliveryReceiptMonitor(const DeliveryMonitor* monitor,
		PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime,
		time_t respTime);
	void registerDeliveryReportMonitors(const DeliveryMonitor* monitor,
		PduRegistry* pduReg, uint32_t deliveryStatus, time_t recvTime,
		time_t respTime);
};

}
}
}

#endif /* TEST_SME_NORMAL_SMS_HANDLER */
