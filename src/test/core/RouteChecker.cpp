#include "RouteChecker.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/TestConfig.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

using std::vector;
using smsc::sms::Address;
using smsc::sms::AddressValue;
using smsc::test::sms::SmsUtil;
using smsc::test::sms::operator!=;
using smsc::test::smpp::SmppUtil;
using namespace smsc::test; //config constants

RouteChecker::RouteChecker(const string& id, const Address& addr,
	const SmeRegistry* _smeReg, const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg)
	: systemId(id), smeAddr(addr), smeReg(_smeReg), aliasReg(_aliasReg),
	routeReg(_routeReg)
{
	__require__(smeReg);
	__require__(aliasReg);
	__require__(routeReg);
}

vector<int> RouteChecker::checkRouteForNormalSms(PduSubmitSm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, destAlias1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAlias1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//������������ destAddr
	bool destOk = false;
	auto_ptr<const Address> destAddr = aliasReg->findAddressByAlias(destAlias1);
	if (!destAddr.get())
	{
		res.push_back(1);
	}
	else if (*destAddr != destAddr2)
	{
		res.push_back(2);
	}
	else
	{
		destOk = true;
	}
	//������������ origAddr
	bool origOk = false;
	auto_ptr<const Address> origAlias = aliasReg->findAliasByAddress(origAddr1);
	if (!origAlias.get())
	{
		res.push_back(3);
	}
	else if (*origAlias != origAlias2)
	{
		res.push_back(4);
	}
	else
	{
		origOk = true;
	}
	//������������ ��������
	if (destOk && origOk)
	{
		const RouteHolder* routeHolder = routeReg->lookup(origAddr1, destAddr2);
		if (!routeHolder)
		{
			res.push_back(5);
		}
		else if (systemId != routeHolder->route.smeSystemId)
		{
			res.push_back(6);
		}
	}
	return res;
}

vector<int> RouteChecker::checkRouteForNotification(PduSubmitSm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, origAddr2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAddr2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//������������ destAddr ��� pdu2
	bool destOk = true;
	if (destAddr2 != origAddr1)
	{
		res.push_back(1);
		destOk = false;
	}
	//������������ origAddr ��� pdu2
	bool origOk = true;
	if (origAddr2 != smscAddr)
	{
		res.push_back(2);
		origOk = false;
	}
	//������������ ��������
	if (destOk && origOk)
	{
		const RouteHolder* routeHolder = routeReg->lookup(smscAddr, destAddr2);
		if (!routeHolder)
		{
			res.push_back(3);
		}
		else if (systemId != routeHolder->route.smeSystemId)
		{
			res.push_back(4);
		}
	}
	return res;
}

bool RouteChecker::isDestReachable(PduAddress& dest, bool checkSme) const
{
	//dest �������� �������
	Address destAlias;
	SmppUtil::convert(dest, &destAlias);
	auto_ptr<const Address> destAddr = aliasReg->findAddressByAlias(destAlias);
	if (!destAddr.get())
	{
		return false;
	}
	//�������� ��������
	const RouteHolder* routeHolder = routeReg->lookup(smeAddr, *destAddr);
	if (checkSme && routeHolder)
	{
		return smeReg->isSmeBound(routeHolder->route.smeSystemId);
	}
	else
	{
		return routeHolder;
	}
}

const RouteInfo* RouteChecker::getRouteInfoForNormalSms(PduAddress& src,
	PduAddress& dest) const
{
	Address origAddr, destAlias;
	SmppUtil::convert(src, &origAddr);
	SmppUtil::convert(dest, &destAlias);
	//������������ destAddr
	auto_ptr<const Address> destAddr = aliasReg->findAddressByAlias(destAlias);
	if (!destAddr.get())
	{
		return NULL;
	}
	//������������ ��������
	const RouteHolder* routeHolder = routeReg->lookup(origAddr, *destAddr);
	if (!routeHolder)
	{
		return NULL;
	}
	return &routeHolder->route;
}

const RouteInfo* RouteChecker::getRouteInfoForNotification(PduAddress& dest) const
{
	Address destAddr;
	SmppUtil::convert(dest, &destAddr);
	//��������� ��������
	const RouteHolder* routeHolder = routeReg->lookup(smscAddr, destAddr);
	if (!routeHolder)
	{
		return NULL;
	}
	return &routeHolder->route;
}

}
}
}

