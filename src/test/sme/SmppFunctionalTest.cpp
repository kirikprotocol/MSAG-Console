#include "system/smsc.hpp"
#include "system/rescheduler.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include "profiler/profiler.hpp"
#include "admin/util/SignalHandler.h"
#include "store/StoreManager.h"
#include "system/state_machine.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "profiler/profiler.hpp"
#include "system/scheduler.hpp"
#include "test/sme/SmppTestCases.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/alias/AliasManagerTestCases.hpp"
#include "test/router/RouteManagerTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include "test/util/TCResultFilter.hpp"
#include "test/util/CheckList.hpp"
#include "util/debug.h"
//#include <memory>
#include <vector>
#include <sstream>
#include <iostream>

#define __print__(val) cout << #val << " = " << (val) << endl

using smsc::sme::SmeConfig;
using smsc::profiler::Profile;
using smsc::util::config::Manager;
using smsc::system::RescheduleCalculator;
using smsc::test::sms::SmsUtil;
using smsc::test::sms::operator<<;
using smsc::test::smeman::SmeManagerTestCases;
using smsc::test::alias::AliasManagerTestCases;
using smsc::test::router::RouteManagerTestCases;
using namespace std;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads; //ThreadPool, ThreadedTask
using namespace smsc::system;
using namespace smsc::test::sme;
using namespace smsc::test::util;

TCResultFilter* filter = new TCResultFilter();
SmeRegistry* smeReg = new SmeRegistry();
AliasRegistry* aliasReg = new AliasRegistry();
RouteRegistry* routeReg = new RouteRegistry();

/**
 * Тестовая sme.
 */
class TestSme : public TestTask, ResultHandler
{
	int smeNum;
	SmppTestCases tc;
	time_t nextCheckTime;
	bool boundOk;
	Event evt;

public:
	TestSme(int smeNum, const SmeConfig& config, const SmeSystemId& systemId,
		const Address& addr, const SmeRegistry* smeReg,
		const AliasRegistry* aliasReg, const RouteRegistry* routeReg);
	virtual ~TestSme() {}
	virtual void executeCycle();
	virtual void onStopped();

private:
	void process(TCResult* res);
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
	typedef map<const string, int> TCStatMap;
	
	static TaskStatList taskStat;
	static TCStatMap tcStat;
	static Mutex mutex;

public:
	static int delay;
	static bool pause;
	
public:
	static void resize(int newSize);
	static void onStopped(int taskNum);
	static bool isStopped();
	static void process(int taskNum, const TCResult* res);
	static void printOpsStatByTC();
};

/**
 * Подмененный SC.
 */
class TestSmsc : public Smsc
{
public:
	TestSmsc(const string& smscHost, int smscPort);
	~TestSmsc() {}
	vector<TestSme*> init(int numAddr, int numSme);
private:
	vector<TestSme*> config(int numAddr, int numSme);
	void process(TCResult* res);
};

/**
 * Стартовалка для SC.
 */
class SmscStarter : public ThreadedTask
{
	TestSmsc* app;
public:
	SmscStarter(TestSmsc* sc) : app(sc) {}
	virtual const char* taskName() { return "SMSC starter"; }
	virtual int Execute();
};

//TestSme
TestSme::TestSme(int _smeNum, const SmeConfig& config, const SmeSystemId& systemId,
	const Address& smeAlias, const SmeRegistry* smeReg,
	const AliasRegistry* aliasReg, const RouteRegistry* routeReg)
	: TestTask("TestSme", _smeNum), smeNum(_smeNum), nextCheckTime(0),
	tc(config, systemId, smeAlias, smeReg, aliasReg, routeReg, this),
	boundOk(false) {}

