#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "test/sme/SmppProfilerTestCases.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/alias/AliasManagerTestCases.hpp"
#include "test/router/RouteManagerTestCases.hpp"
#include "test/profiler/ProfilerTestCases.hpp"
#include "test/core/ProfileUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/config/SmeConfigGen.hpp"
#include "test/config/AliasConfigGen.hpp"
#include "test/config/RouteConfigGen.hpp"
#include "test/util/TestTaskManager.hpp"
#include "SmppProfilerCheckList.hpp"
#include "test/config/ConfigGenCheckList.hpp"
#include "util/debug.h"
#include <vector>
#include <sstream>
#include <iostream>

//#define SIMPLE_TEST
#ifdef SIMPLE_TEST
	#undef ASSERT
#endif

using smsc::sme::SmeConfig;
using smsc::smeman::SmeInfo;
using smsc::alias::AliasInfo;
using smsc::test::sms::SmsUtil;
using smsc::test::sms::operator<<;
using smsc::test::smeman::SmeManagerTestCases;
using smsc::test::alias::AliasManagerTestCases;
using smsc::test::router::RouteManagerTestCases;
using smsc::test::profiler::ProfilerTestCases;
using smsc::test::conf::TestConfig;
using smsc::test::core::ProfileUtil;
using smsc::test::core::RouteHolder;
using namespace std;
using namespace smsc::sms;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads; //ThreadPool, ThreadedTask
using namespace smsc::test::sme;
using namespace smsc::test::config;
using namespace smsc::test::util;

SmeRegistry* smeReg;
AliasRegistry* aliasReg;
RouteRegistry* routeReg;
Profile defProfile;
ProfileRegistry* profileReg;
CheckList* smppChkList;
CheckList* configChkList;

/**
 * Тестовая sme.
 */
class TestSme : public TestTask, SmppResponseSender
{
	int smeNum;
	SmppProfilerTestCases tc;
	time_t nextCheckTime;
	bool boundOk;
	Event evt;

public:
	TestSme(int smeNum, const SmeConfig& config, SmppFixture* fixture);
	virtual ~TestSme() {}
	virtual void executeCycle();
	virtual void onStopped();

private:
	virtual uint32_t sendDeliverySmResp(PduDeliverySm& pdu);
	virtual void updateStat();
};

/**
 * Таск менеджер.
 */
class TestSmeTaskManager
	: public TestTaskManager<TestSme>
{
public:
	TestSmeTaskManager() {}
	virtual bool isStopped() const;
};

/**
 * Статистика работы sme.
 */
struct TestSmeStat
{
	int ops;
	bool stopped;
	TestSmeStat() : ops(0), stopped(false) {}
};

/**
 * Статистика работы всего теста.
 */
class SmppFunctionalTest
{
	typedef vector<TestSmeStat> TaskStatList;
	
	static TaskStatList taskStat;
	static Mutex mutex;

public:
	static int delay;
	static bool pause;
	
public:
	static void resize(int newSize);
	static void onStopped(int taskNum);
	static bool isStopped();
	static void updateStat(int taskNum);
	static void printStat();
};

//TestSme
TestSme::TestSme(int num, const SmeConfig& config, SmppFixture* fixture)
	: TestTask("TestSme", num), smeNum(num), nextCheckTime(0),
	tc(config, fixture), boundOk(false)
{
	fixture->respSender = this;
}

void TestSme::executeCycle()
{
	//Проверка неполученых подтверждений доставки, нотификаций и sms от других sme
	if (time(NULL) > nextCheckTime)
	{
		tc.getBase().checkMissingPdu();
		nextCheckTime = time(NULL) + 10;
	}
	//проверить тест остановлен/замедлен
	//__trace2__("TestSme::executeCycle(): SmppFunctionalTest::pause = %d", SmppFunctionalTest::pause);
	if (SmppFunctionalTest::pause)
	{
		evt.Wait(1000);
		__trace__("TestSme paused. Returned.");
		return;
	}
	//__trace__("TestSme active. Continued.");
	if (SmppFunctionalTest::delay)
	{
		evt.Wait(SmppFunctionalTest::delay);
	}
	//debug("*** start ***");
	//Bind sme зарегистрированной в smsc
	//Bind sme с неправильными параметрами
	if (!boundOk)
	{
		boundOk = tc.getBase().bindCorrectSme(RAND_TC);
		if (!boundOk)
		{
			cout << "Bound failed" << endl;
			exit(0);
		}
		for (int i = 0; i < 3; i++)
		{
			tc.getBase().bindIncorrectSme(RAND_TC); //обязательно после bindCorrectSme
		}
		evt.Wait(5000);
	}
	//Синхронная отправка submit_sm pdu другим sme
	//Асинхронная отправка submit_sm pdu другим sme
#ifdef SIMPLE_TEST
	for (TCSelector s(RAND_TC, 1); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				tc.getTransmitter().submitSmCorrect(rand0(1), 1);
				break;
		}
		updateStat();
	}
