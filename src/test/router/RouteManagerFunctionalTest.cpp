#include "RouteManagerTestCases.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/util/TCResultFilter.hpp"
#include "test/util/CheckList.hpp"
#include "util/Logger.h"

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

static Category& log = Logger::getCategory("RouteManagerFunctionalTest");

class RouteManagerFunctionalTest
{
	SmeManagerTestCases tcSme;
	RouteManagerTestCases tcRoute;
	vector<Address*> addr;
	vector<SmeInfo*> sme;
	RouteRegistry routeReg;
	vector<TCResultStack*> stack;

public:
	~RouteManagerFunctionalTest();
	void executeTest(TCResultFilter* filter, int numAddr);
	void printRoutes();

private:
	TestRouteData* prepareForNewRoute(const Address& origAddr,
		const Address& destAddr);
	void executeTestCases(const Address& origAddr, const Address& destAddr);
};

RouteManagerFunctionalTest::~RouteManagerFunctionalTest()
{
	for (int i  = 0; i < addr.size(); i++)
	{
		delete addr[i];
	}
	for (int i = 0; i < sme.size(); i++)
	{
		delete sme[i];
	}
	for (int i = 0; i < stack.size(); i++)
	{
		delete stack[i];
	}
}

void RouteManagerFunctionalTest::printRoutes()
{
	RouteRegistry::RouteIterator* it = routeReg.iterator();
	for (; it->hasNext(); (*it)++)
	{
		ostringstream os;
		os << **it << endl;
		log.debug("%s", os.str().c_str());
	}
	delete it;
}

TestRouteData* RouteManagerFunctionalTest::prepareForNewRoute(
	const Address& origAddr, const Address& destAddr)
{
	TestRouteData* routeData = new TestRouteData(origAddr, destAddr);
	//Для каждого маршрута - отдельная sme (для идентификации)
	sme.push_back(new SmeInfo());
	stack.back()->push_back(tcSme.addCorrectSme(sme.back(), RAND_TC));
	stack.back()->push_back(tcSme.registerCorrectSmeProxy(sme.back()->systemId,
		&routeData->proxyId));
	return routeData;
}

void RouteManagerFunctionalTest::executeTestCases(
	const Address& origAddr, const Address& destAddr)
{
	log.debug("*** start ***");

	//Создание нового стека для origAddr, destAddr
	stack.push_back(new TCResultStack());

	//Добавление корректного маршрута
	//Добавление корректного маршрута с неправильными (непроверяемыми) значениями
	//Добавление некорректного маршрута
	//Поиск маршрута
	TestRouteData* routeData = NULL;
	for (TCSelector s(RAND_SET_TC, 6); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
			case 2:
				{
					if (routeData)
					{
						delete routeData;
					}
					routeData = prepareForNewRoute(origAddr, destAddr);
					TCResult* res = tcRoute.addCorrectRoute(
						sme.back()->systemId, routeData, RAND_TC);
					routeReg.putRoute(*routeData);
					stack.back()->push_back(res);
				}
				break;
			case 3:
				{
					if (routeData)
					{
						delete routeData;
					}
					routeData = prepareForNewRoute(origAddr, destAddr);
					TCResult* res = tcRoute.addCorrectRoute2(
						sme.back()->systemId, routeData, RAND_TC);
					routeReg.putRoute(*routeData);
					stack.back()->push_back(res);
				}
				break;
			case 4:
				if (routeData)
				{
					TCResult* res = tcRoute.addIncorrectRoute(
						sme.back()->systemId, *routeData->route, RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			default: //case 5..6
				{
					TCResult* res = tcRoute.lookupRoute(routeReg, origAddr, destAddr);
					stack.back()->push_back(res);
				}
		}
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

	//Поиск маршрута для каждой пары адресов
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAddr = *addr[j];
			TCResult* res = tcRoute.lookupRoute(routeReg, origAddr, destAddr);
			filter->addResult(res);
		}
	}

	//Итерирование по списку маршрутов
	filter->addResult(tcRoute.iterateRoutes(routeReg));

	//Удаление зарегистрированных sme
	for (int i = 0; i < sme.size(); i++)
	{
		TCResult* res = tcSme.deleteExistentSme(sme[i]->systemId);
		filter->addResult(res);
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
		}
	}

	//Итерирование по списку маршрутов
	filter->addResult(tcRoute.iterateRoutes(routeReg));

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
	cl.writeResult("Добавление корректного маршрута",
		filter->getResults(TC_ADD_CORRECT_ROUTE));
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
			RouteManagerFunctionalTest test;
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

