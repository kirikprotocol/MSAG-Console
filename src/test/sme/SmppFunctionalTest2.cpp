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
#include "test/util/TextUtil.hpp"
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
using smsc::test::smpp::SmppUtil;
using smsc::test::sms::operator<<;
using smsc::test::sms::operator==;
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
using namespace smsc::profiler;
using namespace smsc::test::sms;
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
 * “естова€ sme.
 */
class TestSme : public TestTask, SmppResponseSender
{
	int smeNum;
	SmppProfilerTestCases tc;
	time_t nextCheckTime;
	bool boundOk;
	Event evt;
	int idx;
	vector<int> seq;

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
 * “аск менеджер.
 */
class TestSmeTaskManager
	: public TestTaskManager<TestSme>
{
public:
	TestSmeTaskManager() {}
	virtual bool isStopped() const;
};

/**
 * —татистика работы sme.
 */
struct TestSmeStat
{
	int ops;
	bool stopped;
	TestSmeStat() : ops(0), stopped(false) {}
};

/**
 * —татистика работы всего теста.
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
	tc(config, fixture), boundOk(false), idx(0)
{
	fixture->respSender = this;
}

void TestSme::executeCycle()
{
	//ѕроверка неполученых подтверждений доставки, нотификаций и sms от других sme
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
		//стартова€ последовательност команд
		seq.push_back(1);
		seq.insert(seq.end(), 3, 3);
		seq.insert(seq.end(), 3, 2);
		seq.push_back(1);
		for (int i = 0; i < seq.size(); i++)
		{
			switch (seq[i])
			{
				case 1: //правильный bind
					evt.Wait(2000);
					boundOk = tc.getBase().bindCorrectSme(RAND_TC);
					if (!boundOk)
					{
						cout << "Bound failed" << endl;
						exit(0);
					}
					break;
				case 2: //неправильный bind
					tc.getBase().bindIncorrectSme(RAND_TC);
					break;
				case 3: //unbind
					tc.getBase().unbind();
					boundOk = false;
					break;
				default:
					__unreachable__("Invalid seq number");
			}
		}
		__require__(boundOk);
		evt.Wait(3000);
		//тестова€ последовательност команд
		seq.clear();
#ifdef SIMPLE_TEST
	seq.push_back(201);
#else
	seq.insert(seq.end(), 30, 1);
	//seq.insert(seq.end(), 10, 2);
	seq.push_back(3);
	seq.push_back(4);
	seq.push_back(5);
#ifdef ASSERT
	seq.push_back(101);
#endif //ASSERT
#endif //SIMPLE_TEST
	random_shuffle(seq.begin(), seq.end());
	}
	idx = idx < seq.size() ? idx : 0;
	switch (seq[idx++])
	{
		case 1: //правильна€ sms
			tc.submitSmCorrect(rand0(1), RAND_TC);
			break;
		case 2: //неправильна€ sms
			tc.submitSmIncorrect(rand0(1), RAND_TC);
			break;
		case 3: //обновление настроек кодировки
			tc.updateCodePageCorrect(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 4: //обновление настроек уведомлени€ о доствке
			tc.updateReportOptionsCorrect(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 5: //обновление профил€ некорректными данными
			tc.updateProfileIncorrect(rand0(1), getDataCoding(RAND_TC));
			break;
		case 6:
			tc.replaceSm(rand0(1), RAND_TC);
			break;
		case 101: //asserts
			tc.submitSmAssert(RAND_TC);
			break;
		/*
		case 102:
			tc.replaceSmAssert(RAND_TC);
			break;
		*/
		case 201: //исключительно правильна€ sms
			tc.submitSmCorrect(rand0(1), 1);
			break;
		default:
			__unreachable__("Invalid seq number");
	}
	updateStat();
}