void TestSme::executeCycle()
{
	//Проверка неполученых подтверждений доставки, нотификаций и sms от других sme
	if (time(NULL) > nextCheckTime)
	{
		process(tc.checkMissingPdu());
		nextCheckTime = time(NULL) + 5;
	}
	//проверить тест остановлен/замедлен
	__trace2__("TestSme::executeCycle(): SmppFunctionalTest::pause = %d", SmppFunctionalTest::pause);
	if (SmppFunctionalTest::pause)
	{
		evt.Wait(1000);
		__trace__("TestSme paused. Returned.");
		return;
	}
	__trace__("TestSme active. Continued.");
	if (SmppFunctionalTest::delay)
	{
		evt.Wait(SmppFunctionalTest::delay);
	}
	//debug("*** start ***");
	//Bind sme зарегистрированной в smsc
	//Bind sme с неправильными параметрами
	if (!boundOk)
	{
		TCResult* res = tc.bindCorrectSme(RAND_TC);
		for (int i = 0; i < res->getFailures().size(); i++)
		{
			__require__(res->getFailures()[i] != 100); //session->connect() failed
		}
		process(res);
		for (int i = 0; i < 3; i++)
		{
			//process(tc.bindIncorrectSme(RAND_TC)); //обязательно после bindCorrectSme
		}
		boundOk = true;
	}
	//Синхронная отправка submit_sm pdu другим sme
	//Асинхронная отправка submit_sm pdu другим sme
#ifdef ASSERT
	for (TCSelector s(RAND_SET_TC, 3); s.check(); s++)
#else
	for (TCSelector s(RAND_SET_TC, 2); s.check(); s++)
#endif
	{
		switch (s.value())
		{
			case 1:
				process(tc.getTransmitter().submitSmSync(RAND_TC));
				break;
			case 2:
				process(tc.getTransmitter().submitSmAsync(RAND_TC));
				break;
#ifdef ASSERT
			case 3:
				process(tc.getTransmitter().submitSmAssert(RAND_TC));
				break;
#endif
		}
	}
}

inline void TestSme::onStopped()
{
	//Unbind для sme соединенной с smsc
	//Unbind для sme несоединенной с smsc
	process(tc.unbind());
	process(tc.unbind());
	SmppFunctionalTest::onStopped(smeNum);
	cout << "TestSme::onStopped(): sme = " << smeNum << endl;
}

inline void TestSme::process(TCResult* res)
{
	SmppFunctionalTest::process(smeNum, res);
}

//TestSmeTaskManager
bool TestSmeTaskManager::isStopped() const
{
	return SmppFunctionalTest::isStopped();
}

//SmppFunctionalTest
int SmppFunctionalTest::delay = 200;
bool SmppFunctionalTest::pause = false;
SmppFunctionalTest::TaskStatList
	SmppFunctionalTest::taskStat =
	SmppFunctionalTest::TaskStatList();
SmppFunctionalTest::TCStatMap
	SmppFunctionalTest::tcStat =
	SmppFunctionalTest::TCStatMap();
Mutex SmppFunctionalTest::mutex = Mutex();
	
inline void SmppFunctionalTest::resize(int newSize)
{
	taskStat.clear();
	tcStat.clear();
	taskStat.resize(newSize);
}

inline void SmppFunctionalTest::onStopped(int taskNum)
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

void SmppFunctionalTest::process(int smeNum, const TCResult* res)
{
	if (res)
	{
		//обновить статистику
		filter->addResult(res);
		taskStat[smeNum].ops++;
	    tcStat[res->getId()]++;
	    delete res;
	}
}

void SmppFunctionalTest::printOpsStatByTC()
{
	int totalOps = 0;
	for (TCStatMap::iterator it = tcStat.begin(); it != tcStat.end(); it++)
	{
		const string& tcId = it->first;
		int ops = it->second;
		totalOps += ops;
		cout << tcId << ": ops = " << ops << endl;
	}
	cout << "Total ops = " << totalOps << endl;
	cout << "-----------------------------" << endl;
}

//TestSmsc
TestSmsc::TestSmsc(const string& host, int port)
{
	smscHost = host;
	smscPort = port;
}

void TestSmsc::process(TCResult* res)
{
	if (res)
	{
		filter->addResult(res);
		delete res;
	}
}

