#include "RouteChecker.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

using std::vector;
using smsc::sms::Address;
using smsc::sms::AddressValue;
using smsc::smeman::SmeInfo;
using smsc::test::sms::operator!=;
using smsc::test::sms::str;
using smsc::test::smpp::SmppUtil;
using smsc::test::conf::TestConfig;

RouteChecker::RouteChecker(const string& id, const Address& addr,
	const SmeRegistry* _smeReg, const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg)
	: systemId(id), smeAddr(addr), smeReg(_smeReg), aliasReg(_aliasReg),
	routeReg(_routeReg)
{
	__require__(smeReg);
	__require__(aliasReg);
	__require__(routeReg);
	const SmeInfo* smeInfo = smeReg->getSme(systemId);
	__require__(smeInfo);
	wantAlias = smeInfo->wantAlias;
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
	//правильность destAddr
	const RouteHolder* routeHolder = NULL;
	const Address destAddr = aliasReg->findAddressByAlias(destAlias1);
	if (destAddr != destAddr2)
	{
		res.push_back(1);
	}
	else
	{
		//правильность маршрута
		routeHolder = routeReg->lookup(origAddr1, destAddr2);
		if (!routeHolder)
		{
			res.push_back(2);
		}
		else if (systemId != routeHolder->route.smeSystemId)
		{
			res.push_back(3);
		}
	}
	//правильность origAddr
	if (routeHolder)
	{
		const SmeInfo* smeInfo = smeReg->getSme(routeHolder->route.smeSystemId);
		__require__(smeInfo);
		if (smeInfo->wantAlias)
		{
			const Address origAlias = aliasReg->findAliasByAddress(origAddr1);
			if (origAlias != origAlias2)
			{
				res.push_back(4);
			}
		}
		else if (origAddr1 != origAlias2)
		{
			res.push_back(5);
		}
	}
	return res;
}

vector<int> RouteChecker::checkRouteForNotificationSms(PduSubmitSm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//правильность destAddr для pdu2
	if (destAddr2 != origAddr1)
	{
		res.push_back(1);
	}
	//правильность маршрута
	const RouteHolder* routeHolder = NULL;
	if (wantAlias)
	{
		const Address origAddr2 = aliasReg->findAddressByAlias(origAlias2);
		routeHolder = routeReg->lookup(origAddr2, destAddr2);
	}
	else
	{
		routeHolder = routeReg->lookup(origAlias2, destAddr2);
	}
	if (!routeHolder)
	{
		res.push_back(3);
	}
	else if (systemId != routeHolder->route.smeSystemId)
	{
		res.push_back(4);
	}
	return res;
}

vector<int> RouteChecker::checkRouteForAcknowledgementSms(PduSubmitSm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, destAlias1, origAlias2, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAlias1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//правильность destAddr для pdu2
	if (destAddr2 != origAddr1)
	{
		res.push_back(1);
	}
	//правильность origAddr для pdu2
	const RouteHolder* routeHolder = NULL;
	if (!wantAlias)
	{
		const Address destAddr = aliasReg->findAddressByAlias(destAlias1);
		if (origAlias2 != destAddr)
		{
			res.push_back(2);
		}
		else
		{
			routeHolder = routeReg->lookup(origAlias2, destAddr2);
		}
	}
	else if (origAlias2 != destAlias1)
	{
		res.push_back(3);
	}
	else
	{
		const Address origAddr2 = aliasReg->findAddressByAlias(origAlias2);
		routeHolder = routeReg->lookup(origAddr2, destAddr2);
	}
	//правильность маршрута
	if (!routeHolder)
	{
		res.push_back(4);
	}
	else if (systemId != routeHolder->route.smeSystemId)
	{
		res.push_back(5);
	}
	return res;
}

bool RouteChecker::isDestReachable(PduAddress& src, PduAddress& dest,
	bool checkSme) const
{
	//dest является алиасом
	Address srcAddr, destAlias;
	SmppUtil::convert(src, &srcAddr);
	SmppUtil::convert(dest, &destAlias);
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	//проверка маршрута
	const RouteHolder* routeHolder = routeReg->lookup(srcAddr, destAddr);
	if (checkSme && routeHolder)
	{
		bool bound = smeReg->isSmeBound(routeHolder->route.smeSystemId);
		__trace2__("isDestReachable(): destAddr = %s, route = %s",
			str(destAddr).c_str(), str(*routeHolder).c_str());
		return bound;
	}
	else
	{
		__trace2__("isDestReachable(): destAddr = %s, route = %s",
			str(destAddr).c_str(), routeHolder ? str(*routeHolder).c_str() : "NULL");
		return routeHolder;
	}
}

const RouteInfo* RouteChecker::getRouteInfoForNormalSms(PduAddress& src,
	PduAddress& dest) const
{
	Address origAddr, destAlias;
	SmppUtil::convert(src, &origAddr);
	SmppUtil::convert(dest, &destAlias);
	//правильность destAddr
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	//правильность маршрута
	const RouteHolder* routeHolder = routeReg->lookup(origAddr, destAddr);
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
	//параметры маршрута
	__cfg_addr__(smscAddr);
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