void TestSme::onStopped()
{
	tc.getBase().unbind(); //Unbind дл€ sme соединенной с smsc
	tc.getBase().unbind(); //Unbind дл€ sme несоединенной с smsc
	SmppFunctionalTest::onStopped(smeNum);
	cout << "TestSme::onStopped(): sme = " << smeNum << endl;
}

uint32_t TestSme::sendDeliverySmResp(PduDeliverySm& pdu)
{
	//на delivery receipt и сообщение от профайлера ответить ok
	Address addr;
	SmppUtil::convert(pdu.get_message().get_source(), &addr);
	__cfg_addr__(smscAddr);
	__cfg_addr__(smscAlias);
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	if (addr == smscAddr || addr == smscAlias ||
		addr == profilerAddr || addr == profilerAlias)
	{
		return tc.sendDeliverySmRespOk(pdu, rand0(1));
	}
	//на остальное ответить как придетс€
#ifdef SIMPLE_TEST
	return tc.sendDeliverySmRespOk(pdu, rand0(1));
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
int SmppFunctionalTest::delay = 1000;
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

void systemSmeConfig()
{
	//smsc sme
	__cfg_addr__(smscAddr);
	__cfg_addr__(smscAlias);
	__cfg_str__(smscSystemId);
	SmeInfo smscSme;
	smscSme.wantAlias = rand0(1);
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&smscSme);
	smscSme.systemId = smscSystemId;
	smeReg->registerSme(smscAddr, smscSme, false, true);
	smeReg->bindSme(smscSme.systemId);
	//алиас дл€ smsc sme
	AliasInfo smscAliasInfo;
	smscAliasInfo.addr = smscAddr;
	smscAliasInfo.alias = smscAlias;
	smscAliasInfo.hide = true; //rand0(2);
	aliasReg->putAlias(smscAliasInfo);
	//регистраци€ profiler
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	__cfg_str__(profilerSystemId);
	SmeInfo profilerSme;
	profilerSme.wantAlias = false;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&profilerSme);
	profilerSme.systemId = profilerSystemId;
	smeReg->registerSme(profilerAddr, profilerSme, false, true);
	smeReg->bindSme(profilerSme.systemId);
	//алиас дл€ profiler
	AliasInfo profilerAliasInfo;
	profilerAliasInfo.addr = profilerAddr;
	profilerAliasInfo.alias = profilerAlias;
	profilerAliasInfo.hide = true; //rand0(2);
	aliasReg->putAlias(profilerAliasInfo);
	//регистраци€ map proxy
	__cfg_str__(mapProxySystemId);
	SmeInfo mapProxySme;
	mapProxySme.wantAlias = rand0(1);
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&mapProxySme);
	mapProxySme.systemId = mapProxySystemId;
	smeReg->registerSme("+123", mapProxySme, false, true);
	smeReg->bindSme(mapProxySme.systemId);
	//abonent info прокси
	__cfg_str__(abonentInfoSystemId);
	SmeInfo abonentInfoSme;
	abonentInfoSme.wantAlias = false;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&abonentInfoSme);
	abonentInfoSme.systemId = abonentInfoSystemId;
	smeReg->registerSme("+321", abonentInfoSme, false, true);
	smeReg->bindSme(abonentInfoSme.systemId);
}

void checkRoute(const Address& origAddr, const SmeSystemId& origSmeId,
	const Address& destAlias, int* numRoutes = NULL, int* numBound = NULL)
{
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	const RouteHolder* routeHolder = routeReg->lookup(origAddr, destAddr);
	if (routeHolder)
	{
		if (numRoutes)
		{
			(*numRoutes)++;
		}
		const SmeSystemId& smeId = routeHolder->route.smeSystemId;
		bool smeBound = smeReg->isSmeBound(smeId);
		if (smeBound)
		{
			if (numBound)
			{
				(*numBound)++;
			}
		}
		__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, route to = %s, sme bound = %s",
			str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str(), smeId.c_str(),
			(smeBound ? "yes" : "no"));
	}
	else
	{
		__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, no route",
			str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str());
	}
}

