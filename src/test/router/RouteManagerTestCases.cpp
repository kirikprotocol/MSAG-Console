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
using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, TestSmeProxy
using namespace smsc::test::core; //route utils, CoreTestManager
using smsc::util::Logger;
using smsc::sms::AddressValue;
using smsc::test::sms::SmsUtil;
using smsc::test::core::RouteUtil;

void RouteManagerTestCases::debugRoute(RouteInfo& route)
{
	ostringstream os;
	os << route << endl;
	getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
}

RouteManagerTestCases::RouteManagerTestCases()
	: routeMan(CoreTestManager::getRouteManager()) {}

Category& RouteManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("RouteManagerTestCases");
	return log;
}

TCResult* RouteManagerTestCases::addCorrectRoute(const SmeSystemId& smeSystemId,
	TestRouteData* data, int num)
{
	int num1 = 8; int num2 = 8; int num3 = 2;
	TCSelector s(num, num1 * num2 * num3);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE, s.getChoice());
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
			AddressValue origAddrVal;
			uint8_t origAddrLen = data->origAddr.getValue(origAddrVal);
			int oaLen = rand1(origAddrLen);
			int oaLen2 = rand1(oaLen);
			switch(s.value1(num1, num2))
			{
				case 1: //source ����� � ����� ��� ����� '?' � �����
					memset(origAddrVal + origAddrLen - oaLen, '?', oaLen);
					route->source.setValue(origAddrLen, origAddrVal);
					data->origAddrMatch = 100 - oaLen;
					break;
				case 2: //source ����� ������� �� '?'
					memset(origAddrVal, '?', origAddrLen);
					route->source.setValue(origAddrLen, origAddrVal);
					data->origAddrMatch = 100 - origAddrLen;
					break;
				case 3: //source ����� � ������ '?'
					if (origAddrLen < MAX_ADDRESS_VALUE_LENGTH)
					{
						origAddrVal[origAddrLen] = '?';
						route->source.setValue(origAddrLen + 1, origAddrVal);
					}
					else
					{
						origAddrVal[origAddrLen - 1 ] = '+';
						route->source.setValue(origAddrLen, origAddrVal);
					}
					data->match = false;
					break;
				case 4: //source ����� ������� ��� �������� �� '*' � �����
					memset(origAddrVal + origAddrLen - oaLen, '*', oaLen2);
					route->source.setValue(origAddrLen - oaLen + oaLen2, origAddrVal);
					data->origAddrMatch = 100 - oaLen - 0.5; //'*' ����� ����������� '?'
					break;
				case 5: //source ����� �� '*'
					memset(origAddrVal, '*', oaLen);
					route->source.setValue(oaLen, origAddrVal);
					data->origAddrMatch = 100 - origAddrLen - 0.5;
					break;
				case 6: //source ����� � ������ '*'
					if (origAddrLen < MAX_ADDRESS_VALUE_LENGTH)
					{
						origAddrVal[origAddrLen] = '*';
						route->source.setValue(origAddrLen + 1, origAddrVal);
					}
					data->origAddrMatch = 100;
					break;
				case 7: //source ����� � ������������� typeOfNumber
					route->source.setTypeOfNumber(
						data->origAddr.getTypeOfNumber() + 1);
					data->match = false;
					break;
				case 8: //source ����� � ������������� numberingPlan
					route->source.setNumberingPlan(
						data->origAddr.getNumberingPlan() + 1);
					data->match = false;
					break;
				default:
					throw s;
			}
			//destAddr
			route->dest = data->destAddr;
			AddressValue destAddrVal;
			uint8_t destAddrLen = data->destAddr.getValue(destAddrVal);
			int daLen = rand1(destAddrLen);
			int daLen2 = rand1(daLen);
			switch(s.value2(num1, num2))
			{
				case 1: //dest ����� � ����� ��� ����� '?' � �����
					memset(destAddrVal + destAddrLen - daLen, '?', daLen);
					route->dest.setValue(destAddrLen, destAddrVal);
					data->destAddrMatch = 100 - daLen;
					break;
				case 2: //dest ����� ������� �� '?'
					memset(destAddrVal, '?', destAddrLen);
					route->dest.setValue(destAddrLen, destAddrVal);
					data->destAddrMatch = 100 - destAddrLen;
					break;
				case 3: //dest ����� � ������ '?'
					if (destAddrLen < MAX_ADDRESS_VALUE_LENGTH)
					{
						destAddrVal[destAddrLen] = '?';
						route->dest.setValue(destAddrLen + 1, destAddrVal);
					}
					else
					{
						destAddrVal[destAddrLen - 1] = '+';
						route->dest.setValue(destAddrLen, destAddrVal);
					}
					data->match = false;
					break;
				case 4: //dest ����� ������� ��� �������� �� '*' � �����
					memset(destAddrVal + destAddrLen - daLen, '*', daLen2);
					route->dest.setValue(destAddrLen - daLen + daLen2, destAddrVal);
					data->destAddrMatch = 100 - daLen - 0.5; //'*' ����� ����������� '?'
					break;
				case 5: //dest ����� �� '*'
					memset(destAddrVal, '*', daLen);
					route->dest.setValue(daLen, destAddrVal);
					data->destAddrMatch = 100 - destAddrLen - 0.5;
					break;
				case 6: //dest ����� � ������ '*'
					if (destAddrLen < MAX_ADDRESS_VALUE_LENGTH)
					{
						destAddrVal[destAddrLen] = '*';
						route->dest.setValue(destAddrLen + 1, destAddrVal);
					}
					data->destAddrMatch = 100;
					break;
				case 7: //dest ����� � ������������� typeOfNumber
					route->dest.setTypeOfNumber(
						data->destAddr.getTypeOfNumber() + 1);
					data->match = false;
					break;
				case 8: //dest ����� � ������������� numberingPlan
					route->dest.setNumberingPlan(
						data->destAddr.getNumberingPlan() + 1);
					data->match = false;
					break;
				default:
					throw s;
			}
			//���������
			switch(s.value3(num1, num2))
			{
				case 1: //������������ ���������
					route->priority = RoutePriorityMax;
					break;
				case 2: //����������� ���������
					route->priority = RoutePriorityMin;
					break;
				default:
					throw s;
			}
__require__(route->source.getLenght());
__require__(route->dest.getLenght());
debugRoute(*route);
			routeMan->addRoute(*route);
		}
		catch(...)
		{
			error();
			delete route;
			data->route = NULL;
			res->addFailure(200);
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addCorrectRoute2(const SmeSystemId& smeSystemId,
	TestRouteData* data, int num)
{
	TCSelector s(num, 8, 1000);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE, s.getChoice());
	for (; s.check(); s++)
	{
		RouteInfo* route = new RouteInfo();
		data->match = false;
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
debugRoute(*route);
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
					break;
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

TCResult* RouteManagerTestCases::lookupRoute(const RouteRegistry& routeReg,
	const Address& origAddr, const Address& destAddr)
{
	TCResult* res = new TCResult(TC_LOOKUP_ROUTE);
	try
	{
		SmeProxy* proxy;
		bool found = routeMan->lookup(origAddr, destAddr, proxy);
		const vector<uint32_t> ids = routeReg.lookup(origAddr, destAddr);
		if (!found)
		{
			if (ids.size())
			{
				res->addFailure(101);
			}
		}
		else if (!proxy)
		{
			res->addFailure(102);
		}
		else
		{
			uint32_t id = proxy->getUniqueId();
			found = false;
			for (int i = 0; i < ids.size(); i++)
			{
				if (ids[i] == id)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				res->addFailure(104);
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

