#include "RouteChecker.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace core {

using std::vector;
using smsc::sms::Address;
using smsc::sms::AddressValue;
using smsc::test::sms::SmsUtil;
using smsc::test::smpp::SmppUtil;

RouteChecker::RouteChecker(const string& id, const Address& addr,
	const SmeRegistry* _smeReg, const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg)
	: systemId(id), smeAlias(addr), smeReg(_smeReg), aliasReg(_aliasReg),
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
	Address origAlias1, origAlias2, destAlias1, destAlias2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAlias1);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAlias1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAlias2);
	//������������ destAddr
	if (!SmsUtil::compareAddresses(destAlias1, destAlias2))
	{
		res.push_back(1);
	}
	const AliasHolder* destHolder = aliasReg->findAddressByAlias(destAlias1);
	bool destOk = false;
	Address destAddr;
	if (!destHolder)
	{
		res.push_back(2);
	}
	else if (!destHolder->aliasToAddress(destAlias1, destAddr))
	{
		res.push_back(3);
	}
	else
	{
		destOk = true;
	}
	//������������ origAddr
	if (!SmsUtil::compareAddresses(origAlias1, origAlias2))
	{
		res.push_back(4);
	}
	const AliasHolder* origHolder = aliasReg->findAddressByAlias(origAlias1);
	Address origAddr;
	bool origOk = false;
	if (!origHolder)
	{
		res.push_back(5);
	}
	else if (!origHolder->aliasToAddress(origAlias1, origAddr))
	{
		res.push_back(6);
	}
	else
	{
		origOk = true;
	}
	//������������ ��������
	if (destOk && origOk)
	{
		const RouteHolder* routeHolder = routeReg->lookup(origAddr, destAddr);
		if (!routeHolder)
		{
			res.push_back(7);
		}
		else if (systemId != routeHolder->route.smeSystemId)
		{
			res.push_back(8);
		}
	}
	return res;
}

vector<int> RouteChecker::checkRouteForNotification(PduDeliverySm& pdu1,
	PduDeliverySm& pdu2) const
{
	vector<int> res;
	Address origAlias1, origAlias2, destAlias2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAlias1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAlias2);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAlias2);
	//������������ destAddr ��� pdu2
	/*
	if (!SmsUtil::compareAddresses(smeAlias, origAlias1))
	{
		res.push_back(1);
	}
	*/
	if (!SmsUtil::compareAddresses(origAlias1, destAlias2))
	{
		res.push_back(1);
	}
	//������������ origAddr ��� pdu2
	//origAddr2 ������ ���� ������ (��� ������ ������ ������ ������)
	AddressValue addrVal;
	origAlias2.getValue(addrVal);
	if (origAlias2.getTypeOfNumber() != 0 ||
		origAlias2.getNumberingPlan() != 0 ||
		strcmp(addrVal, "") != 0)
	{
		res.push_back(2);
	}
	return res;
}

bool RouteChecker::isDestReachable(PduAddress& dest, bool checkSme) const
{
	//�������� smeAlias
	const AliasHolder* origHolder = aliasReg->findAddressByAlias(smeAlias);
	Address origAddr;
	if (!origHolder || !origHolder->aliasToAddress(smeAlias, origAddr))
	{
		return false;
	}
	//dest �������� �������
	Address destAlias;
	SmppUtil::convert(dest, &destAlias);
	const AliasHolder* destHolder = aliasReg->findAddressByAlias(destAlias);
	Address destAddr;
	if (!destHolder || !destHolder->aliasToAddress(destAlias, destAddr))
	{
		return false;
	}
	//�������� ��������
	const RouteHolder* routeHolder = routeReg->lookup(origAddr, destAddr);
	if (checkSme && routeHolder)
	{
		return smeReg->isSmeBound(routeHolder->route.smeSystemId);
	}
	else
	{
		return routeHolder;
	}
}

}
}
}