#else //!SIMPLE_TEST
	#ifdef ASSERT
	for (TCSelector s(RAND_TC, 3); s.check(); s++)
	#else
	for (TCSelector s(RAND_TC, 2); s.check(); s++)
	#endif //ASSERT
	{
		switch (s.value())
		{
			case 1:
				tc.submitSmCorrect(rand0(1), RAND_TC);
				break;
			case 2:
				tc.submitSmIncorrect(rand0(1), RAND_TC);
				break;
	#ifdef ASSERT
			case 3:
				tc.submitSmAssert(RAND_TC);
				break;
	#endif //ASSERT
		}
		updateStat();
	}
#endif //SIMPLE_TEST
}

void TestSme::onStopped()
{
	tc.getBase().unbind(); //Unbind для sme соединенной с smsc
	tc.getBase().unbind(); //Unbind для sme несоединенной с smsc
	SmppFunctionalTest::onStopped(smeNum);
	cout << "TestSme::onStopped(): sme = " << smeNum << endl;
}

uint32_t TestSme::sendDeliverySmResp(PduDeliverySm& pdu)
{
#ifdef SIMPLE_TEST
	return tc.getTransmitter().sendDeliverySmRespOk(pdu, rand0(1));
#else
	switch (rand1(3))
	{
		case 1:
			return tc.sendDeliverySmRespError(pdu, rand0(1), RAND_TC);
		case 2:
			return tc.sendDeliverySmRespRetry(pdu, rand0(1), RAND_TC);
		default:
			return tc.sendDeliverySmRespOk(pdu, rand0(1));
	}
#endif //SIMPLE_TEST
}

void TestSme::updateStat()
{
	SmppFunctionalTest::updateStat(smeNum);
}

//TestSmeTaskManager
bool TestSmeTaskManager::isStopped() const
{
	return SmppFunctionalTest::isStopped();
}

//SmppFunctionalTest
int SmppFunctionalTest::delay = 500;
bool SmppFunctionalTest::pause = false;
SmppFunctionalTest::TaskStatList
	SmppFunctionalTest::taskStat =
	SmppFunctionalTest::TaskStatList();
Mutex SmppFunctionalTest::mutex = Mutex();
	
void SmppFunctionalTest::resize(int newSize)
{
	taskStat.clear();
	taskStat.resize(newSize);
}

void SmppFunctionalTest::onStopped(int taskNum)
{
	//MutexGuard guard(mutex);
	taskStat[taskNum].stopped = true;
}

bool SmppFunctionalTest::isStopped()
{
	//MutexGuard guard(mutex);
	bool stopped = true;
	for (int i = 0; i < taskStat.size(); i++)
	{
		if(!taskStat[i].stopped)
		{
			cout<< "Still running = " << i << endl;
		}
		stopped &= taskStat[i].stopped;
	}
	return stopped;
}

void SmppFunctionalTest::updateStat(int smeNum)
{
	//обновить статистику
	taskStat[smeNum].ops++;
}

void SmppFunctionalTest::printStat()
{
	for (int i = 0; i < taskStat.size(); i++)
	{
		cout << "sme_" << i << ": ops = " << taskStat[i].ops << endl;
	}
}

