#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "profiler/profiler.hpp"
#include "system/smsc.hpp"
#include "system/rescheduler.hpp"
#include "system/scheduler.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include "system/state_machine.hpp"
#include "admin/util/SignalHandler.h"
#include "store/StoreManager.h"
#include "test/sme/SmppProfilerTestCases.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/alias/AliasManagerTestCases.hpp"
#include "test/router/RouteManagerTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include "SystemSmeCheckList.hpp"
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

SmeRegistry* smeReg = new SmeRegistry();
AliasRegistry* aliasReg = new AliasRegistry();
RouteRegistry* routeReg = new RouteRegistry();
CheckList* chkList = new SmppCheckList();

/**
 * �������� sme.
 */
class TestSme : public TestTask, SmppResponseSender
{
	int smeNum;
	SmppTestCases tc;
	time_t nextCheckTime;
	bool boundOk;
	Event evt;

public:
	TestSme(int smeNum, const SmeConfig& config, const SmeSystemId& systemId,
		const Address& addr, const SmeRegistry* smeReg,
		const AliasRegistry* aliasReg, const RouteRegistry* routeReg,
		CheckList* chkList);
	virtual ~TestSme() {}
	virtual void executeCycle();
	virtual void onStopped();

private:
	virtual bool sendDeliverySmResp(PduDeliverySm& pdu);
	virtual void updateStat();
};

/**
 * ���� ��������.
 */
class TestSmeTaskManager
	: public TestTaskManager<TestSme>
{
public:
	TestSmeTaskManager() {}
	virtual bool isStopped() const;
};

/**
 * ���������� ������ sme.
 */
struct TestSmeStat
{
	int ops;
	bool stopped;
	TestSmeStat() : ops(0), stopped(false) {}
};

/**
 * ���������� ������ ����� �����.
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

/**
 * ����������� SC.
 */
class TestSmsc : public Smsc
{
public:
	TestSmsc(const string& smscHost, int smscPort);
	~TestSmsc() {}
	vector<TestSme*> init(int numAddr, int numAlias, int numSme);
private:
	vector<TestSme*> config(int numAddr, int numAlias, int numSme);
};

