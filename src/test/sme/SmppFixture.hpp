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
#include "test/util/CheckList.hpp"
#include "SmppPduSender.hpp"
#include <map>

namespace smsc {
namespace test {
namespace sme {

using std::map;
using std::pair;
using std::string;
using smsc::sme::SmppSession;
using smsc::smeman::SmeInfo;
using smsc::sms::Address;
using smsc::smpp::SmppHeader;
using smsc::profiler::Profile;
using smsc::test::core::AliasRegistry;
using smsc::test::core::SmeAckMonitor;
using smsc::test::core::PduRegistry;
using smsc::test::core::RouteRegistry;
using smsc::test::core::SmeType;
using smsc::test::core::SmeRegistry;
using smsc::test::core::ProfileRegistry;
using smsc::test::core::RouteChecker;
using smsc::test::core::PduDataObject;
using smsc::test::core::RespPduFlag;
using smsc::test::sms::ltAddress;
using smsc::test::util::CheckList;

struct SmppResponseSender
{
	virtual pair<uint32_t, time_t> sendSmsResp(SmppHeader* pdu) = NULL;
};

struct PduHandler
{
	virtual void processPdu(SmppHeader* pdu, time_t recvTime) = NULL;
	RespPduFlag isAccepted(uint32_t status);
};

struct AckText : public PduDataObject
{
	string text;
	uint8_t dataCoding;
	bool valid;
	AckText(const string& _text, uint8_t _dataCoding, bool _valid)
		: text(_text), dataCoding(_dataCoding), valid(_valid) {}
};

struct SenderData : public PduDataObject
{
	const SmeInfo* smeInfo;
	const Address srcAddr;
	const Profile profile;
	bool validProfile;
	SenderData(const SmeInfo* _smeInfo, const Address& _srcAddr,
		const Profile& _profile, bool _validProfile)
	: smeInfo(_smeInfo), srcAddr(_srcAddr), profile(_profile),
		validProfile(_validProfile) { __require__(smeInfo); }
};

struct RecipientData : public PduDataObject
{
	//const SmeInfo* smeInfo;
	const Address destAddr;
	const Profile profile;
	bool validProfile;
	RecipientData(const Address& _destAddr, const Profile& _profile, bool _validProfile)
		: destAddr(_destAddr), profile(_profile), validProfile(_validProfile) {}
};

struct SmsMsg : public PduDataObject
{
	bool udhi;
	const char* msg;
	int len;
	uint8_t dataCoding;
	bool valid;
	int offset;
	SmsMsg(bool _udhi, const char* _msg, int _len, uint8_t _dataCoding, bool _valid)
	: udhi(_udhi), msg(_msg), len(_len), dataCoding(_dataCoding), valid(_valid),
		offset(0) {}
	virtual ~SmsMsg() { if (msg) delete msg; }
};

class SmppBaseTestCases;
class SmppTransmitterTestCases;
class SmppReceiverTestCases;
class SmppPduChecker;

typedef map<const Address, PduHandler*, ltAddress> PduHandlerMap;

struct SmppFixture
{
	const SmeType smeType;
	SmppSession* session;
	const SmeInfo smeInfo;
	const Address smeAddr;
	SmppResponseSender* respSender;
	PduHandlerMap pduHandler;
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
	SmppPduSender* pduSender;

	SmppFixture(SmeType smeType, const SmeInfo& smeInfo, const Address& smeAddr,
		SmppResponseSender* respSender, const SmeRegistry* smeReg,
		const AliasRegistry* aliasReg, const RouteRegistry* routeReg,
		ProfileRegistry* profileReg, CheckList* chkList);
	~SmppFixture();
};

}
}
}

#endif /* TEST_SME_SMPP_FIXTURE */