void checkRoute2(const Address& origAddr, const SmeSystemId& origSmeId,
	const Address& destAlias)
{
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	const RouteHolder* routeHolder1 = routeReg->lookup(origAddr, destAddr);
	if (routeHolder1)
	{
		const SmeSystemId& smeId1 = routeHolder1->route.smeSystemId;
		bool smeBound1 = smeReg->isSmeBound(smeId1);
		const RouteHolder* routeHolder2 = routeReg->lookup(destAddr, origAddr);
		if (routeHolder2)
		{
			const SmeSystemId& smeId2 = routeHolder2->route.smeSystemId;
			bool smeBound2 = smeReg->isSmeBound(smeId2);
			__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, route to = %s, sme bound = %s, back route to = %s, sme bound = %s",
				str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str(),
				smeId1.c_str(), (smeBound1 ? "yes" : "no"),
				smeId2.c_str(), (smeBound2 ? "yes" : "no"));
		}
		else
		{
			__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, route to = %s, sme bound = %s, no back route",
				str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str(),
				smeId1.c_str(), (smeBound1 ? "yes" : "no"));
		}
	}
	else
	{
		__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, no route",
			str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str());
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

	__cfg_addr__(smscAddr);
	__cfg_str__(smscSystemId);
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	__cfg_str__(profilerSystemId);
	
	//системные sme
	systemSmeConfig();

	vector<Address*> addr;
	vector<Address*> alias;
	vector<SmeInfo*> smeInfo;
	addr.reserve(numAddr);
	alias.reserve(numAlias);
	smeInfo.reserve(numAddr);
	//регистраци€ sme
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		smeInfo.push_back(new SmeInfo());
		smeInfo.back()->wantAlias = rand0(1);
		tcSme.addCorrectSme(addr.back(), smeInfo.back(), RAND_TC);
		__trace2__("register sme: addr = %s, systemId = %s",
			str(*addr.back()).c_str(), smeInfo.back()->systemId.c_str());
	}
	//регистраци€ алиасов
	for (int i = 0; i < numAlias; i++)
	{
		alias.push_back(new Address());
		SmsUtil::setupRandomCorrectAddress(alias.back(),
			MAX_ADDRESS_VALUE_LENGTH / 2, MAX_ADDRESS_VALUE_LENGTH);
		smeReg->registerAddress(*alias.back());
		__trace2__("register alias: alias = %s", str(*alias.back()).c_str());
	}
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAlias; j++)
		{
			for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
			{
				AliasInfo aliasInfo;
				aliasInfo.addr = *addr[i];
				aliasInfo.alias = *alias[j];
				switch (s.value())
				{
					case 1:
					case 2:
					case 3:
						tcAlias.addCorrectAliasMatch(&aliasInfo, RAND_TC);
						break;
					case 4:
						tcAlias.addCorrectAliasNotMatchAddress(&aliasInfo, RAND_TC);
						break;
					case 5:
						tcAlias.addCorrectAliasNotMatchAlias(&aliasInfo, RAND_TC);
						break;
					default:
						__unreachable__("Invalid alias test case");
				}
			}
		}
	}
	//проверка
	__trace__("*** Checking alias constraint d(a(A)) = A ***");
	for (int i = 0; i < numAddr; i++)
	{
		const Address _alias = aliasReg->findAliasByAddress(*addr[i]);
		const Address _addr = aliasReg->findAddressByAlias(_alias);
		__trace2__("addr->alias->addr: %s -> %s -> %s",
			str(*addr[i]).c_str(), str(_alias).c_str(), str(_addr).c_str());
		if (_addr != *addr[i])
		{
			cout << "Alias constraint d(a(A)) = A violated" << endl;
			break;
		}
	}
	//tcAlias->commit();
	//регистраци€ маршрутов
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
				route.enabling = rand0(2);
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
	//маршруты на smsc (delivery receipts)
	for (int i = 0; i < numAddr; i++)
	{
		//smsc -> sme
		RouteInfo route;
		route.source = smscAddr;
		route.dest = *addr[i];
		route.smeSystemId = smeInfo[i]->systemId;
		route.enabling = true;
		tcRoute.addCorrectRouteMatch(&route, NULL, RAND_TC);
	}
	//tcRoute->commit();
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
	//печать таблицы маршрутов sme->sme
	__trace__("*** Route table ***");
	int numRoutes = 0;
	int numBound = 0;
	for (int i = 0; i < numAddr; i++)
	{
		const vector<const Address*>& addrList = smeReg->getAddressList();
		for (int j = 0; j < addrList.size(); j++)
		{
			checkRoute(*addr[i], smeInfo[i]->systemId, *addrList[j], &numRoutes, &numBound);
		}
	}
	//печать таблицы маршрутов smsc->sme
	for (int i = 0; i < numSme; i++)
	{
		checkRoute(smscAddr, smscSystemId, *addr[i]);
	}
	//печать таблицы маршрутов sme<->profiler
	for (int i = 0; i < numSme; i++)
	{
		checkRoute2(*addr[i], smeInfo[i]->systemId, profilerAlias);
	}
	if (!numBound)
	{
		cout << "Invalid routes generated" << endl;
		//exit(-1);
	}
	cout << "Valid routes: " << numRoutes << endl;
	cout << "Valid routes with sme: " << numBound << endl;
	//сохранение конфигов
	configChkList->reset();
	SmeConfigGen smeCfg(smeReg, configChkList);
	AliasConfigGen aliasCfg(aliasReg, configChkList);
	RouteConfigGen routeCfg(routeReg, configChkList);
	smeCfg.saveConfig("../conf/sme.xml");
	aliasCfg.saveConfig("../conf/aliases.xml");
	routeCfg.saveConfig("../conf/routes.xml");
	for (int i = 0; i < numAddr; i++)
	{
		delete addr[i];
		delete smeInfo[i];
	}
	for (int i = 0; i < numAlias; i++)
	{
		delete alias[i];
	}
	return sme;
}

