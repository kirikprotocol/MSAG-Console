#include "SmppFixture.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "system/status.h"

namespace smsc {
namespace test {
namespace sme {

using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::system;
using namespace smsc::test::core;
using namespace smsc::test::smpp;
using namespace std;

RespPduFlag PduHandler::isAccepted(uint32_t status)
{
	switch (status)
	{
		//повторная доставка
		case ESME_ROK:
			return RESP_PDU_OK;
		case ESME_RX_T_APPN:
		case ESME_RMSGQFUL:
			return RESP_PDU_RESCHED;
		case Status::DELIVERYTIMEDOUT: //ошибка отправки deliver_sm_resp
			return RESP_PDU_MISSING;
		case ESME_RX_P_APPN:
			__unreachable__("Not supported");
		//все остальные коды ошибок
		default:
			return RESP_PDU_ERROR;
	}
}

SmsMsg::SmsMsg(bool _udhi, const char* _msg, int _len, uint8_t _dataCoding,
	bool _valid) : udhi(_udhi), msg(_msg), len(_len), dataCoding(_dataCoding),
	valid(_valid), offset(0)
{
	ostringstream s;
	s << hex;
	int udhLen = udhi ? 1 + *(unsigned char*) msg : 0;
	for (int i = 0; i < len && i < MAX_OSTR_PRINT_SIZE; i++)
	{
		if (udhi && i == udhLen) { s << endl; }
		unsigned int ch = (unsigned char) *(msg + i);
		s << (ch < 0x10 ? "0" : "") << ch << " ";
	}
	__trace2__("sms msg created: this = %p, udhi = %s, dataCoding = %d, valid = %s, msg: len = %d\n%s",
		this, udhi ? "true" : "false", (int) dataCoding, valid ? "true" : "false", len, s.str().c_str());
}

SmppFixture::SmppFixture(SmeType _smeType, const SmeInfo& _smeInfo,
	const Address& _smeAddr, SmppResponseSender* _respSender,
	const SmeRegistry* _smeReg, const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg, ProfileRegistry* _profileReg,
	CheckList* _chkList)
: smeType(_smeType), session(NULL), smeInfo(_smeInfo), smeAddr(_smeAddr),
	respSender(_respSender), smeReg(_smeReg), aliasReg(_aliasReg),
	routeReg(_routeReg), pduReg(smeReg->getPduRegistry(smeAddr)),
	profileReg(_profileReg), chkList(_chkList), routeChecker(NULL),
	pduChecker(NULL), base(NULL), transmitter(NULL), receiver(NULL),
	pduSender(NULL)
{
	//__require__(respSender);
	__require__(smeReg);
	//__require__(aliasReg);
	//__require__(routeReg);
	//__require__(profileReg);
	//__require__(chkList);
	if (pduReg && aliasReg && routeReg)
	{
		routeChecker = new RouteChecker(smeReg, aliasReg, routeReg);
	}
}

SmppFixture::~SmppFixture()
{
	if (routeChecker) { delete routeChecker; routeChecker = NULL; }
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
		session = NULL;
	}
}

}
}
}

