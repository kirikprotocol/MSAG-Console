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

RespPduFlag PduHandler::isAccepted(uint32_t status)
{
	switch (status)
	{
		//��������� ��������
		case ESME_ROK:
			return RESP_PDU_OK;
		case ESME_RX_T_APPN:
		case ESME_RMSGQFUL:
			return RESP_PDU_RESCHED;
		case Status::DELIVERYTIMEDOUT: //������ �������� deliver_sm_resp
			return RESP_PDU_MISSING;
		case ESME_RX_P_APPN:
			__unreachable__("Not supported");
		//��� ��������� ���� ������
		default:
			return RESP_PDU_ERROR;
	}
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