/**
 * ����������� ��� SC.
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
	const Address& smeAddr, const SmeRegistry* smeReg,
	const AliasRegistry* aliasReg, const RouteRegistry* routeReg, CheckList* chkList)
	: TestTask("TestSme", _smeNum), smeNum(_smeNum), nextCheckTime(0),
	tc(config, systemId, smeAddr, this, smeReg, aliasReg, routeReg, chkList),
	boundOk(false) {}

void TestSme::executeCycle()
{
	//�������� ����������� ������������� ��������, ����������� � sms �� ������ sme
	if (time(NULL) > nextCheckTime)
	{
		tc.checkMissingPdu();
		nextCheckTime = time(NULL) + 10;
	}
	//��������� ���� ����������/��������
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
	//Bind sme ������������������ � smsc
	//Bind sme � ������������� �����������
	if (!boundOk)
	{
		boundOk = tc.bindCorrectSme(RAND_TC);
		__require__(boundOk);
		for (int i = 0; i < 3; i++)
		{
			tc.bindIncorrectSme(RAND_TC); //����������� ����� bindCorrectSme
		}
		evt.Wait(5000);
	}
	//���������� �������� submit_sm pdu ������ sme
	//����������� �������� submit_sm pdu ������ sme
#ifdef ASSERT
	for (TCSelector s(RAND_TC, 3); s.check(); s++)
#else
	for (TCSelector s(RAND_TC, 2); s.check(); s++)
#endif
	{
		switch (s.value())
		{
			case 1:
				tc.getTransmitter().submitSm(true, RAND_TC);
				break;
			case 2:
				tc.getTransmitter().submitSm(false, RAND_TC);
				break;
#ifdef ASSERT
			case 3:
				tc.getTransmitter().submitSmAssert(RAND_TC);
				break;
#endif
		}
		updateStat();
	}
}

inline void TestSme::onStopped()
{
	tc.unbind(); //Unbind ��� sme ����������� � smsc
	tc.unbind(); //Unbind ��� sme ������������� � smsc
	SmppFunctionalTest::onStopped(smeNum);
	cout << "TestSme::onStopped(): sme = " << smeNum << endl;
}

bool TestSme::sendDeliverySmResp(PduDeliverySm& pdu)
{
	if (rand0(1))
	{
		tc.getTransmitter().sendDeliverySmRespOk(pdu, RAND_TC);
		return true;
	}
	else
	{
		tc.getTransmitter().sendDeliverySmRespErr(pdu, RAND_TC);
		return false;
	}
}

inline void TestSme::updateStat()
{
	SmppFunctionalTest::updateStat(smeNum);
}

//TestSmeTaskManager
bool TestSmeTaskManager::isStopped() const
{
	return SmppFunctionalTest::isStopped();
}

//SmppFunctionalTest
int SmppFunctionalTest::delay = 250;
bool SmppFunctionalTest::pause = false;
SmppFunctionalTest::TaskStatList
	SmppFunctionalTest::taskStat =
	SmppFunctionalTest::TaskStatList();
Mutex SmppFunctionalTest::mutex = Mutex();
	
inline void SmppFunctionalTest::resize(int newSize)
{
	taskStat.clear();
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

void SmppFunctionalTest::updateStat(int smeNum)
{
	//�������� ����������
	taskStat[smeNum].ops++;
}

void SmppFunctionalTest::printStat()
{
	for (int i = 0; i < taskStat.size(); i++)
	{
		cout << "sme_" << i << ": ops = " << taskStat[i].ops << endl;
	}
}

//TestSmsc
TestSmsc::TestSmsc(const string& host, int port)
{
	smscHost = host;
	smscPort = port;
}

vector<TestSme*> TestSmsc::config(int numAddr, int numAlias, int numSme)
{
	__require__(numSme <= numAddr);
	SmeManagerTestCases tcSme(&smeman, smeReg, NULL);
	AliasManagerTestCases tcAlias(&aliaser, aliasReg, NULL);
	RouteManagerTestCases tcRoute(&router, routeReg, NULL);

	vector<Address*> addr;
	vector<SmeInfo*> smeInfo;
	addr.reserve(numAddr);
	smeInfo.reserve(numAddr);
	//����������� sme
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		smeInfo.push_back(new SmeInfo());
		tcSme.addCorrectSme(addr[i], smeInfo[i], 1 /*RAND_TC*/);
		ostringstream os;
		os << *addr[i];
		__trace2__("TestSmsc::config(): addr = %s, systemId = %s", os.str().c_str(), smeInfo[i]->systemId.c_str());
	}
	//����������� �������
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
	tcAlias.commit();
	//����������� ���������
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
	tcRoute.commit();
	//�������� sme
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
			smeReg, aliasReg, routeReg, chkList)); //throws Exception
		smeReg->bindSme(smeInfo[i]->systemId);
	}
	//������ ������� ���������
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
			auto_ptr<const Address> destAddr =
				aliasReg->findAddressByAlias(destAlias);
			if (destAddr.get())
			{
				const RouteHolder* routeHolder =
					routeReg->lookup(origAddr, *destAddr);
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
vector<TestSme*> TestSmsc::init(int numAddr, int numAlias, int numSme)
{
	//������������
	Manager::init("config.xml");
	Manager& cfgMan = Manager::getInstance();
	router.assign(&smeman);
	vector<TestSme*> sme = config(numAddr, numAlias, numSme);

	//�������������
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

void executeFunctionalTest(int numAddr, int numAlias, int numSme,
	const string& smscHost, int smscPort)
{
	SmppFunctionalTest::resize(numSme);
	//������ SMSC
	TestSmsc *app = new TestSmsc(smscHost, smscPort);
	vector<TestSme*> sme = app->init(numAddr, numAlias, numSme);
	ThreadPool pool;
	pool.startTask(new SmscStarter(app));
	sleep(5);
	//������ sme
	TestSmeTaskManager tm;
	for (int i = 0; i < sme.size(); i++)
	{
		tm.addTask(sme[i]);
	}
	tm.startTimer();
	//��������� ������ �������
	string cmd;
	bool help = true;
	while (true)
	{
		//����
		if (help)
		{
			help = false;
			cout << "test <pause|resume> - pause/resume test execution" << endl;
			cout << "stat - print statistics" << endl;
			cout << "chklist - save checklist" << endl;
			cout << "dump pdu - dump pdu registry" << endl;
			cout << "set delay <msec> - slow down test cycle execution" << endl;
			cout << "quit - stop test and quit" << endl;
		}

		//��������� ������
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
			SmppFunctionalTest::printStat();
		}
		else if (cmd == "chklist")
		{
			chkList->save();
			cout << "Checklist saved" << endl;
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
			chkList->save();
			cout << "Checklist saved" << endl;
			tm.stopTasks();
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
	if (argc < 4)
	{
		cout << "Usage: TestSmsc <numAddr> <numAlias> <numSme> [host] [port]" << endl;
		exit(0);
	}
	const int numAddr = atoi(argv[1]);
	const int numAlias = atoi(argv[2]);
	const int numSme = atoi(argv[3]);
	__require__(numAddr >= numAlias && numAddr >= numSme);
	string smscHost = "smsc";
	int smscPort = 15975;
	if (argc == 6)
	{
		smscHost = argv[4];
		smscPort = atoi(argv[5]);
	}
	try
	{
		executeFunctionalTest(numAddr, numAlias, numSme, smscHost, smscPort);
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
	delete chkList;
	return 0;
}

