#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "DbSmeTestCases.hpp"
#include "DbSmeCheckList.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/alias/AliasManagerTestCases.hpp"
#include "test/router/RouteManagerTestCases.hpp"
#include "test/profiler/ProfilerTestCases.hpp"
#include "test/core/ProfileUtil.hpp"
#include "test/config/SmeConfigGen.hpp"
#include "test/config/AliasConfigGen.hpp"
#include "test/config/RouteConfigGen.hpp"
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
using smsc::test::profiler::ProfilerTestCases;
using smsc::test::sme::SmppResponseSender;
using smsc::test::conf::TestConfig;
using namespace std;
using namespace smsc::sms;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads; //ThreadPool, ThreadedTask
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
 * �������� sme.
 */
class TestSme : public TestTask, SmppResponseSender
{
	int smeNum;
	DbSmeTestCases tc;
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
	tc(config, fixture, dbSmeReg), boundOk(false), idx(0)
{
	fixture->respSender = this;
}

void TestSme::executeCycle()
{
	//�������� ����������� ������������� ��������, ����������� � sms �� ������ sme
	if (time(NULL) > nextCheckTime)
	{
		tc.getBase().checkMissingPdu();
		nextCheckTime = time(NULL) + 10;
	}
	//��������� ���� ����������/��������
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
	//Bind sme ������������������ � smsc
	//Bind sme � ������������� �����������
	if (!boundOk)
	{
		boundOk = tc.getBase().bindCorrectSme(RAND_TC);
		if (!boundOk)
		{
			cout << "Bound failed" << endl;
			exit(-1);
		}
		//����������� ����������������� ������
		seq.insert(seq.end(), 5, 1); //format
		seq.insert(seq.end(), 5, 2); //insert
		seq.push_back(4); //select
		seq.push_back(3); //update
		seq.push_back(4); //select
		seq.push_back(5); //delete (��� ������)
		seq.push_back(4); //select (������ �������)
		//���������
		evt.Wait(5000);
	}
	static int count = 0;
	//������ 10-�� ����
	if (count % 10 == 0)
	{
		count++;
		tc.submitIncorrectDateFormatDbSmeCmd(rand0(1),
			getDataCoding(RAND_TC), ALL_TC);
		tc.submitIncorrectNumberFormatDbSmeCmd(rand0(1),
			getDataCoding(RAND_TC), ALL_TC);
		tc.submitIncorrectParamsDbSmeCmd(rand0(1),
			getDataCoding(RAND_TC), ALL_TC);
	}
	idx = idx < seq.size() ? idx : 0;
	switch (seq[idx++])
	{
		case 1:
			tc.submitCorrectFormatDbSmeCmd(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 2:
			tc.submitCorrectInsertDbSmeCmd(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 3:
			tc.submitCorrectUpdateDbSmeCmd(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 4:
			tc.submitCorrectSelectDbSmeCmd(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			evt.Wait(1000);
			break;
		case 5:
			tc.submitCorrectDeleteDbSmeCmd(rand0(1), getDataCoding(RAND_TC));
			break;
		default:
			__unreachable__("Invalid tc");
	}
	updateStat();
}

void TestSme::onStopped()
{
	tc.getBase().unbind();
	DbSmeFunctionalTest::onStopped(smeNum);
	cout << "TestSme::onStopped(): sme = " << smeNum << endl;
}

uint32_t TestSme::sendDeliverySmResp(PduDeliverySm& pdu)
{
	return tc.sendDeliverySmRespOk(pdu, rand0(1));
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
int DbSmeFunctionalTest::delay = 500;
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
	//�������� ����������
	taskStat[smeNum].ops++;
}

void DbSmeFunctionalTest::printStat()
{
	for (int i = 0; i < taskStat.size(); i++)
	{
		cout << "sme_" << i << ": ops = " << taskStat[i].ops << endl;
	}
}

vector<TestSme*> genConfig(int numSme, const string& smscHost, int smscPort)
{
	__trace__("*** Generating config files ***");
	smeReg->clear();
	aliasReg->clear();
	routeReg->clear();
	SmeManagerTestCases tcSme(NULL, smeReg, NULL);
	AliasManagerTestCases tcAlias(NULL, aliasReg, NULL);
	RouteManagerTestCases tcRoute(NULL, routeReg, NULL);
	
	__cfg_addr__(dbSmeAddr);
	__cfg_addr__(dbSmeAlias);
	__cfg_str__(dbSmeSystemId);
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	__cfg_str__(profilerSystemId);

	vector<Address*> addr;
	vector<SmeInfo*> smeInfo;
	addr.reserve(numSme);
	smeInfo.reserve(numSme);
	//����������� sme
	for (int i = 0; i < numSme; i++)
	{
		addr.push_back(new Address());
		smeInfo.push_back(new SmeInfo());
		tcSme.addCorrectSme(addr[i], smeInfo[i], RAND_TC);
		ostringstream os;
		os << *addr[i];
	}
	//����������� db sme
	SmeInfo dbSmeInfo;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&dbSmeInfo);
	dbSmeInfo.systemId = dbSmeSystemId;
	smeReg->registerSme(dbSmeAddr, dbSmeInfo, false, true);
	smeReg->bindSme(dbSmeInfo.systemId);
	//����������� profiler
	SmeInfo profilerInfo;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&profilerInfo);
	profilerInfo.systemId = profilerSystemId;
	smeReg->registerSme(profilerAddr, profilerInfo, false, true);
	smeReg->bindSme(profilerInfo.systemId);
	//����������� ������� (����� ������� �����)
	for (int i = 0; i < numSme; i++)
	{
		for (int j = 0; j < numSme; j++)
		{
			AliasInfo alias;
			alias.alias = *addr[i];
			alias.addr = *addr[j];
			tcAlias.addCorrectAliasMatch(&alias, RAND_TC);
		}
	}
	//����� ��� db sme
	AliasInfo dbSmeAliasInfo;
	dbSmeAliasInfo.addr = dbSmeAddr;
	dbSmeAliasInfo.alias = dbSmeAlias;
	dbSmeAliasInfo.hide = true;
	aliasReg->putAlias(dbSmeAliasInfo);
	//����� ��� profiler
	AliasInfo profilerAliasInfo;
	profilerAliasInfo.addr = profilerAddr;
	profilerAliasInfo.alias = profilerAlias;
	profilerAliasInfo.hide = true;
	aliasReg->putAlias(profilerAliasInfo);
	//tcAlias->commit();
	//����������� ��������� (�� ������ sme �� db sme � �������)
	for (int i = 0; i < numSme; i++)
	{
		//sme -> db sme
		RouteInfo route1;
		route1.source = *addr[i];
		route1.dest = dbSmeAddr;
		route1.smeSystemId = dbSmeSystemId;
		route1.enabling = true;
		tcRoute.addCorrectRouteMatch(&route1, NULL, RAND_TC);
		//db sme -> sme
		RouteInfo route2;
		route2.source = dbSmeAddr;
		route2.dest = *addr[i];
		route2.smeSystemId = smeInfo[i]->systemId;
		route2.enabling = true;
		tcRoute.addCorrectRouteMatch(&route2, NULL, RAND_TC);
	}
	//���������� ��������
	SmeConfigGen smeCfg(smeReg, NULL);
	AliasConfigGen aliasCfg(aliasReg, NULL);
	RouteConfigGen routeCfg(routeReg, NULL);
	smeCfg.saveConfig("../conf/sme.xml");
	aliasCfg.saveConfig("../conf/aliases.xml");
	routeCfg.saveConfig("../conf/routes.xml");
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
		SmppFixture* fixture = new SmppFixture(smeInfo[i]->systemId, *addr[i],
			NULL, NULL, smeReg, aliasReg, routeReg, profileReg, chkList);
		sme.push_back(new TestSme(i, config, fixture));
		smeReg->bindSme(smeInfo[i]->systemId);
	}
	//������ ������� ���������
	__trace__("Route table");
	for (int i = 0; i < numSme; i++)
	{
		ostringstream os;
		os << "sme: addr = " << str(*addr[i]) << ", smeId = " << smeInfo[i]->systemId;
		const Address& _smeAddr = *addr[i];
		static const Address _dbSmeAddr = aliasReg->findAddressByAlias(dbSmeAlias);
		const RouteHolder* routeHolder1 = routeReg->lookup(_smeAddr, _dbSmeAddr);
		if (routeHolder1)
		{
			const SmeSystemId& smeId1 = routeHolder1->route.smeSystemId;
			bool smeBound1 = smeReg->isSmeBound(smeId1);
			os << ", route to = " << smeId1 << ", bound = " << (smeBound1 ? "yes" : "no");
			const RouteHolder* routeHolder2 = routeReg->lookup(_dbSmeAddr, _smeAddr);
			if (routeHolder2)
			{
				const SmeSystemId& smeId2 = routeHolder2->route.smeSystemId;
				bool smeBound2 = smeReg->isSmeBound(smeId2);
				os << ", back route to = " << smeId2 << ", bound = " << (smeBound2 ? "yes" : "no");
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
	}
	for (int i = 0; i < numSme; i++)
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
	//��������� ������ �������
	string cmd;
	bool help = true;
	while (true)
	{
		//����
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

		//��������� ������
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