vector<TestSme*> genConfig(int numAddr, int numAlias, int numSme,
	const string& smscHost, int smscPort)
{
	__require__(numSme <= numAddr);
	__trace__("*** Generating config files ***");
	smeReg->clear();
	aliasReg->clear();
	routeReg->clear();
	SmeManagerTestCases tcSme(NULL, smeReg, NULL);
	AliasManagerTestCases tcAlias(NULL, aliasReg, NULL);
	RouteManagerTestCases tcRoute(NULL, routeReg, NULL);

	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	__cfg_str__(profilerSystemId);
	__cfg_str__(mapProxySystemId);
	
	vector<Address*> addr;
	vector<SmeInfo*> smeInfo;
	addr.reserve(numAddr);
	smeInfo.reserve(numAddr);
	//регистрация sme
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		smeInfo.push_back(new SmeInfo());
		tcSme.addCorrectSme(addr[i], smeInfo[i], RAND_TC);
		ostringstream os;
		os << *addr[i];
		__trace2__("genConfig(): addr = %s, systemId = %s", os.str().c_str(), smeInfo[i]->systemId.c_str());
	}
	//регистрация profiler
	SmeInfo profilerInfo;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&profilerInfo);
	profilerInfo.systemId = profilerSystemId;
	smeReg->registerSme(profilerAddr, profilerInfo, false, true);
	smeReg->bindSme(profilerInfo.systemId);
	//регистрация map proxy
	SmeInfo mapProxyInfo;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&mapProxyInfo);
	mapProxyInfo.systemId = mapProxySystemId;
	smeReg->registerSme("+123", mapProxyInfo, false, true);
	smeReg->bindSme(mapProxyInfo.systemId);
	//регистрация алиасов
	for (int i = 0; i < numAlias; i++)
	{
		for (int j = 0; j < numAlias; j++)
		{
			for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
			{
				AliasInfo alias;
				alias.alias = *addr[i];
				alias.addr = *addr[j];
				switch (s.value())
				{
					case 1:
					case 2:
					case 3:
						tcAlias.addCorrectAliasMatch(&alias, RAND_TC);
						break;
					case 4:
						tcAlias.addCorrectAliasNotMatchAddress(&alias, RAND_TC);
						break;
					case 5:
						tcAlias.addCorrectAliasNotMatchAlias(&alias, RAND_TC);
						break;
					default:
						__unreachable__("Invalid alias test case");
				}
			}
		}
	}
	//алиас для profiler
	AliasInfo profilerAliasInfo;
	profilerAliasInfo.addr = profilerAddr;
	profilerAliasInfo.alias = profilerAlias;
	profilerAliasInfo.hide = true;
	aliasReg->putAlias(profilerAliasInfo);
	//tcAlias->commit();
	//регистрация маршрутов
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
			{
				RouteInfo route;
				route.source = *addr[i];
				route.dest = *addr[j];
				route.smeSystemId = smeInfo[rand0(numAddr - 1)]->systemId;
				switch (s.value())
				{
					case 1:
					case 2:
					case 3:
						tcRoute.addCorrectRouteMatch(&route, NULL, RAND_TC);
						break;
					case 4:
						tcRoute.addCorrectRouteNotMatch(&route, NULL, RAND_TC);
						break;
					case 5:
						tcRoute.addCorrectRouteNotMatch2(&route, NULL, RAND_TC);
						break;
					default:
						__unreachable__("Invalid route test case");
				}
			}
		}
	}
	//маршруты на profiler
	for (int i = 0; i < numAddr; i++)
	{
		//sme -> profiler
		RouteInfo route1;
		route1.source = *addr[i];
		route1.dest = profilerAddr;
		route1.smeSystemId = profilerSystemId;
		route1.enabling = true;
		tcRoute.addCorrectRouteMatch(&route1, NULL, RAND_TC);
		//profiler -> sme
		RouteInfo route2;
		route2.source = profilerAddr;
		route2.dest = *addr[i];
		route2.smeSystemId = smeInfo[i]->systemId;
		route2.enabling = true;
		tcRoute.addCorrectRouteMatch(&route2, NULL, RAND_TC);
	}
	//tcRoute->commit();
	//сохранение конфигов
	configChkList->reset();
	SmeConfigGen smeCfg(smeReg, configChkList);
	AliasConfigGen aliasCfg(aliasReg, configChkList);
	RouteConfigGen routeCfg(routeReg, configChkList);
	smeCfg.saveConfig("../conf/sme.xml");
	aliasCfg.saveConfig("../conf/aliases.xml");
	routeCfg.saveConfig("../conf/routes.xml");
	//создание sme
	vector<TestSme*> sme;
	for (int i = 0; i < numSme; i++)
	{
		SmeConfig config;
		config.host = smscHost;
		config.port = smscPort;
		//config.sid = tmp.get();
		config.sid = smeInfo[i]->systemId;
		config.timeOut = 10;
		//config.password;
		//config.systemType;
		//config.origAddr;
		SmppFixture* fixture = new SmppFixture(smeInfo[i]->systemId, *addr[i],
			NULL, NULL, smeReg, aliasReg, routeReg, profileReg, smppChkList);
		sme.push_back(new TestSme(i, config, fixture)); //throws Exception
		smeReg->bindSme(smeInfo[i]->systemId);
	}
	//печать таблицы маршрутов
	__trace__("Route table");
	int numRoutes = 0;
	int numBound = 0;
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAlias = *addr[j];
			string smeId = "<>";
			bool smeBound = false;
			const Address destAddr = aliasReg->findAddressByAlias(destAlias);
			const RouteHolder* routeHolder =
				routeReg->lookup(origAddr, destAddr);
			if (routeHolder)
			{
				smeId = routeHolder->route.smeSystemId;
				smeBound = smeReg->isSmeBound(smeId);
				numRoutes++;
				if (smeBound)
				{
					numBound++;
				}
			}
			ostringstream os;
			os << "origAddr = " << origAddr << ", destAlias = " << destAlias;
			__trace2__("%s, route to = %s, sme bound = %s",
				os.str().c_str(), smeId.c_str(), (smeBound ? "yes" : "no"));
		}
	}
	if (!numBound)
	{
		cout << "Invalid routes generated" << endl;
		//exit(-1);
	}
	cout << "Valid routes: " << numRoutes << endl;
	cout << "Valid routes with sme: " << numBound << endl;
	for (int i = 0; i < numAddr; i++)
	{
		delete addr[i];
		delete smeInfo[i];
	}
	return sme;
}

