#include "RouteManagerTestCases.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/util/TCResultFilter.hpp"
#include "test/util/CheckList.hpp"
#include "util/Logger.h"
#include "util/debug.h"

#define ENTER __trace2__("Enter in %s", __PRETTY_FUNCTION__);
#define LEAVE __trace2__("Leave from %s", __PRETTY_FUNCTION__);

using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, SmeManagerTestCases
using namespace smsc::test::router; //constants, TestRouteData
using log4cpp::Category;
using smsc::sms::Address;
using smsc::util::Logger;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeProxy;
using smsc::test::sms::SmsUtil;
using smsc::test::util::CheckList;
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
	vector<TCResultStack*> stack;

public:
	RouteManagerFunctionalTest(SmeManager* smeMan, RouteManager* routeMan,
		RouteRegistry* routeReg);
	~RouteManagerFunctionalTest();
	void executeTest(TCResultFilter* filter, int numAddr);
	void printRoutes();

private:
	RouteInfo prepareForNewRoute(const Address& origAddr,
		const Address& destAddr, bool createProxy);
	void executeTestCases(const Address& origAddr, const Address& destAddr);
	void printRoute(const RouteInfo* route);
};

RouteManagerFunctionalTest::RouteManagerFunctionalTest(SmeManager* _smeMan,
	RouteManager* _routeMan, RouteRegistry* _routeReg)
	: smeMan(_smeMan), routeMan(_routeMan), tcSme(_smeMan),
	tcRoute(_routeMan, _routeReg) {}

RouteManagerFunctionalTest::~RouteManagerFunctionalTest()
{
	for (int i  = 0; i < addr.size(); i++)
	{
		delete addr[i];
	}
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
	for (int i = 0; i < stack.size(); i++)
	{
		delete stack[i];
	}
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
	stack.back()->push_back(tcSme.addCorrectSme(sme.back(), RAND_TC));
	if (createProxy)
	{
		SmeProxy* p;
		stack.back()->push_back(tcSme.registerCorrectSmeProxy(
			sme.back()->systemId, &p));
		proxy.push_back(p);
	}
	RouteInfo route;
	route.smeSystemId = sme.back()->systemId;
	route.source = origAddr;
	route.dest = destAddr;
	return route;
}

void RouteManagerFunctionalTest::executeTestCases(
	const Address& origAddr, const Address& destAddr)
{
	log.debug("*** start ***");

	//Создание нового стека для origAddr, destAddr
	stack.push_back(new TCResultStack());

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
					stack.back()->push_back(tcRoute.addCorrectRouteMatch(
						&route, proxy.back(), RAND_TC));
					if (!existentRoute)
					{
						existentRoute = new RouteInfo(route);
					}
				}
				break;
			case 2:
				{
					RouteInfo route = prepareForNewRoute(origAddr, destAddr, false);
					stack.back()->push_back(tcRoute.addCorrectRouteMatch(
						&route, NULL, RAND_TC));
					if (!existentRoute)
					{
						existentRoute = new RouteInfo(route);
					}
				}
				break;
			case 3:
				{
					RouteInfo route = prepareForNewRoute(origAddr, destAddr, true);
					stack.back()->push_back(tcRoute.addCorrectRouteNotMatch(
						&route, proxy.back(), RAND_TC));
					if (!existentRoute)
					{
						existentRoute = new RouteInfo(route);
					}
				}
				break;
			case 4:
				{
					RouteInfo route = prepareForNewRoute(origAddr, destAddr, true);
					stack.back()->push_back(tcRoute.addCorrectRouteNotMatch2(
						&route, proxy.back(), RAND_TC));
					if (!existentRoute)
					{
						existentRoute = new RouteInfo(route);
					}
				}
				break;
			case 5:
				if (existentRoute)
				{
					TCResult* res = tcRoute.addIncorrectRoute(
						*existentRoute, RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			/*
			default: //case 6..7
				{
					TCResult* res = tcRoute.lookupRoute(origAddr, destAddr);
					stack.back()->push_back(res);
				}
			*/
		}
	}
	if (existentRoute)
	{
		delete existentRoute;
	}
}

void RouteManagerFunctionalTest::executeTest(
	TCResultFilter* filter, int numAddr)
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
			TCResult* res = tcRoute.lookupRoute(origAddr, destAddr);
			filter->addResult(res);
			delete res;
		}
	}

	//Итерирование по списку маршрутов
	{
		TCResult* res = tcRoute.iterateRoutes();
		filter->addResult(res);
		delete res;
	}

	/*
	//Удаление зарегистрированных sme
	for (int i = 0; i < sme.size(); i++)
	{
		TCResult* res = tcSme.deleteExistentSme(sme[i]->systemId);
		filter->addResult(res);
		delete res;
	}

	//Поиск маршрута для каждой пары адресов
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAddr = *addr[j];
			TCResult* res = tcRoute.lookupRoute(routeReg, origAddr, destAddr);
			filter->addResult(res);
			delete res;
		}
	}

	//Итерирование по списку маршрутов
	{
		TCResult* res = tcRoute.iterateRoutes();
		filter->addResult(res);
		delete res;
	}
	*/

	//обработка результатов
	for (int i = 0; i < stack.size(); i++)
	{
		filter->addResultStack(*stack[i]);
	}
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "Сохранение checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Route Manager", "smsc::router");
	//имплементированные тест кейсы
	cl.writeResult("Добавление корректного рабочего маршрута",
		filter->getResults(TC_ADD_CORRECT_ROUTE_MATCH));
	cl.writeResult("Добавление корректного нерабочего маршрута",
		filter->getResults(TC_ADD_CORRECT_ROUTE_NOT_MATCH));
	cl.writeResult("Добавление некорректного маршрута",
		filter->getResults(TC_ADD_INCORRECT_ROUTE));
	cl.writeResult("Поиск маршрута",
		filter->getResults(TC_LOOKUP_ROUTE));
	cl.writeResult("Итерирование по списку маршрутов",
		filter->getResults(TC_ITERATE_ROUTES));
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
		TCResultFilter* filter = new TCResultFilter();
		for (int i = 0; i < numCycles; i++)
		{
			SmeManager smeMan;
			RouteManager routeMan;
			routeMan.assign(&smeMan);
			RouteRegistry routeReg;
			RouteManagerFunctionalTest test(&smeMan, &routeMan, &routeReg);
			test.executeTest(filter, numAddr);
			test.printRoutes();
		}
		saveCheckList(filter);
		delete filter;
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

