#include "system/smsc.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include "admin/util/SignalHandler.h"
#include "store/StoreManager.h"
#include "system/state_machine.hpp"
#include "core/synchronization/Event.hpp"
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

using std::vector;
using std::ostringstream;
using smsc::sme::SmeConfig;
using smsc::test::sms::SmsUtil;
using smsc::test::smeman::SmeManagerTestCases;
using smsc::test::alias::AliasManagerTestCases;
using smsc::test::router::RouteManagerTestCases;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads; //ThreadPool, ThreadedTask
using namespace smsc::system;
using namespace smsc::test::sme;
using namespace smsc::test::util;

/**
 * �������� sme.
 */
class TestSme : public TestTask, ResultHandler
{
	int smeNum;
	SmppTestCases tc;
	time_t nextCheckTime;

public:
	TestSme(int smeNum, const SmeConfig& config, const SmeSystemId& systemId,
		const Address& addr, const SmeRegistry* smeReg,
		const AliasRegistry* aliasReg, const RouteRegistry* routeReg);
	virtual ~TestSme();
	virtual void executeCycle();
	virtual void onStopped();

private:
	void process(TCResult* res);
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
	boolean stopped;
	TestSmeStat() : ops(0), stopped(false) {}
};

/**
 * ���������� ������ ����� �����.
 */
class SmppFunctionalTest
{
	typedef vector<TestSmeStat> TaskStatList;
	typedef map<const string, int> TCStatMap;
	
	static TaskStatList taskStat;
	static TCStatMap tcStat;

public:
	static void resize(int newSize);
	static void onStopped(int taskNum);
	static bool isStopped();
	static void process(int taskNum, const TCResult* res);
	static void printOpsStatByTC();
};

/**
 * ����������� SC.
 */
class TestSmsc : public Smsc
{
public:
	vector<TestSme*> init(int numAddr, int numSme);
private:
	vector<TestSme*> config(int numAddr, int numSme);
	void process(TCResult* res);
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
	const AliasRegistry* aliasReg, const RouteRegistry* routeReg)
	: TestTask("TestSme", _smeNum), smeNum(_smeNum), nextCheckTime(0),
	tc(config, systemId, smeAddr, smeReg, aliasReg, routeReg, this)
{
	process(tc.bindNonRegisteredSme(RAND_TC));
	process(tc.bindRegisteredSme(RAND_TC));
	process(tc.getTransmitter().submitSmAssert(RAND_TC));
}
TestSme::~TestSme()
{
	process(tc.unbindBounded());
	process(tc.unbindNonBounded());
}

void TestSme::executeCycle()
{
	//debug("*** start ***");

	//���������� �������� submit_sm pdu ������ sme
	//����������� �������� submit_sm pdu ������ sme
	for (TCSelector s(RAND_SET_TC, 2); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				process(tc.getTransmitter().submitSmSync(RAND_TC));
				break;
			case 2:
				process(tc.getTransmitter().submitSmAsync(RAND_TC));
				break;
		}
	}
	//�������� ����������� ������������� ��������, ����������� � sms �� ������ sme
	if (time(NULL) > nextCheckTime)
	{
		process(tc.processInvalidSms());
		nextCheckTime = time(NULL) + 5;
	}
}