void executeFunctionalTest(const string& smscHost, int smscPort)
{
	vector<TestSme*> sme;
	TestSmeTaskManager tm;
	tm.startTimer();
	//обработка команд консоли
	string cmd;
	bool help = true;
	while (true)
	{
		//хелп
		if (help)
		{
			help = false;
			cout << "conf <numAddr> <numAlias> <numSme> - generate config files" << endl;
			cout << "test <start|pause|resume> - pause/resume test execution" << endl;
			cout << "stat - print statistics" << endl;
			cout << "chklist - save checklist" << endl;
			cout << "dump pdu - dump pdu registry" << endl;
			cout << "set delay <msec> - slow down test cycle execution" << endl;
			cout << "quit - stop test and quit" << endl;
		}

		//обработка команд
		cin >> cmd;
		if (cmd == "conf")
		{
			int numAddr, numAlias, numSme;
			cin >> numAddr;
			cin >> numAlias;
			cin >> numSme;
			if (numAddr < numSme)
			{
				cout << "Must be: numAddr >= numSme" << endl;
			}
			else if (numAddr < numAlias)
			{
				cout << "Must be: numAddr >= numAlias" << endl;
			}
			else
			{
				sme = genConfig(numAddr, numAlias, numSme, smscHost, smscPort);
				cout << "Config generated" << endl;
			}
		}
		else if (cmd == "test")
		{
			cin >> cmd;
			if (cmd == "start")
			{
				SmppFunctionalTest::resize(sme.size());
				for (int i = 0; i < sme.size(); i++)
				{
					tm.addTask(sme[i]);
				}
				tm.startTimer();
				cout << "Started " << sme.size() << " sme" << endl;
			}
			else if (cmd == "pause")
			{
				SmppFunctionalTest::pause = true;
				cout << "Test paused successfully" << endl;
			}
			else if (cmd == "resume")
			{
				SmppFunctionalTest::pause = false;
				cout << "Test resumed successfully" << endl;
			}
			else
			{
				help = true;
			}
			__trace2__("executeFunctionalTest(): SmppFunctionalTest::pause = %d", SmppFunctionalTest::pause);
		}
		else if (cmd == "stat")
		{
			cout << "Time = " << tm.getExecutionTime() << endl;
			SmppFunctionalTest::printStat();
		}
		else if (cmd == "chklist")
		{
			smppChkList->save();
			smppChkList->saveHtml();
			configChkList->save();
			configChkList->saveHtml();
			cout << "Checklists saved" << endl;
		}
		else if (cmd == "dump")
		{
			cin >> cmd;
			if (cmd == "pdu")
			{
				smeReg->dump(TRACE_LOG_STREAM);
				cout << "Pdu registry dumped successfully" << endl;
			}
			else
			{
				help = true;
			}
		}
		else if (cmd == "set")
		{
			int newVal;
			cin >> cmd;
			cin >> newVal;
			if (cmd == "delay")
			{
				SmppFunctionalTest::delay = newVal;
			}
			else
			{
				help = true;
			}
		}
		else if (cmd == "quit")
		{
			//smppChkList->saveHtml();
			//configChkList->saveHtml();
			//cout << "Checklist saved" << endl;
			tm.stopTasks();
			cout << "Total time = " << tm.getExecutionTime() << endl;
			return;
		}
		else
		{
			help = true;
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc != 1 && argc != 3)
	{
		cout << "Usage: TestSmsc [host] [port]" << endl;
		exit(0);
	}
	string smscHost = "smsc";
	int smscPort = 15971;
	if (argc == 3)
	{
		smscHost = argv[1];
		smscPort = atoi(argv[2]);
	}
	smeReg = new SmeRegistry();
	aliasReg = new AliasRegistry();
	routeReg = new RouteRegistry();
	ProfileUtil::setupRandomCorrectProfile(defProfile);
	profileReg = new ProfileRegistry(defProfile);
	smppChkList = new SmppProfilerCheckList();
	configChkList = new ConfigGenCheckList();
	try
	{
		executeFunctionalTest(smscHost, smscPort);
	}
	catch (exception& e)
	{
		cout << "Failed to execute test: " << e.what() << endl;
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}
	delete smeReg;
	delete aliasReg;
	delete routeReg;
	delete profileReg;
	delete smppChkList;
	delete configChkList;
	return 0;
}

