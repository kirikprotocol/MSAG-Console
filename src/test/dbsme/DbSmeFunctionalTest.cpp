#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "test/sme/SmppBaseTestCases.hpp"
#include "test/sme/SmppTransmitterTestCases.hpp"
#include "test/sme/SmppReceiverTestCases.hpp"
#include "test/sme/SmppProtocolTestCases.hpp"
#include "test/sme/SmppProfilerTestCases.hpp"
#include "DbSmeTestCases.hpp"
#include "DbSmeCheckList.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/alias/AliasManagerTestCases.hpp"
#include "test/router/RouteManagerTestCases.hpp"
#include "test/core/ProfileUtil.hpp"
#include "test/config/SmeConfigGen.hpp"
#include "test/config/AliasConfigGen.hpp"
#include "test/config/RouteConfigGen.hpp"
#include "test/config/ConfigUtil.hpp"
#include "test/util/TestTaskManager.hpp"
#include "test/util/TextUtil.hpp"
#include "util/debug.h"
#include <vector>
#include <sstream>
#include <iostream>

using smsc::sme::SmeConfig;
using smsc::smeman::SmeInfo;
using smsc::alias::AliasInfo;
using smsc::profiler::Profile;
using smsc::test::sms::operator<<;
using smsc::test::smeman::SmeManagerTestCases;
using smsc::test::alias::AliasManagerTestCases;
using smsc::test::router::RouteManagerTestCases;
using smsc::test::conf::TestConfig;
using namespace std;
using namespace smsc::sms;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads; //ThreadPool, ThreadedTask
using namespace smsc::test::sme;
using namespace smsc::test::dbsme;
using namespace smsc::test::config;
using namespace smsc::test::util;
using namespace smsc::test::core;
using namespace smsc::test::sms;

SmeRegistry* smeReg;
AliasRegistry* aliasReg;
RouteRegistry* routeReg;
Profile defProfile;
ProfileRegistry* profileReg;
DbSmeRegistry* dbSmeReg;
CheckList* chkList;

/**
 * “естова€ sme.
 */
