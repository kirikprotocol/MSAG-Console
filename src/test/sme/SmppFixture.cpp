#include "SmppFixture.hpp"
#include "test/smpp/SmppUtil.hpp"

namespace smsc {
namespace test {
namespace sme {

using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::core;
using namespace smsc::test::smpp;

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
		case DELIVERY_STATUS_NO_RESPONSE: //ошибка отправки deliver_sm_resp
			return RESP_PDU_MISSING;
		case ESME_RX_P_APPN:
			__unreachable__("Not supported");
		//все остальные коды ошибок
		default:
			return RESP_PDU_ERROR;
	}
}

SmppFixture::SmppFixture(const SmeInfo& _smeInfo, const Address& _smeAddr,
	SmppResponseSender* _respSender, const SmeRegistry* _smeReg,
	const AliasRegistry* _aliasReg, const RouteRegistry* _routeReg,
	ProfileRegistry* _profileReg, CheckList* _chkList)
: session(NULL), smeInfo(_smeInfo), smeAddr(_smeAddr),
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

