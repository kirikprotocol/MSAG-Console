#include "RouteChecker.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

using std::vector;
using smsc::sms::Address;
using smsc::test::sms::SmsUtil;
using smsc::test::smpp::SmppUtil;

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

vector<int> RouteChecker::checkRouteForNormalSms(PduDeliverySm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAddr1, origAddr2, destAddr1, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAddr2);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAddr1);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//Проверить правильность destination адреса для pdu2
	if (!SmsUtil::compareAddresses(smeAddr, destAddr2))
	{
		res.push_back(1);
	}
	//destAddr1 является алиасом
	const AliasHolder* destHolder = aliasReg->findAddressByAlias(destAddr1);
	Address tmp;
	bool destFound = (destHolder && destHolder->aliasToAddress(destAddr1, tmp) &&
		SmsUtil::compareAddresses(tmp, destAddr2));
	if (!destFound)
	{
		res.push_back(2);
	}
	else
	{
		const RouteHolder* routeHolder = routeReg->lookup(origAddr1, destAddr2);
		bool smeFound = (routeHolder && systemId == routeHolder->route.smeSystemId);
		if (!smeFound)
		{
			res.push_back(3);
		}
	}
	//Проверить правильность source адреса для pdu2
	//origAddr2 является алиасом
	const AliasHolder* origHolder = aliasReg->findAliasByAddress(origAddr1);
	bool origFound = (origHolder && origHolder->addressToAlias(origAddr1, tmp) &&
		SmsUtil::compareAddresses(tmp, origAddr2));
	if (!origFound)
	{
		res.push_back(4);
	}
	return res;
}

vector<int> RouteChecker::checkRouteForNotification(PduDeliverySm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address destAddr2;
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	PduAddress& origAddr2 = pdu2.get_message().get_source();
	//Проверить правильность destination адреса для pdu2
	if (!SmsUtil::compareAddresses(smeAddr, destAddr2))
	{
		res.push_back(1);
	}
	//Проверить правильность source адреса для pdu2
	//origAddr2 должен быть пустым (или равным адресу сервис центра)
	if (origAddr2.get_typeOfNumber() != 0 ||
		origAddr2.get_numberingPlan() != 0 ||
		strcmp(origAddr2.get_value(), "") != 0)
	{
		res.push_back(2);
	}
	return res;
}

bool RouteChecker::isDestReachable(PduAddress& dest, bool checkSme) const
{
	//dest является алиасом
	Address destAlias;
	SmppUtil::convert(dest, &destAlias);
	const AliasHolder* destHolder = aliasReg->findAddressByAlias(destAlias);
	Address destAddr;
	if (destHolder && destHolder->aliasToAddress(destAlias, destAddr))
	{
		const RouteHolder* routeHolder = routeReg->lookup(smeAddr, destAddr);
		if (checkSme && routeHolder)
		{
			return smeReg->isSmeAvailable(routeHolder->route.smeSystemId);
		}
		else
		{
			return routeHolder;
		}
	}
	return false;
}

}
}
}

