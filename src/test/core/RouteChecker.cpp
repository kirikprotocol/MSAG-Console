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
using smsc::test::sms::operator!=;
using smsc::test::sms::str;
using smsc::test::smpp::SmppUtil;
using smsc::test::conf::TestConfig;

RouteChecker::RouteChecker(const SmeRegistry* _smeReg,
	const AliasRegistry* _aliasReg, const RouteRegistry* _routeReg)
	: smeReg(_smeReg), aliasReg(_aliasReg), routeReg(_routeReg)
{
	__require__(smeReg);
	__require__(aliasReg);
	__require__(routeReg);
}

SmeType RouteChecker::isDestReachable(const Address& srcAddr,
	const Address& destAlias) const
{
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	//проверка маршрута
	const RouteHolder* routeHolder = routeReg->lookup(srcAddr, destAddr);
	SmeType smeType;
	if (routeHolder)
	{
		smeType = smeReg->getSmeBindType(routeHolder->route.smeSystemId);
		__require__(smeType != SME_NO_ROUTE);
		__trace2__("isDestReachable(): srcAddr = %s, destAlias = %s, destAddr = %s, route = %s, smeType = %d",
			str(srcAddr).c_str(), str(destAlias).c_str(), str(destAddr).c_str(), str(*routeHolder).c_str(), smeType);
	}
	else
	{
		smeType = SME_NO_ROUTE;
		__trace2__("isDestReachable(): srcAddr = %s, destAlias = %s, destAddr = %s, no route",
			str(srcAddr).c_str(), str(destAlias).c_str(), str(destAddr).c_str());
	}
	return smeType;
}

SmeType RouteChecker::isDestReachable(PduAddress& srcAddr,
	PduAddress& destAlias) const
{
	//dest является алиасом
	Address src, dest;
	SmppUtil::convert(srcAddr, &src);
	SmppUtil::convert(destAlias, &dest);
	return isDestReachable(src, dest);
}

const RouteInfo* RouteChecker::getRouteInfoForNormalSms(const Address& srcAddr,
	const Address& destAlias) const
{
	//правильность destAddr
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	//правильность маршрута
	const RouteHolder* routeHolder = routeReg->lookup(srcAddr, destAddr);
	if (!routeHolder)
	{
		return NULL;
	}
	return &routeHolder->route;
}

const RouteInfo* RouteChecker::getRouteInfoForNormalSms(PduAddress& srcAddr,
	PduAddress& destAlias) const
{
	Address src, dest;
	SmppUtil::convert(srcAddr, &src);
	SmppUtil::convert(destAlias, &dest);
	return getRouteInfoForNormalSms(src, dest);
}

const RouteInfo* RouteChecker::getRouteInfoForNotification(
	const Address& destAddr) const
{
	//параметры маршрута
	__cfg_addr__(smscAddr);
	const RouteHolder* routeHolder = routeReg->lookup(smscAddr, destAddr);
	if (!routeHolder)
	{
		return NULL;
	}
	return &routeHolder->route;
}

const RouteInfo* RouteChecker::getRouteInfoForNotification(PduAddress& destAddr) const
{
	Address dest;
	SmppUtil::convert(destAddr, &dest);
	return getRouteInfoForNotification(dest);
}

}
}
}