inline void TestSme::onStopped()
{
	SmppFunctionalTest::onStopped(smeNum);
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
SmppFunctionalTest::TaskStatList
	SmppFunctionalTest::taskStat =
	SmppFunctionalTest::TaskStatList();
SmppFunctionalTest::TCStatMap
	SmppFunctionalTest::tcStat =
	SmppFunctionalTest::TCStatMap();
	
inline void SmppFunctionalTest::resize(int newSize)
{
	taskStat.resize(newSize);
}

inline void SmppFunctionalTest::onStopped(int taskNum)
{
	taskStat[taskNum].stopped = true;
}

bool SmppFunctionalTest::isStopped()
{
	bool stopped = true;
	for (int i = 0; stopped && (i < taskStat.size()); i++)
	{
		stopped &= taskStat[i].stopped;
	}
	return stopped;
}

void SmppFunctionalTest::process(int taskNum, const TCResult* res)
{
	if (res)
	{
		//�������� ����������
		taskStat[taskNum].ops++;
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

vector<TestSme*> TestSmsc::config(int numAddr, int numSme)
{
	__require__(numSme <= numAddr);
	SmeRegistry* smeReg = new SmeRegistry();
	AliasRegistry* aliasReg = new AliasRegistry();
	RouteRegistry* routeReg = new RouteRegistry();
	SmeManagerTestCases* tcSme = new SmeManagerTestCases(&smeman);
smsc::alias::AliasManager* aliasMan = new smsc::alias::AliasManager();
	AliasManagerTestCases* tcAlias = new AliasManagerTestCases(aliasMan, aliasReg);
	RouteManagerTestCases* tcRoute = new RouteManagerTestCases(&router, routeReg);

	vector<Address> addr;
	vector<SmeInfo> smeInfo;
	addr.reserve(numAddr);
	smeInfo.reserve(numAddr);
	//����������� sme
	bool emptySystemIdSme = false;
	for (int i = 0; i < numAddr; i++)
	{
		SmsUtil::setupRandomCorrectAddress(&addr[i]);
		if (!emptySystemIdSme)
		{
			emptySystemIdSme = true;
			process(tcSme->addCorrectSmeWithEmptySystemId(&smeInfo[i]));
		}
		else
		{
			process(tcSme->addCorrectSme(&smeInfo[i], RAND_TC));
		}
		if (i < numSme)
		{
			smeReg->registerSme(addr[i]);
		}
		else
		{
			smeReg->registerAddressWithNoSme(addr[i]);
		}
	}
	//����������� �������
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			AliasInfo alias;
			alias.alias = addr[i];
			alias.addr = addr[j];
			for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
			{
				switch (s.value())
				{
					case 1:
					case 2:
						process(tcAlias->addCorrectAliasMatch(&alias, RAND_TC));
						break;
					case 3:
						process(tcAlias->addCorrectAliasNotMatchAddress(&alias, RAND_TC));
						break;
					case 4:
						process(tcAlias->addCorrectAliasNotMatchAlias(&alias, RAND_TC));
						break;
					case 5:
						process(tcAlias->addCorrectAliasException(&alias, RAND_TC));
						break;
				}
			}
		}
	}
	//����������� ���������
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = addr[i];
			Address& destAddr = addr[j];
			TestRouteData route(origAddr, destAddr);
			for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
			{
				switch (s.value())
				{
					case 1:
					case 2:
						process(tcRoute->addCorrectRouteMatch(
							smeInfo[i].systemId, &route, RAND_TC));
						break;
					case 3:
						process(tcRoute->addCorrectRouteNotMatch(
							smeInfo[i].systemId, &route, RAND_TC));
						break;
					case 4:
						process(tcRoute->addCorrectRouteNotMatch2(
							smeInfo[i].systemId, &route, RAND_TC));
						break;
				}
			}
		}
	}
	//�������� sme
	vector<TestSme*> sme;
	for (int i = 0; i < numSme; i++)
	{
		SmeConfig config;
		config.host = "localhost";
		config.port = 45678;
		//config.sid;
		config.timeOut = 10;
		//config.password;
		//config.systemType;
		//config.origAddr;
		sme.push_back(new TestSme(i, config, smeInfo[i].systemId, addr[i],
			smeReg, aliasReg, routeReg)); //throws Exception
	}
	return sme;
}

void TestSmsc::process(TCResult* res)
{
	delete res;
}

