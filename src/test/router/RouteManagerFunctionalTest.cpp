#include "RouteManagerTestCases.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "RouteManagerCheckList.hpp"
#include "logger/Logger.h"
#include "util/debug.h"

using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, SmeManagerTestCases
using namespace smsc::test::router; //constants, TestRouteData
using log4cpp::Category;
using smsc::sms::Address;
using smsc::util::Logger;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeProxy;
using smsc::test::sms::SmsUtil;
using smsc::test::core::operator<<;

static Category& log = Logger::getCategory("RouteManagerFunctionalTest");

class RouteManagerFunctionalTest
{
	SmeManager* smeMan;
	RouteManager* routeMan;
	SmeManagerTestCases tcSme;
	RouteManagerTestCases tcRoute;
	vector<Address*> addr;
	vector<SmeInfo*> sme;
	vector<SmeProxy*> proxy;

public:
	RouteManagerFunctionalTest(SmeManager* smeMan, RouteManager* routeMan,
		RouteRegistry* routeReg, SmeRegistry* smeReg, CheckList* chkList);
	~RouteManagerFunctionalTest();
	void executeTest(int numAddr);
	void printRoutes();

private:
	RouteInfo prepareForNewRoute(const Address& origAddr,
		const Address& destAddr, bool createProxy);
	void executeTestCases(const Address& origAddr, const Address& destAddr);
	void printRoute(const RouteInfo* route);
};

RouteManagerFunctionalTest::RouteManagerFunctionalTest(SmeManager* _smeMan,
	RouteManager* _routeMan, RouteRegistry* routeReg, SmeRegistry* smeReg,
	CheckList* chkList)
	: smeMan(_smeMan), routeMan(_routeMan), tcSme(smeMan, smeReg, NULL),
	tcRoute(routeMan, routeReg, chkList) {}

RouteManagerFunctionalTest::~RouteManagerFunctionalTest()
{
	for_each(addr.begin(), addr.end(), Deletor<Address>());
	for (int i = 0; i < sme.size(); i++)
	{
		smeMan->unregisterSmeProxy(sme[i]->systemId);
		delete sme[i];
	}
	/*
	for (int i = 0; i < proxy.size(); i++)
	{
		SmeProxy* p = dynamic_cast<smsc::smeman::SmeRecord*>(proxy[i])->proxy;
		if (p)
		{
			delete p;
		}
	}
	*/
}

void RouteManagerFunctionalTest::printRoute(const RouteInfo* route)
{
	ostringstream os;
	os << *route;
	log.debug("[%d]\t%s", thr_self(), os.str().c_str());
}

void RouteManagerFunctionalTest::printRoutes()
{
	/*
	RouteRegistry::RouteIterator* it = routeReg.iterator();
	for (; it->hasNext(); (*it)++)
	{
		printRoute(**it);
	}
	delete it;
	*/
}

RouteInfo RouteManagerFunctionalTest::prepareForNewRoute(
	const Address& origAddr, const Address& destAddr, bool createProxy)
{
	//Для каждого маршрута - отдельная sme (для идентификации)
	sme.push_back(new SmeInfo());
	Address smeAddr;
	tcSme.addCorrectSme(&smeAddr, sme.back(), RAND_TC);
	if (createProxy)
	{
		SmeProxy* p = NULL;
		tcSme.registerCorrectSmeProxy(sme.back()->systemId, &p);
		proxy.push_back(p);
	}
	RouteInfo route;
	route.smeSystemId = sme.back()->systemId;
	route.source = origAddr;
	route.dest = destAddr;
	route.enabling = rand0(2);
	return route;
}

void RouteManagerFunctionalTest::executeTestCases(
	const Address& origAddr, const Address& destAddr)
{
	log.debug("*** start ***");

	//Добавление корректного рабочего маршрута, 1/5
	//Добавление корректного нерабочего маршрута, 2/5
	//Добавление корректного маршрута с неправильными (непроверяемыми) значениями, 1/5
	//Добавление некорректного маршрута, 1/5
	RouteInfo* existentRoute = NULL;
	for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				{
					RouteInfo route = prepareForNewRoute(origAddr, destAddr, true);
					tcRoute.addCorrectRouteMatch(&route, proxy.back(), RAND_TC);
					if (!existentRoute)
					{
						existentRoute = new RouteInfo(route);
					}
				}
				break;
			case 2:
				{
					RouteInfo route = prepareForNewRoute(origAddr, destAddr, false);
					tcRoute.addCorrectRouteMatch(&route, NULL, RAND_TC);
					if (!existentRoute)
					{
						existentRoute = new RouteInfo(route);
					}
				}
				break;
			case 3:
				{
					RouteInfo route = prepareForNewRoute(origAddr, destAddr, true);
					tcRoute.addCorrectRouteNotMatch(&route, proxy.back(), RAND_TC);
					if (!existentRoute)
					{
						existentRoute = new RouteInfo(route);
					}
				}
				break;
			case 4:
				{
					RouteInfo route = prepareForNewRoute(origAddr, destAddr, true);
					tcRoute.addCorrectRouteNotMatch2(&route, proxy.back(), RAND_TC);
					if (!existentRoute)
					{
						existentRoute = new RouteInfo(route);
					}
				}
				break;
			case 5:
				if (existentRoute)
				{
					tcRoute.addIncorrectRoute(*existentRoute, RAND_TC);
				}
				break;
			/*
			default: //case 6..7
				{
					tcRoute.lookupRoute(origAddr, destAddr);
				}
			*/
		}
	}
	if (existentRoute)
	{
		delete existentRoute;
	}
}

void RouteManagerFunctionalTest::executeTest(int numAddr)
{
	//Подготовка списка адресов
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		SmsUtil::setupRandomCorrectAddress(addr.back());
	}

	//На каждую пару адресов регистрация случайного количества маршрутов
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAddr = *addr[j];
			executeTestCases(origAddr, destAddr);
		}
	}
	tcRoute.commit();
	
	//Поиск маршрута для каждой пары адресов
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAddr = *addr[j];
			tcRoute.lookupRoute(origAddr, destAddr);
		}
	}

	/*
	//Итерирование по списку маршрутов
	tcRoute.iterateRoutes();
	
	//Удаление зарегистрированных sme
	for (int i = 0; i < sme.size(); i++)
	{
		tcSme.deleteExistentSme(sme[i]->systemId);
	}

	//Поиск маршрута для каждой пары адресов
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAddr = *addr[j];
			tcRoute.lookupRoute(routeReg, origAddr, destAddr);
		}
	}

	//Итерирование по списку маршрутов
	{
		tcRoute.iterateRoutes();
	}
	*/
}

/**
 * Выполняет тестирование RouteManager и
 * выводит результат по тест кейсам в checklist.
 */
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: RouteManagerFunctionalTest <numCycles> <numAddr>" << endl;
		exit(0);
	}

	const int numCycles = atoi(argv[1]);
	const int numAddr = atoi(argv[2]);
	try
	{
		//Manager::init("config.xml");
		RouteManagerCheckList chkList;
		for (int i = 0; i < numCycles; i++)
		{
			SmeManager smeMan;
			RouteManager routeMan;
			routeMan.assign(&smeMan);
			RouteRegistry routeReg;
			SmeRegistry smeReg;
			RouteManagerFunctionalTest test(&smeMan, &routeMan, &routeReg, &smeReg, &chkList);
			test.executeTest(numAddr);
			test.printRoutes();
		}
		chkList.saveHtml();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

