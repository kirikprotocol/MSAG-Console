#include "SmeUtil.hpp"
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

RouteChecker::RouteChecker(const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg)
	: aliasReg(_aliasReg), routeReg(_routeReg)
{
	__require__(aliasReg);
	__require__(routeReg);
}

vector<int> RouteChecker::checkRouteForNormalSms(PduDeliverySm& pdu1,
	PduDeliverySm& pdu2, const string& systemId, const Address& smeAddr)
{
	vector<int> res;
	Address origAddr1, origAddr2, destAddr1, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAddr2);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAddr1);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//ѕроверить правильность destination адреса дл€ pdu2
	if (!SmsUtil::compareAddresses(smeAddr, destAddr2))
	{
		res.push_back(1);
	}
	//destAddr1 €вл€етс€ алиасом
	const AliasRegistry::AliasList destList =
		aliasReg->findAddressByAlias(destAddr1);
	bool destFound = false;
	for (int i = 0; i < destList.size(); i++)
	{
		Address tmp;
		if (destList[i]->aliasToAddress(destAddr1, tmp) &&
			SmsUtil::compareAddresses(tmp, destAddr2))
		{
			destFound = true;
			break;
		}
	}
	if (!destFound)
	{
		res.push_back(2);
	}
	else
	{
		const RouteRegistry::RouteList routeList =
			routeReg->lookup(origAddr1, destAddr2);
		bool smeFound = false;
		for (int i = 0; i < routeList.size(); i++)
		{
			if (systemId == routeList[i]->route->smeSystemId)
			{
				smeFound = true;
				break;
			}
		}
		if (!smeFound)
		{
			res.push_back(3);
		}
	}
	//ѕроверить правильность source адреса дл€ pdu2
	//origAddr2 €вл€етс€ алиасом
	const AliasRegistry::AliasList origList =
		aliasReg->findAliasByAddress(origAddr1);
	bool origFound = false;
	for (int i = 0; i < origList.size(); i++)
	{
		Address tmp;
		if (origList[i]->addressToAlias(origAddr1, tmp) &&
			SmsUtil::compareAddresses(tmp, origAddr2))
		{
			origFound = true;
			break;
		}
	}
	if (!origFound)
	{
		res.push_back(4);
	}
	return res;
}

vector<int> RouteChecker::checkRouteForNotification(PduDeliverySm& pdu1,
	PduDeliverySm& pdu2, const string& systemId, const Address& smeAddr,
	const Address& smscAddr)
{
	vector<int> res;
	Address origAddr1, origAddr2, destAddr1, destAddr2;
	SmppUtil::convert(pdu1.get_message().get_source(), &origAddr1);
	SmppUtil::convert(pdu2.get_message().get_source(), &origAddr2);
	SmppUtil::convert(pdu1.get_message().get_dest(), &destAddr1);
	SmppUtil::convert(pdu2.get_message().get_dest(), &destAddr2);
	//ѕроверить правильность destination адреса дл€ pdu2
	if (!SmsUtil::compareAddresses(smeAddr, destAddr2))
	{
		res.push_back(1);
	}
	const RouteRegistry::RouteList routeList =
		routeReg->lookup(smscAddr, destAddr2);
	bool smeFound = false;
	for (int i = 0; i < routeList.size(); i++)
	{
		if (systemId == routeList[i]->route->smeSystemId)
		{
			smeFound = true;
			break;
		}
	}
	if (!smeFound)
	{
		res.push_back(2);
	}
	//ѕроверить правильность source адреса дл€ pdu2
	//origAddr2 €вл€етс€ адресом сервис центра
	if (!SmsUtil::compareAddresses(smscAddr, origAddr2))
	{
		res.push_back(3);
	}
	return res;
}

}
}
}

