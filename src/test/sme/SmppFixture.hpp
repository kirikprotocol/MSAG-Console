#ifndef TEST_SME_SMPP_FIXTURE
#define TEST_SME_SMPP_FIXTURE

#include "sme/SmppBase.hpp"
#include "profiler/profiler.hpp"
#include "test/core/AliasRegistry.hpp"
#include "test/core/PduRegistry.hpp"
#include "test/core/SmeRegistry.hpp"
#include "test/core/ProfileRegistry.hpp"
#include "test/core/RouteRegistry.hpp"
#include "test/core/RouteChecker.hpp"
#include "test/sms/SmsUtil.hpp"
#include "SmppPduChecker.hpp"
#include <map>

namespace smsc {
namespace test {
namespace sme {

using std::map;
using smsc::sme::SmppSession;
using smsc::smeman::SmeSystemId;
using smsc::sms::Address;
using smsc::profiler::Profile;
using smsc::test::core::AliasRegistry;
using smsc::test::core::SmeAckMonitor;
using smsc::test::core::PduRegistry;
using smsc::test::core::RouteRegistry;
using smsc::test::core::SmeRegistry;
using smsc::test::core::ProfileRegistry;
using smsc::test::core::RouteChecker;
using smsc::test::core::PduDataObject;
using smsc::test::sms::ltAddress;
using smsc::test::util::CheckList;

struct SmppResponseSender
{
	virtual uint32_t sendDeliverySmResp(PduDeliverySm& pdu) = NULL;
};

struct SmeAcknowledgementHandler
{
	virtual void processSmeAcknowledgement(SmeAckMonitor* monitor,
		PduDeliverySm &pdu, time_t recvTime) = NULL;
};

struct AckText : public PduDataObject
{
	string text;
	uint8_t dataCoding;
	bool valid;
	AckText(const string& _text, uint8_t _dataCoding, bool _valid)
		: text(_text), dataCoding(_dataCoding), valid(_valid) {}
};

class SmppBaseTestCases;
class SmppTransmitterTestCases;
class SmppReceiverTestCases;

struct SmppFixture
{
	SmppSession* session;
	const SmeSystemId systemId;
	const Address smeAddr;
	SmppResponseSender* respSender;
	map<const Address, SmeAcknowledgementHandler*, ltAddress> ackHandler;
	const SmeRegistry* smeReg;
	const AliasRegistry* aliasReg;
	const RouteRegistry* routeReg;
	ProfileRegistry* profileReg;
	PduRegistry* pduReg;
	CheckList* chkList;
	RouteChecker* routeChecker;
	SmppPduChecker* pduChecker;
	SmppBaseTestCases* base;
	SmppTransmitterTestCases* transmitter;
	SmppReceiverTestCases* receiver;

	SmppFixture(const SmeSystemId& _systemId, const Address& _smeAddr,
		SmppResponseSender* _respSender, const SmeRegistry* _smeReg,
		const AliasRegistry* _aliasReg, const RouteRegistry* _routeReg,
		ProfileRegistry* _profileReg, CheckList* _chkList)
	: session(NULL), systemId(_systemId), smeAddr(_smeAddr),
	respSender(_respSender), smeReg(_smeReg), aliasReg(_aliasReg),
	routeReg(_routeReg), pduReg(smeReg->getPduRegistry(smeAddr)),
	profileReg(_profileReg), chkList(_chkList), routeChecker(NULL),
	pduChecker(NULL), base(NULL), transmitter(NULL), receiver(NULL)
	{
		//__require__(respSender);
		__require__(smeReg);
		//__require__(aliasReg);
		//__require__(routeReg);
		//__require__(profileReg);
		//__require__(chkList);
		if (pduReg && aliasReg && routeReg)
		{
			routeChecker = new RouteChecker(systemId, smeAddr, smeReg, aliasReg, routeReg);
			pduChecker = new SmppPduChecker(pduReg, routeChecker, chkList);
		}
	}
	~SmppFixture()
	{
		delete pduChecker;
		delete routeChecker;
		if (session)
		{
			try
			{
				session->close();
			}
			catch(...)
			{
				//nothing
			}
			delete session;
		}
	}

};

}
}
}

#endif /* TEST_SME_SMPP_FIXTURE */