class TestSme : public TestTask, SmppResponseSender
{
	int smeNum;
	SmppFixture* fixture;
	SmppSession session;
	SmppBaseTestCases baseTc;
	SmppTransmitterTestCases transmitterTc;
	SmppReceiverTestCases receiverTc;
	SmppProtocolTestCases protocolTc;
	SmppProfilerTestCases profilerTc;
	DbSmeTestCases dbSmeTc;
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
	SmeAcknowledgementHandler* getProfilerAckHandler() { return &profilerTc; }
	SmeAcknowledgementHandler* getDbSmeAckHandler() { return &dbSmeTc; }

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
class DbSmeFunctionalTest
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
	baseTc(config, fixture), receiverTc(fixture), transmitterTc(fixture),
	session(config, &receiverTc), protocolTc(fixture), profilerTc(fixture),
	dbSmeTc(fixture, dbSmeReg), boundOk(false), idx(0)
{
	fixture->session = &session;
	fixture->respSender = this;
}

void TestSme::executeCycle()
{
	//ѕроверка неполученых подтверждений доставки, нотификаций и sms от других sme
	if (time(NULL) > nextCheckTime)
	{
		baseTc.checkMissingPdu();
		nextCheckTime = time(NULL) + 10;
	}
	//проверить тест остановлен/замедлен
	//__trace2__("TestSme::executeCycle(): DbSmeFunctionalTest::pause = %d", DbSmeFunctionalTest::pause);
	if (DbSmeFunctionalTest::pause)
	{
		evt.Wait(1000);
		__trace__("TestSme paused. Returned.");
		return;
	}
	//__trace__("TestSme active. Continued.");
	if (DbSmeFunctionalTest::delay)
	{
		evt.Wait(DbSmeFunctionalTest::delay);
	}
	//debug("*** start ***");
	//Bind sme зарегистрированной в smsc
	//Bind sme с неправильными параметрами
	if (!boundOk)
	{
		boundOk = baseTc.bindCorrectSme(RAND_TC);
		if (!boundOk)
		{
			cout << "Bound failed" << endl;
			exit(-1);
		}
		//установить профиль
		profilerTc.updateCodePageCorrect(rand0(1), getDataCoding(RAND_TC), RAND_TC);
		//подготовить последовательност команд
		seq.insert(seq.end(), 5, 1); //format
		seq.insert(seq.end(), 5, 2); //insert
		seq.push_back(4); //select
		seq.push_back(3); //update
		seq.push_back(4); //select
		seq.push_back(5); //delete (все записи)
		seq.push_back(4); //select (пуста€ таблица)
		//зависнуть
		evt.Wait(5000);
	}
	static int count = 0;
	//каждый 10-ый цикл
	if (count % 10 == 0)
	//if (false)
	{
		count++;
		dbSmeTc.submitIncorrectDateFormatDbSmeCmd(rand0(1),
			getDataCoding(RAND_TC), ALL_TC);
		dbSmeTc.submitIncorrectNumberFormatDbSmeCmd(rand0(1),
			getDataCoding(RAND_TC), ALL_TC);
		dbSmeTc.submitIncorrectParamsDbSmeCmd(rand0(1),
			getDataCoding(RAND_TC), ALL_TC);
	}
	idx = idx < seq.size() ? idx : 0;
	switch (seq[idx++])
	{
		case 1:
			dbSmeTc.submitCorrectFormatDbSmeCmd(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 2:
			dbSmeTc.submitCorrectInsertDbSmeCmd(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 3:
			dbSmeTc.submitCorrectUpdateDbSmeCmd(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 4:
			dbSmeTc.submitCorrectSelectDbSmeCmd(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			evt.Wait(1000);
			break;
		case 5:
			dbSmeTc.submitCorrectDeleteDbSmeCmd(rand0(1), getDataCoding(RAND_TC));
			break;
		default:
			__unreachable__("Invalid tc");
	}
	updateStat();
}

void TestSme::onStopped()
{
	baseTc.unbind();
	DbSmeFunctionalTest::onStopped(smeNum);
	cout << "TestSme::onStopped(): sme = " << smeNum << endl;
}

uint32_t TestSme::sendDeliverySmResp(PduDeliverySm& pdu)
{
	return protocolTc.sendDeliverySmRespOk(pdu, rand0(1));
}

void TestSme::updateStat()
{
	DbSmeFunctionalTest::updateStat(smeNum);
}

//TestSmeTaskManager
bool TestSmeTaskManager::isStopped() const
{
	return DbSmeFunctionalTest::isStopped();
}

//DbSmeFunctionalTest
int DbSmeFunctionalTest::delay = 1000;
bool DbSmeFunctionalTest::pause = false;
DbSmeFunctionalTest::TaskStatList
	DbSmeFunctionalTest::taskStat =
	DbSmeFunctionalTest::TaskStatList();
Mutex DbSmeFunctionalTest::mutex = Mutex();
	
void DbSmeFunctionalTest::resize(int newSize)
{
	taskStat.clear();
	taskStat.resize(newSize);
}

void DbSmeFunctionalTest::onStopped(int taskNum)
{
	//MutexGuard guard(mutex);
	taskStat[taskNum].stopped = true;
}

bool DbSmeFunctionalTest::isStopped()
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

void DbSmeFunctionalTest::updateStat(int smeNum)
{
	//обновить статистику
	taskStat[smeNum].ops++;
}

void DbSmeFunctionalTest::printStat()
{
	for (int i = 0; i < taskStat.size(); i++)
	{
		cout << "sme_" << i << ": ops = " << taskStat[i].ops << endl;
	}
}

bool checkRoute(const Address& smeAddr, const SmeSystemId& smeId,
	const Address& destAlias)
{
	bool routeOk = false;
	ostringstream os;
	os << "smeAddr = " << str(smeAddr) << ", smeId = " << smeId
		<< ", destAlias = " << destAlias;
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	const RouteHolder* routeHolder1 = routeReg->lookup(smeAddr, destAddr);
	if (routeHolder1)
	{
		const SmeSystemId& smeId1 = routeHolder1->route.smeSystemId;
		bool smeBound1 = smeReg->isSmeBound(smeId1);
		os << ", route to = " << smeId1 << ", bound = " << (smeBound1 ? "yes" : "no");
		const RouteHolder* routeHolder2 = routeReg->lookup(destAddr, smeAddr);
		if (routeHolder2)
		{
			const SmeSystemId& smeId2 = routeHolder2->route.smeSystemId;
			bool smeBound2 = smeReg->isSmeBound(smeId2);
			os << ", back route to = " << smeId2 << ", bound = " << (smeBound2 ? "yes" : "no");
			routeOk = true;
		}
		else
		{
			os << ", no back route";
		}
	}
	else
	{
		os << ", no route";
	}
	__trace2__("%s", os.str().c_str());
	return routeOk;
}

vector<TestSme*> genConfig(int numSme, const string& smscHost, int smscPort)
{
	__cfg_addr__(dbSmeAddr);
	__cfg_addr__(dbSmeAlias);
	__cfg_addr__(dbSmeInvalidAddr);
	__cfg_str__(dbSmeSystemId);
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	__cfg_str__(profilerSystemId);
	__cfg_addr__(smscAddr);
	__cfg_str__(smscSystemId);
	__cfg_str__(mapProxySystemId);
	__cfg_str__(abonentInfoSystemId);

	__trace__("*** Generating config files ***");
	smeReg->clear();
	aliasReg->clear();
	routeReg->clear();
	SmeManagerTestCases tcSme(NULL, smeReg, NULL);
	AliasManagerTestCases tcAlias(NULL, aliasReg, NULL);
	RouteManagerTestCases tcRoute(NULL, routeReg, NULL);
	ConfigUtil cfgUtil(smeReg, aliasReg, routeReg);
	cfgUtil.setupSystemSme();
	
	vector<Address*> addr;
	vector<Address*> alias;
	vector<SmeInfo*> smeInfo;
	addr.reserve(numSme);
	alias.reserve(numSme);
	smeInfo.reserve(numSme);
	//регистраци€ sme
	for (int i = 0; i < numSme; i++)
	{
		addr.push_back(new Address());
		smeInfo.push_back(new SmeInfo());
		smeInfo.back()->wantAlias = rand0(1);
		tcSme.addCorrectSme(addr.back(), smeInfo.back(), RAND_TC);
		__trace2__("register sme: addr = %s, systemId = %s",
			str(*addr.back()).c_str(), smeInfo.back()->systemId.c_str());
	}
	//регистраци€ алиасов
	for (int i = 0; i < numSme; i++)
	{
		alias.push_back(new Address());
		SmsUtil::setupRandomCorrectAddress(alias.back(),
			MAX_ADDRESS_VALUE_LENGTH / 2, MAX_ADDRESS_VALUE_LENGTH);
		smeReg->registerAddress(*alias.back());
		__trace2__("register alias: alias = %s", str(*alias.back()).c_str());
	}
	for (int i = 0; i < numSme; i++)
	{
		AliasInfo aliasInfo;
		aliasInfo.addr = *addr[i];
		aliasInfo.alias = *alias[i];
		aliasInfo.hide = rand0(1);
		tcAlias.addCorrectAliasMatch(&aliasInfo, RAND_TC);
	}
	//регистраци€ db sme
	SmeInfo dbSmeInfo;
	dbSmeInfo.wantAlias = false;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&dbSmeInfo);
	dbSmeInfo.systemId = dbSmeSystemId;
	smeReg->registerSme(dbSmeAddr, dbSmeInfo, false, true);
	smeReg->bindSme(dbSmeInfo.systemId);
	//алиас дл€ db sme
	AliasInfo dbSmeAliasInfo;
	dbSmeAliasInfo.addr = dbSmeAddr;
	dbSmeAliasInfo.alias = dbSmeAlias;
	dbSmeAliasInfo.hide = true;
	aliasReg->putAlias(dbSmeAliasInfo);
	//tcAlias->commit();
	//регистраци€ маршрутов sme <-> db sme
	for (int i = 0; i < numSme; i++)
	{
		//sme -> db sme
		RouteInfo route1;
		route1.source = *addr[i];
		route1.dest = dbSmeAddr;
		route1.smeSystemId = dbSmeSystemId;
		route1.enabling = true;
		tcRoute.addCorrectRouteMatch(&route1, NULL, RAND_TC);
		route1.dest = dbSmeInvalidAddr;
		tcRoute.addCorrectRouteMatch(&route1, NULL, RAND_TC);
		//db sme -> sme
		RouteInfo route2;
		route2.source = dbSmeAddr;
		route2.dest = *addr[i];
		route2.smeSystemId = smeInfo[i]->systemId;
		route2.enabling = true;
		tcRoute.addCorrectRouteMatch(&route2, NULL, RAND_TC);
		route2.source = dbSmeInvalidAddr;
		tcRoute.addCorrectRouteMatch(&route2, NULL, RAND_TC);
	}
	//регистраци€ маршрутов sme <-> profiler
	for (int i = 0; i < numSme; i++)
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
	//регистраци€ маршрутов smsc -> sme (delivery receipts)
	for (int i = 0; i < numSme; i++)
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
		config.sid = smeInfo[i]->systemId;
		config.timeOut = 10;
		//config.password;
		//config.systemType;
		//config.origAddr;
		SmppFixture* fixture = new SmppFixture(smeInfo[i]->systemId, *addr[i],
			NULL, smeReg, aliasReg, routeReg, profileReg, chkList);
		sme.push_back(new TestSme(i, config, fixture));
		fixture->ackHandler[profilerAddr] = sme.back()->getProfilerAckHandler();
		fixture->ackHandler[dbSmeAddr] = sme.back()->getDbSmeAckHandler();
		fixture->ackHandler[dbSmeInvalidAddr] = sme.back()->getDbSmeAckHandler();
		smeReg->bindSme(smeInfo[i]->systemId);
	}
	__trace__("Route table");
	//печать таблицы маршрутов sme<->db sme
	for (int i = 0; i < numSme; i++)
	{
		cfgUtil.checkRoute2(*addr[i], smeInfo[i]->systemId, dbSmeAlias);
	}
	//печать таблицы маршрутов sme<->profiler
	for (int i = 0; i < numSme; i++)
	{
		cfgUtil.checkRoute2(*addr[i], smeInfo[i]->systemId, profilerAlias);
	}
	//печать таблицы маршрутов smsc->sme
	for (int i = 0; i < numSme; i++)
	{
		cfgUtil.checkRoute(smscAddr, smscSystemId, *addr[i]);
	}
	//сохранение конфигов
	SmeConfigGen smeCfg(smeReg, NULL);
	AliasConfigGen aliasCfg(aliasReg, NULL);
	RouteConfigGen routeCfg(routeReg, NULL);
	smeCfg.saveConfig("../conf/sme.xml");
	aliasCfg.saveConfig("../conf/aliases.xml");
	routeCfg.saveConfig("../conf/routes.xml");
	//чистка
	for (int i = 0; i < numSme; i++)
	{
		delete addr[i];
		delete alias[i];
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
			cout << "conf <numSme> - generate config files" << endl;
			cout << "test <start|pause|resume> - pause/resume test execution" << endl;
			cout << "stat - print statistics" << endl;
			cout << "chklist - save checklist" << endl;
			cout << "set delay <msec> - slow down test cycle execution" << endl;
			cout << "quit - stop test and quit" << endl;
		}

		//обработка команд
		cin >> cmd;
		if (cmd == "conf")
		{
			int numSme;
			cin >> numSme;
			sme = genConfig(numSme, smscHost, smscPort);
			cout << "Config generated" << endl;
		}
		else if (cmd == "test")
		{
			cin >> cmd;
			if (cmd == "start")
			{
				DbSmeFunctionalTest::resize(sme.size());
				for (int i = 0; i < sme.size(); i++)
				{
					tm.addTask(sme[i]);
				}
				tm.startTimer();
				cout << "Started " << sme.size() << " sme" << endl;
			}
			else if (cmd == "pause")
			{
				DbSmeFunctionalTest::pause = true;
				cout << "Test paused successfully" << endl;
			}
			else if (cmd == "resume")
			{
				DbSmeFunctionalTest::pause = false;
				cout << "Test resumed successfully" << endl;
			}
			else
			{
				help = true;
			}
			__trace2__("executeFunctionalTest(): DbSmeFunctionalTest::pause = %d", DbSmeFunctionalTest::pause);
		}
		else if (cmd == "stat")
		{
			cout << "Time = " << tm.getExecutionTime() << endl;
			DbSmeFunctionalTest::printStat();
		}
		else if (cmd == "chklist")
		{
			chkList->save();
			chkList->saveHtml();
			cout << "Checklist saved" << endl;
		}
		else if (cmd == "set")
		{
			int newVal;
			cin >> cmd;
			cin >> newVal;
			if (cmd == "delay")
			{
				DbSmeFunctionalTest::delay = newVal;
			}
			else
			{
				help = true;
			}
		}
		else if (cmd == "quit")
		{
			//chkList->save();
			//chkList->saveHtml();
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
		cout << "Usage: DbSmeFunctionalTest [host] [port]" << endl;
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
	dbSmeReg = new DbSmeRegistry();
	chkList = new DbSmeCheckList();
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
	delete dbSmeReg;
	delete chkList;
	return 0;
}