//TestSmsc
vector<TestSme*> TestSmsc::init(int numAddr, int numSme)
{
	tp.preCreateThreads(5);
	smsc::util::config::Manager::init("config.xml");
	cfgman=&cfgman->getInstance();

	tp.startTask(
	  new smppio::SmppAcceptor(
		cfgman->getString("smpp.host"),
		cfgman->getInt("smpp.port"),
		&ssockman
	  )
	);
	
	smsc::store::StoreManager::startup(smsc::util::config::Manager::getInstance());
	store=smsc::store::StoreManager::getMessageStore();

	tp.startTask(new StateMachine(eventqueue,store,this));
	tp.startTask(new StateMachine(eventqueue,store,this));
	tp.startTask(new StateMachine(eventqueue,store,this));
	tp.startTask(new StateMachine(eventqueue,store,this));
	tp.startTask(new StateMachine(eventqueue,store,this));

	//smsc::admin::util::SignalHandler::registerShutdownHandler(new SmscSignalHandler(this));

	tp.startTask(new Scheduler(eventqueue,store));

	//tp.startTask(new SpeedMonitor(eventqueue));
	
	return config(numAddr, numSme);
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

void saveCheckList(TCResultFilter* filter)
{
    cout << "���������� checklist" << endl;
    CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
    cl.startNewGroup("Smpp", "smsc::smpp");
    cl.writeResult("Bind ��� sme ������������������ � smsc",
        filter->getResults(TC_BIND_REGISTERED_SME));
    cl.writeResult("Bind ��� sme �������������������� � smsc",
        filter->getResults(TC_BIND_NON_REGISTERED_SME));
    cl.writeResult("��� ������������� ��������, ����������� � sms ������������ � �� ��������",
        filter->getResults(TC_PROCESS_INVALID_SMS));
    cl.writeResult("Unbind ��� sme ����������� � smsc",
        filter->getResults(TC_UNBIND_BOUNDED));
    cl.writeResult("Unbind ��� sme ������������� � smsc",
        filter->getResults(TC_UNBIND_NON_BOUNDED));
    cl.writeResult("���������� �������� submit_sm pdu ������ sme",
        filter->getResults(TC_SUBMIT_SM_SYNC));
    cl.writeResult("����������� �������� submit_sm pdu ������ sme",
        filter->getResults(TC_SUBMIT_SM_ASYNC));
    cl.writeResult("���������� � �������� submit_sm pdu � ������������� ���������� �����",
        filter->getResults(TC_SUBMIT_SM_ASSERT));
    cl.writeResult("��������� submit_sm_resp pdu ��� ������������ submit_sm ��������",
        filter->getResults(TC_PROCESS_SUBMIT_SM_RESP));
    cl.writeResult("��������� ������������ deliver_sm pdu",
        filter->getResults(TC_PROCESS_DELIVERY_SM));
    cl.writeResult("��������� ��������� ������������ �� ������ sme �������",
        filter->getResults(TC_PROCESS_NORMAL_SMS));
    cl.writeResult("������������� �������� (delivery receipts) �������� ���������",
        filter->getResults(TC_PROCESS_DELIVERY_RECEIPT));
    cl.writeResult("������������� ����������� (intermediate notifications) �������� ���������",
        filter->getResults(TC_PROCESS_INTERMEDIATE_NOTIFICATION));
}

void executeFunctionalTest(int numAddr, int numSme)
{
	TCResultFilter* filter = new TCResultFilter();
	//������ SMSC
	TestSmsc *app = new TestSmsc();
	vector<TestSme*> sme = app->init(numAddr, numSme);
	ThreadPool pool;
	pool.startTask(new SmscStarter(app));
	//������ sme
	SmppFunctionalTest::resize(sme.size());
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
			cout << "stat - print statistics" << endl;
			cout << "quit - stop test and quit" << endl;
		}

		//��������� ������
		cin >> cmd;
		if (cmd == "stat")
		{
			cout << "Time = " << tm.getExecutionTime() << endl;
			SmppFunctionalTest::printOpsStatByTC();
		}
		else if (cmd == "quit")
		{
			tm.stopTasks();
			cout << "Total time = " << tm.getExecutionTime() << endl;
			saveCheckList(filter);
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
	if (argc != 3)
	{
		cout << "Usage: TestSmsc <numAddr> <numSme>" << endl;
		exit(0);
	}
	const int numAddr = atoi(argv[1]);
	const int numSme = atoi(argv[2]);
	try
	{
		executeFunctionalTest(numAddr, numSme);
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}
	return 0;
}