vector<TestSme*> TestSmsc::config(int numAddr, int numSme)
{
	__require__(numSme <= numAddr);
	SmeManagerTestCases tcSme(&smeman, smeReg);
	AliasManagerTestCases tcAlias(&aliaser, aliasReg);
	RouteManagerTestCases tcRoute(&router, routeReg);

	vector<Address*> addr;
	vector<SmeInfo*> smeInfo;
	addr.reserve(numAddr);
	smeInfo.reserve(numAddr);
	//регистрация sme
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		smeInfo.push_back(new SmeInfo());
		process(tcSme.addCorrectSme(addr[i], smeInfo[i], 1 /*RAND_TC*/));
		ostringstream os;
		os << *addr[i];
		__trace2__("TestSmsc::config(): addr = %s, systemId = %s", os.str().c_str(), smeInfo[i]->systemId.c_str());
	}
	//регистрация алиасов
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
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
						process(tcAlias.addCorrectAliasMatch(&alias, RAND_TC));
						break;
					case 4:
						process(tcAlias.addCorrectAliasNotMatchAddress(&alias, RAND_TC));
						break;
					case 5:
						process(tcAlias.addCorrectAliasNotMatchAlias(&alias, RAND_TC));
						break;
					default:
						__unreachable__("Invalid alias test case");
				}
			}
		}
	}
	tcAlias.commit();
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
						process(tcRoute.addCorrectRouteMatch(
							&route, NULL, RAND_TC));
						break;
					case 4:
						process(tcRoute.addCorrectRouteNotMatch(
							&route, NULL, RAND_TC));
						break;
					case 5:
						process(tcRoute.addCorrectRouteNotMatch2(
							&route, NULL, RAND_TC));
						break;
					default:
						__unreachable__("Invalid route test case");
				}
			}
		}
	}
	tcRoute.commit();
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
		sme.push_back(new TestSme(i, config, smeInfo[i]->systemId, *addr[i],
			smeReg, aliasReg, routeReg)); //throws Exception
		smeReg->bindSme(smeInfo[i]->systemId);
	}
	//печать таблицы маршрутов
	__trace__("Route table");
	bool routesOk = false;
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAlias = *addr[j];
			string smeId = "<>";
			bool smeBound = false;
			const AliasHolder* aliasHolder =
				aliasReg->findAddressByAlias(destAlias);
			Address destAddr;
			if (aliasHolder && aliasHolder->aliasToAddress(destAlias, destAddr))
			{
				const RouteHolder* routeHolder =
					routeReg->lookup(origAddr, destAddr);
				if (routeHolder)
				{
					smeId = routeHolder->route.smeSystemId;
					smeBound = smeReg->isSmeBound(smeId);
					routesOk |= smeBound;
				}
			}
			ostringstream os;
			os << "origAddr = " << origAddr << ", destAlias = " << destAlias;
			__trace2__("%s, route to = %s, sme bound = %s",
				os.str().c_str(), smeId.c_str(), (smeBound ? "yes" : "no"));
		}
	}
	if (!routesOk)
	{
		cout << "Invalid routes generated" << endl;
		abort();
	}
	for (int i = 0; i < numAddr; i++)
	{
		delete addr[i];
		delete smeInfo[i];
	}
	return sme;
}

//TestSmsc
vector<TestSme*> TestSmsc::init(int numAddr, int numSme)
{
	//конфигурация
	Manager::init("config.xml");
	Manager& cfgMan = Manager::getInstance();
	router.assign(&smeman);
	vector<TestSme*> sme = config(numAddr, numSme);

	//инициализация
	tp.preCreateThreads(15);
	
	smsc::store::StoreManager::startup(smsc::util::config::Manager::getInstance());
	store = smsc::store::StoreManager::getMessageStore();

	int cnt = cfgMan.getInt("core.state_machines_count");
    time_t maxValidTime = cfgMan.getInt("sms.max_valid_time");
	for(int i = 0; i < cnt; i++)
	{
      StateMachine *m = new StateMachine(eventqueue, store, this);
      m->maxValidTime = maxValidTime;
      tp.startTask(m);
	}
	RescheduleCalculator::Init(Manager::getInstance().getString("core.reschedule_table"));

	//smsc::admin::util::SignalHandler::registerShutdownHandler(new SmscSignalHandler(this));
	//tp.startTask(new SpeedMonitor(eventqueue));

	smsc::profiler::Profile defProfile;
	defProfile.codepage = 0;
	defProfile.reportoptions = 0;
	tp.startTask(new smsc::profiler::Profiler(defProfile));
	
	return sme;
}

