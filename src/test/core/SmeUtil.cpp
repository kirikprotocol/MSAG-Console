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
	PduDeliverySm& pdu2)
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
	//Проверить правильность source адреса для pdu2
	//origAddr2 является алиасом
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
	PduDeliverySm& pdu2)
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

bool RouteChecker::checkExistsReachableRoute(PduAddress& dest, bool checkSme)
{
	//dest является алиасом
	Address destAlias;
	SmppUtil::convert(dest, &destAlias);
	const AliasRegistry::AliasList destList =
		aliasReg->findAddressByAlias(destAlias);
	for (int i = 0; i < destList.size(); i++)
	{
		Address destAddr;
		if (!destList[i]->aliasToAddress(destAlias, destAddr))
		{
			continue;
		}
		const RouteRegistry::RouteList routeList =
			routeReg->lookup(smeAddr, destAddr);
		if (checkSme)
		{
			for (int i = 0; i < routeList.size(); i++)
			{
				if (smeReg->isSmeRegistered(routeList[i]->route->smeSystemId))
				{
					return true;
				}
			}
		}
		else if (routeList.size())
		{
			return true;
		}
	}
	return false;
}

bool RouteChecker::checkExistsUnreachableRoute(PduAddress& dest, bool checkSme)
{
	//dest является алиасом
	Address destAlias;
	SmppUtil::convert(dest, &destAlias);
	const AliasRegistry::AliasList destList =
		aliasReg->findAddressByAlias(destAlias);
	if (!destList.size())
	{
		return true;
	}
	for (int i = 0; i < destList.size(); i++)
	{
		Address destAddr;
		if (!destList[i]->aliasToAddress(destAlias, destAddr))
		{
			return true;
		}
		const RouteRegistry::RouteList routeList =
			routeReg->lookup(smeAddr, destAddr);
		if (!routeList.size())
		{
			return true;
		}
		if (checkSme)
		{
			for (int i = 0; i < routeList.size(); i++)
			{
				if (!smeReg->isSmeRegistered(routeList[i]->route->smeSystemId))
				{
					return true;
				}
			}
		}
	}
	return false;
}

}
}
}

