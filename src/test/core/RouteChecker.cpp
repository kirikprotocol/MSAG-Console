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

SmeType RouteChecker::isDestReachable(PduAddress& src, PduAddress& dest) const
{
	//dest �������� �������
	Address srcAddr, destAlias;
	SmppUtil::convert(src, &srcAddr);
	SmppUtil::convert(dest, &destAlias);
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	//�������� ��������
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

const RouteInfo* RouteChecker::getRouteInfoForNormalSms(PduAddress& src,
	PduAddress& dest) const
{
	Address origAddr, destAlias;
	SmppUtil::convert(src, &origAddr);
	SmppUtil::convert(dest, &destAlias);
	//������������ destAddr
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	//������������ ��������
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
	//��������� ��������
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

