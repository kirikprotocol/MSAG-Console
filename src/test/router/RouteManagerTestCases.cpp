#include "RouteManagerTestCases.hpp"
#include "test/core/CoreTestManager.hpp"
#include "test/smeman/SmeManagerUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/core/RouteUtil.hpp"
#include "test/util/Util.hpp"
#include <sys/types.h>
#include <sstream>
#include <util/debug.h>

namespace smsc {
namespace test {
namespace router {

using namespace std;
using namespace smsc::sms; //constants, AddressValue
using namespace smsc::router; //constants
using namespace smsc::test::sms; //SmsUtil, print Address
using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, TestSmeProxy
using namespace smsc::test::core; //route utils, CoreTestManager
using smsc::util::Logger;
using smsc::sms::AddressValue;
using smsc::test::core::RouteUtil;

void RouteManagerTestCases::debugRoute(const TestRouteData* routeData)
{
	getLog().debugStream() << "addRoute(): " << *routeData;
}

RouteManagerTestCases::RouteManagerTestCases()
	: routeMan(CoreTestManager::getRouteManager()) {}

Category& RouteManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("RouteManagerTestCases");
	return log;
}

float RouteManagerTestCases::setupRandomAddressMatch(Address& addr, int num)
{
	AddressValue addrVal;
	uint8_t addrLen = addr.getValue(addrVal);
	int len = rand1(addrLen);
	int len2 = rand1(len);
	int len3 = rand1(MAX_ADDRESS_VALUE_LENGTH - addrLen + len - len2);
	switch(num)
	{
		case 1: //����� � ����� ��� ����� '?' � �����
			memset(addrVal + addrLen - len, '?', len);
			addr.setValue(addrLen, addrVal);
			return (100 - len);
		case 2: //����� ������� �� '?'
			memset(addrVal, '?', addrLen);
			addr.setValue(addrLen, addrVal);
			return (100 - addrLen);
		case 3: //����� ������� ��� �������� �� '*' � �����
			memset(addrVal + addrLen - len, '*', len2);
			addr.setValue(addrLen - len + len2, addrVal);
			return (100 - len - 0.5); //'*' ����� ����������� '?'
		case 4: //����� �� '*'
			memset(addrVal, '*', len);
			addr.setValue(len, addrVal);
			return (100 - addrLen - 0.5);
		case 5: //����� � ������ '*'
			if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
			{
				addrVal[addrLen] = '*';
				addr.setValue(addrLen + 1, addrVal);
				return 99.5;
			}
			return 100;
		case 6: //����� � '?*'
			if (addrLen - len + len2 < MAX_ADDRESS_VALUE_LENGTH)
			{
				memset(addrVal + addrLen - len, '?', len2);
				memset(addrVal + addrLen - len + len2, '*', len3);
				addr.setValue(addrLen - len + len2 + len3, addrVal);
				return (100 - len - 0.25);
			}
			return 100;
		case 7: //����� � '*?'
			if (addrLen - len + len2 < MAX_ADDRESS_VALUE_LENGTH)
			{
				memset(addrVal + addrLen - len, '*', len3);
				memset(addrVal + addrLen - len + len3, '?', len2);
				addr.setValue(addrLen - len + len2 + len3, addrVal);
				return (100 - len - 0.25);
			}
			return 100;
		default:
			throw "";
	}
}

void RouteManagerTestCases::setupRandomAddressNotMatch(Address& addr, int num)
{
	AddressValue addrVal;
	uint8_t addrLen = addr.getValue(addrVal);
	int len = rand1(addrLen);
	switch(num)
	{
		case 1: //����� � ������ '?'
			if (addrLen < MAX_ADDRESS_VALUE_LENGTH)
			{
				addrVal[addrLen] = '?';
				addr.setValue(addrLen + 1, addrVal);
				break;
			}
			//break;
		case 2: //����� � '*?' � ������� '?'
			if (addrLen + 1 < MAX_ADDRESS_VALUE_LENGTH)
			{
				memset(addrVal + addrLen - len, '*', 1);
				memset(addrVal + addrLen - len + 1, '?', len + 1);
				addr.setValue(addrLen + 2, addrVal);
				break;
			}
			//break;
		case 3: //������������ �����
			addrVal[len - 1] = '+';
			addr.setValue(addrLen, addrVal);
			break;
		case 4: //����� � ������������� typeOfNumber
			addr.setTypeOfNumber(addr.getTypeOfNumber() + 1);
			break;
		case 5: //����� � ������������� numberingPlan
			addr.setNumberingPlan(addr.getNumberingPlan() + 1);
			break;
		default:
			throw "";
	}
}

void RouteManagerTestCases::setupRandomPriority(RoutePriority* priority, int num)
{
	switch(num)
	{
		case 1: //������������ ���������
			*priority = RoutePriorityMax;
			break;
		case 2: //����������� ���������
			*priority = RoutePriorityMin;
			break;
		default:
			throw "";
	}
}

TCResult* RouteManagerTestCases::addCorrectRouteMatch(
	const SmeSystemId& smeSystemId, TestRouteData* data, int num)
{
	int num1 = 7; int num2 = 7; int num3 = 2;
	TCSelector s(num, num1 * num2 * num3);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE_MATCH, s.getChoice());
	for (; s.check(); s++)
	{
		RouteInfo* route = new RouteInfo();
		data->match = true;
		data->route = route;
		try
		{
			RouteUtil::setupRandomCorrectRouteInfo(smeSystemId, route);
			//origAddr
			route->source = data->origAddr;
			data->origAddrMatch =
				setupRandomAddressMatch(route->source, s.value1(num1, num2));
			//destAddr
			route->dest = data->destAddr;
			data->destAddrMatch =
				setupRandomAddressMatch(route->dest, s.value2(num1, num2));
			//���������
			setupRandomPriority(&route->priority, s.value3(num1, num2));
			getLog().debugStream() << "addCorrectRouteMatch(" <<
				s.value1(num1, num2) << "," << s.value2(num1, num2) << "," <<
				s.value3(num1, num2) << "): " << *data;
			routeMan->addRoute(*route);
		}
		catch(...)
		{
			error();
			delete route;
			data->route = NULL;
			res->addFailure(1000);
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addCorrectRouteNotMatch(
	const SmeSystemId& smeSystemId, TestRouteData* data, int num)
{
	int numMatch = 7; int numNotMatch = 5; int numType = 2;
	TCSelector s(num, numMatch * numNotMatch * numType);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE_NOT_MATCH, s.getChoice());
	for (; s.check(); s++)
	{
		RouteInfo* route = new RouteInfo();
		data->match = false;
		data->route = route;
		try
		{
			RouteUtil::setupRandomCorrectRouteInfo(smeSystemId, route);
			route->source = data->origAddr;
			route->dest = data->destAddr;
			route->priority = RoutePriorityMax;
			switch (s.value3(numMatch, numNotMatch))
			{
				case 1: //���������� origAddr
					data->origAddrMatch = 0;
					setupRandomAddressNotMatch(route->source,
						s.value1(numNotMatch, numMatch));
					data->destAddrMatch = setupRandomAddressMatch(route->dest,
						s.value2(numNotMatch, numMatch));
					break;
				case 2: //���������� destAddr
					data->origAddrMatch = setupRandomAddressMatch(route->source,
						s.value1(numMatch, numNotMatch));
					data->destAddrMatch = 0;
					setupRandomAddressNotMatch(route->dest,
						s.value2(numMatch, numNotMatch));
					break;
				default:
					throw s;
			}
			getLog().debugStream() << "addCorrectRouteNotMatch(" <<
				s.value1(numMatch, numNotMatch) << "," <<
				s.value2(numMatch, numNotMatch) << "," <<
				s.value3(numMatch, numNotMatch) << "): " << *data;
			routeMan->addRoute(*route);
		}
		catch(...)
		{
			error();
			delete route;
			data->route = NULL;
			res->addFailure(1000);
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addCorrectRouteNotMatch2(
	const SmeSystemId& smeSystemId, TestRouteData* data, int num)
{
	TCSelector s(num, 8, 1000);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE_NOT_MATCH, s.getChoice());
	for (; s.check(); s++)
	{
		RouteInfo* route = new RouteInfo();
		data->match = false;
		data->origAddrMatch = 0;
		data->destAddrMatch = 0;
		data->route = route;
		try
		{
			RouteUtil::setupRandomCorrectRouteInfo(smeSystemId, route);
			SmsUtil::setupRandomCorrectAddress(&route->source);
			SmsUtil::setupRandomCorrectAddress(&route->dest);
			switch(s.value())
			{
				case 1001: //origAddr.typeOfNumber ��� ���������
					route->source.setTypeOfNumber(rand2(0x7, 0xff));
					break;
				case 1002: //origAddr.numberingPlan ��� ���������
					route->source.setNumberingPlan(rand2(0x13, 0xff));
					break;
				case 1003: //������ '?' � �������� origAddr
					{
						AddressValue addrVal;
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_VALUE_LENGTH);
						memcpy(addrVal, tmp.get(), MAX_ADDRESS_VALUE_LENGTH);
						addrVal[MAX_ADDRESS_VALUE_LENGTH / 2] = '?';
						route->source.setValue(MAX_ADDRESS_VALUE_LENGTH, addrVal);
					}
					break;
				case 1004: //������ '*' � �������� origAddr
					{
						AddressValue addrVal;
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_VALUE_LENGTH);
						memcpy(addrVal, tmp.get(), MAX_ADDRESS_VALUE_LENGTH);
						addrVal[MAX_ADDRESS_VALUE_LENGTH / 2] = '*';
						route->source.setValue(MAX_ADDRESS_VALUE_LENGTH, addrVal);
					}
					break;
				case 1005: //destAddr.typeOfNumber ��� ���������
					route->dest.setTypeOfNumber(rand2(0x7, 0xff));
					break;
				case 1006: //destAddr.numberingPlan ��� ���������
					route->dest.setNumberingPlan(rand2(0x13, 0xff));
					break;
				case 1007: //������ '?' � �������� destAddr
					{
						AddressValue addrVal;
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_VALUE_LENGTH);
						memcpy(addrVal, tmp.get(), MAX_ADDRESS_VALUE_LENGTH);
						addrVal[MAX_ADDRESS_VALUE_LENGTH / 2] = '?';
						route->dest.setValue(MAX_ADDRESS_VALUE_LENGTH, addrVal);
					}
					break;
				case 1008: //������ '*' � �������� destAddr
					{
						AddressValue addrVal;
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_VALUE_LENGTH);
						memcpy(addrVal, tmp.get(), MAX_ADDRESS_VALUE_LENGTH);
						addrVal[MAX_ADDRESS_VALUE_LENGTH / 2] = '*';
						route->dest.setValue(MAX_ADDRESS_VALUE_LENGTH, addrVal);
					}
					break;
				default:
					throw s;
			}
			getLog().debugStream() << "addCorrectRouteNotMatch2(" <<
				s.value() << "): " << *data;
			routeMan->addRoute(*route);
		}
		catch(...)
		{
			error();
			delete route;
			data->route = NULL;
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addIncorrectRoute(const SmeSystemId& smeSystemId,
	const RouteInfo& existingRoute, int num)
{
	TCSelector s(num, 2);
	TCResult* res = new TCResult(TC_ADD_INCORRECT_ROUTE, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			RouteInfo route;
			route.priority = rand2(RoutePriorityMin, RoutePriorityMax);
			route.billing = rand0(1);
			route.paid = rand0(1);
			route.archived = rand0(1);
			route.routeId = rand0(INT_MAX);
			route.smeSystemId = smeSystemId;
			SmsUtil::setupRandomCorrectAddress(&route.source);
			SmsUtil::setupRandomCorrectAddress(&route.dest);
			switch(s.value())
			{
				case 1: //����������� routeId
					//route.routeId = existingRoute.routeId;
					//break;
				case 2: //�������������� smeSystemId
					{
						auto_ptr<char> tmp = rand_char(rand1(MAX_SYSTEM_ID_LENGTH));
						route.smeSystemId = tmp.get();
					}
					break;
				default:
					throw s;
			}
			routeMan->addRoute(route);
			res->addFailure(101);
		}
		catch(...)
		{
			//Ok
		}
	}
	debug(res);
	return res;
}

void RouteManagerTestCases::printLookupResult(const Address& origAddr,
	const Address& destAddr, const vector<const SmeProxy*>& proxyList,
	const SmeProxy* proxy)
{
	stringstream ss;
	ss << "lookupRoute(): ids = {";
	/*
	char buf[20];
	for (int i = 0; i < proxyList.size(); i++)
	{
		if (i > 0)
		{
			ss  << ',';
		}
		ss << proxyList[i]->getUniqueId();
	}
	*/
	/*
	if (proxy)
	{
		//ss << "}, res = " << proxy->getUniqueId();
	}
	else
	{
		ss << "}, res = <>";
	}
	*/
	ss << ", origAddr = " << origAddr;
	ss << ", destAddr = " << destAddr;
	//getLog().debugStream() << ss.str();
}

TCResult* RouteManagerTestCases::lookupRoute(const RouteRegistry& routeReg,
	const Address& origAddr, const Address& destAddr)
{
	TCResult* res = new TCResult(TC_LOOKUP_ROUTE);
	try
	{
		SmeProxy* proxy;
		bool found = routeMan->lookup(origAddr, destAddr, proxy);
		const vector<const SmeProxy*> proxyList = routeReg.lookup(origAddr, destAddr);
		printLookupResult(origAddr, destAddr, proxyList, proxy);
		if (found && proxy)
		{
			uint32_t id = proxy->getUniqueId();
			found = false;
			for (int i = 0; i < proxyList.size(); i++)
			{
const SmeProxy* p = proxyList[i];
getLog().debug("proxyList[%d] = %lx", i, p);
uint32_t uid = p->getUniqueId();
getLog().debug("proxyList[%d]->getUniqueId() = %x", i,uid);
				if (proxyList[i]->getUniqueId() == id)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				res->addFailure(101);
			}
		}
		else if (proxyList.size())
		{
			res->addFailure(102);
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::iterateRoutes(const RouteRegistry& routeReg)
{
	TCResult* res = new TCResult(TC_ITERATE_ROUTES);
	try
	{
		RouteIterator* iter = routeMan->iterator();
		int foundRoutes = 0;
		int extraRoutes = 0;
		typedef map<int, int> MismatchMap;
		MismatchMap mismatch;
		while(iter->next())
		{
			//SmeProxy* proxy = iter->getSmeProxy();
			//int index = iter->getIndex();
			RouteInfo route = iter->getRouteInfo();
			const RouteInfo* correctRoute = routeReg.getRoute(route.routeId);
			if (!correctRoute)
			{
				extraRoutes++;
			}
			else
			{
				foundRoutes++;
				vector<int> tmp = RouteUtil::compareRoutes(*correctRoute, route);
				for (int i = 0; i < tmp.size(); i++)
				{
					mismatch[tmp[i]]++;
				}
			}
		}
		delete iter;
		//�������� ������ ������ ��������
		if (extraRoutes)
		{
			res->addFailure(101);
		}
		//�������� ��������� ��������� ��������
		if (foundRoutes != routeReg.size())
		{
			res->addFailure(102);
		}
		//����������� ������� � ���������
		for (MismatchMap::iterator it = mismatch.begin(); it != mismatch.end(); it++)
		{
			if (it->second)
			{
				res->addFailure(it->first);
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

}
}
}

