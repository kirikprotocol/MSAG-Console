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
 * �������� sme.
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

//TestSme
TestSme::TestSme(int num, const SmeConfig& config, SmppFixture* fixture)
	: TestTask("TestSme", num), smeNum(num), nextCheckTime(0),
	tc(config, fixture), boundOk(false), idx(0)
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
	//Bind sme ������������������ � smsc
	//Bind sme � ������������� �����������
	if (!boundOk)
	{
		//��������� ����������������� ������
		seq.push_back(1);
		seq.insert(seq.end(), 3, 3);
		seq.insert(seq.end(), 3, 2);
		seq.push_back(1);
		for (int i = 0; i < seq.size(); i++)
		{
			switch (seq[i])
			{
				case 1: //���������� bind
					evt.Wait(2000);
					boundOk = tc.getBase().bindCorrectSme(RAND_TC);
					if (!boundOk)
					{
						cout << "Bound failed" << endl;
						exit(0);
					}
					break;
				case 2: //������������ bind
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
		//�������� ����������������� ������
		seq.clear();
#ifdef SIMPLE_TEST
	seq.push_back(201);
#else
	//seq.insert(seq.end(), 15, 1);
	//seq.insert(seq.end(), 10, 2);
	seq.insert(seq.end(), 10, 3);
	seq.push_back(4);
#ifdef ASSERT
	seq.push_back(101);
#endif //ASSERT
#endif //SIMPLE_TEST
	random_shuffle(seq.begin(), seq.end());
	}
	idx = idx < seq.size() ? idx : 0;
	switch (seq[idx++])
	{
		case 1: //���������� sms
			tc.submitSmCorrect(rand0(1), RAND_TC);
			break;
		case 2: //������������ sms
			tc.submitSmIncorrect(rand0(1), RAND_TC);
			break;
		case 3: //���������� ������� ����������� �������
			tc.updateProfileCorrect(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 4: //���������� ������� ������������� �������
			tc.updateProfileIncorrect(rand0(1), getDataCoding(RAND_TC));
			break;
		case 5:
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
		case 201: //������������� ���������� sms
			tc.submitSmCorrect(rand0(1), 1);
			break;
		default:
			__unreachable__("Invalid seq number");
	}
	updateStat();
}

void TestSme::onStopped()
{
	tc.getBase().unbind(); //Unbind ��� sme ����������� � smsc
	tc.getBase().unbind(); //Unbind ��� sme ������������� � smsc
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
	__cfg_str__(abonentInfoSystemId);
	
	vector<Address*> addr;
	vector<SmeInfo*> smeInfo;
	addr.reserve(numAddr);
	smeInfo.reserve(numAddr);
	//����������� sme
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		smeInfo.push_back(new SmeInfo());
		tcSme.addCorrectSme(addr[i], smeInfo[i], RAND_TC);
		ostringstream os;
		os << *addr[i];
		__trace2__("genConfig(): addr = %s, systemId = %s", os.str().c_str(), smeInfo[i]->systemId.c_str());
	}
	//����������� profiler
	SmeInfo profilerInfo;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&profilerInfo);
	profilerInfo.systemId = profilerSystemId;
	smeReg->registerSme(profilerAddr, profilerInfo, false, true);
	smeReg->bindSme(profilerInfo.systemId);
	//����������� map proxy
	SmeInfo mapProxyInfo;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&mapProxyInfo);
	mapProxyInfo.systemId = mapProxySystemId;
	smeReg->registerSme("+123", mapProxyInfo, false, true);
	smeReg->bindSme(mapProxyInfo.systemId);
	//abonent info ������
	SmeInfo abonentProxyInfo;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&abonentProxyInfo);
	abonentProxyInfo.systemId = abonentInfoSystemId;
	smeReg->registerSme("+321", abonentProxyInfo, false, true);
	smeReg->bindSme(abonentProxyInfo.systemId);
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
	//����� ��� profiler
	AliasInfo profilerAliasInfo;
	profilerAliasInfo.addr = profilerAddr;
	profilerAliasInfo.alias = profilerAlias;
	profilerAliasInfo.hide = true;
	aliasReg->putAlias(profilerAliasInfo);
	//tcAlias->commit();
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
	//�������� �� profiler
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
	//���������� ��������
	configChkList->reset();
	SmeConfigGen smeCfg(smeReg, configChkList);
	AliasConfigGen aliasCfg(aliasReg, configChkList);
	RouteConfigGen routeCfg(routeReg, configChkList);
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
			NULL, NULL, smeReg, aliasReg, routeReg, profileReg, smppChkList);
		sme.push_back(new TestSme(i, config, fixture)); //throws Exception
		smeReg->bindSme(smeInfo[i]->systemId);
	}
	//������ ������� ��������� ����� sme
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
	//������ ������� ��������� ����� sme � �����������
	for (int i = 0; i < numSme; i++)
	{
		ostringstream os;
		os << "sme: addr = " << *addr[i] << ", smeId = " << smeInfo[i]->systemId;
		const Address& _smeAddr = *addr[i];
		static const Address _profilerAddr = aliasReg->findAddressByAlias(profilerAlias);
		const RouteHolder* routeHolder1 = routeReg->lookup(_smeAddr, _profilerAddr);
		if (routeHolder1)
		{
			const SmeSystemId& smeId1 = routeHolder1->route.smeSystemId;
			bool smeBound1 = smeReg->isSmeBound(smeId1);
			os << ", route to = " << smeId1 << ", bound = " << (smeBound1 ? "yes" : "no");
			const RouteHolder* routeHolder2 = routeReg->lookup(_profilerAddr, _smeAddr);
			if (routeHolder2)
			{
				const SmeSystemId& smeId2 = routeHolder2->route.smeSystemId;
				bool smeBound2 = smeReg->isSmeBound(smeId2);
				os << ", back route to = " << smeId2 << ", bound = " << (smeBound2 ? "yes" : "no");
				//routeCount++;
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
	//��������� ������ �������
	bool help = true;
	string cmdLine;
	while(help || getline(cin, cmdLine))
	{
		//����
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
		//��������� ������
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