//SmscStarter
int SmscStarter::Execute()
{
	try
	{
		app->run();
	}
	catch(exception& e)
	{
		cout << "Failed to start SC: " << e.what() << endl;
		exit(-1);
	}
	catch(...)
	{
		cout << "Fatal exception in SC!" << endl;
		exit(-0);
	}
	cout << "Stopping ... ";
	delete app;
	cout << "Ok" << endl;
}

void saveCheckList()
{
    cout << "Сохранение checklist" << endl;
    CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
    cl.startNewGroup("Smpp", "smsc::smpp");
    cl.writeResult("Bind sme зарегистрированной в smsc",
        filter->getResults(TC_BIND_CORRECT_SME));
    cl.writeResult("Bind sme с неправильными параметрами",
        filter->getResults(TC_BIND_INCORRECT_SME));
    cl.writeResult("Все подтверждений доставки, нотификации и sms доставляются и не теряются",
        filter->getResults(TC_CHECK_MISSING_PDU));
    cl.writeResult("Unbind для sme",
        filter->getResults(TC_UNBIND));
    cl.writeResult("Синхронная отправка submit_sm pdu другим sme",
        filter->getResults(TC_SUBMIT_SM_SYNC));
    cl.writeResult("Асинхронная отправка submit_sm pdu другим sme",
        filter->getResults(TC_SUBMIT_SM_ASYNC));
    cl.writeResult("Заполнение и отправка submit_sm pdu с недопустимыми значениями полей",
        filter->getResults(TC_SUBMIT_SM_ASSERT));
    cl.writeResult("Получение submit_sm_resp pdu для асинхронного submit_sm реквеста",
        filter->getResults(TC_PROCESS_SUBMIT_SM_RESP));
    cl.writeResult("Получение асинхронного deliver_sm pdu",
        filter->getResults(TC_PROCESS_DELIVERY_SM));
    cl.writeResult("Сообщения правильно доставляются от одного sme другому",
        filter->getResults(TC_PROCESS_NORMAL_SMS));
    cl.writeResult("Подтверждения доставки (delivery receipts) работают правильно",
        filter->getResults(TC_PROCESS_DELIVERY_RECEIPT));
    cl.writeResult("Промежуточные нотификации (intermediate notifications) работают правильно",
        filter->getResults(TC_PROCESS_INTERMEDIATE_NOTIFICATION));
    cl.writeResult("Отсутствие внутренних ошибок в smpp receiver",
        filter->getResults(TC_HANDLE_ERROR));
}

void executeFunctionalTest(int numAddr, int numSme,
	const string& smscHost, int smscPort)
{
	SmppFunctionalTest::resize(numSme);
	//запуск SMSC
	TestSmsc *app = new TestSmsc(smscHost, smscPort);
	vector<TestSme*> sme = app->init(numAddr, numSme);
	ThreadPool pool;
	pool.startTask(new SmscStarter(app));
	sleep(5);
	//запуск sme
	TestSmeTaskManager tm;
	for (int i = 0; i < sme.size(); i++)
	{
		tm.addTask(sme[i]);
	}
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
			cout << "test <pause|resume> - pause/resume test execution" << endl;
			cout << "stat - print statistics" << endl;
			cout << "dump pdu - dump pdu registry" << endl;
			cout << "set delay <msec> - slow down test cycle execution" << endl;
			cout << "quit - stop test and quit" << endl;
		}

		//обработка команд
		cin >> cmd;
		if (cmd == "test")
		{
			cin >> cmd;
			if (cmd == "pause")
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
			SmppFunctionalTest::printOpsStatByTC();
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
			tm.stopTasks();
			saveCheckList();
			app->stop();
			pool.shutdown();
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
	if (argc < 3)
	{
		cout << "Usage: TestSmsc <numAddr> <numSme> [host] [port]" << endl;
		exit(0);
	}
	const int numAddr = atoi(argv[1]);
	const int numSme = atoi(argv[2]);
	string smscHost = "smsc";
	int smscPort = 15975;
	if (argc == 5)
	{
		smscHost = argv[3];
		smscPort = atoi(argv[4]);
	}
	try
	{
		executeFunctionalTest(numAddr, numSme, smscHost, smscPort);
	}
	catch (exception& e)
	{
		cout << "Failed to execute test: " << e.what() << endl;
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}
	delete filter;
	delete smeReg;
	delete aliasReg;
	delete routeReg;
	return 0;
}