void executeFunctionalTest(const string& smscHost, int smscPort)
{
	vector<TestSme*> sme;
	TestSmeTaskManager tm;
	tm.startTimer();
	//обработка команд консоли
	bool help = true;
	string cmdLine;
	while(help || getline(cin, cmdLine))
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
			cout << "quit [checklist] - stop test and quit, optionaly save checklist" << endl;
			continue;
		}
		//обработка команд
		istringstream is(cmdLine);
		string cmd;
		is >> cmd;
		if (cmd == "conf")
		{
			int numAddr = -1, numAlias = -1, numSme = -1;
			is >> numAddr;
			is >> numAlias;
			is >> numSme;
			if (numAddr < 0 || numAlias < 0 || numSme < 0)
			{
				cout << "Required parameters missing" << endl;
			}
			else if (numAddr < numSme)
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
			is >> cmd;
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
			is >> cmd;
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
			is >> cmd;
			if (cmd == "delay")
			{
				int newVal = -1;
				is >> newVal;
				if (newVal < 0)
				{
					cout << "Required parameters missing" << endl;
				}
				else
				{
					SmppFunctionalTest::delay = newVal;
				}
			}
			else
			{
				help = true;
			}
		}
		else if (cmd == "quit")
		{
			is >> cmd;
			if (cmd == "checklist")
			{
				smppChkList->saveHtml();
				configChkList->saveHtml();
				cout << "Checklist saved" << endl;
			}
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
	//ProfileUtil::setupRandomCorrectProfile(defProfile);
	defProfile.codepage = ProfileCharsetOptions::Default;
	defProfile.reportoptions = ProfileReportOptions::ReportNone;
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

