#include "SmppProtocolErrorTestCases.hpp"
#include "SmppProtocolCheckList.hpp"
#include "smeman/smeinfo.h"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/Event.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/util/Util.hpp"
#include "test/core/SmeRegistry.hpp"
#include "test/core/AliasRegistry.hpp"
#include "test/core/RouteRegistry.hpp"
#include "test/config/SmeConfigGen.hpp"
#include "test/config/AliasConfigGen.hpp"
#include "test/config/RouteConfigGen.hpp"
#include "test/config/ConfigUtil.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"

using smsc::sms::Address;
using smsc::smeman::SmeInfo;
using smsc::sme::SmeConfig;
using smsc::router::RouteInfo;
using smsc::core::threads::ThreadPool;
using smsc::core::threads::ThreadedTask;
using smsc::core::synchronization::Event;
using smsc::test::smeman::SmeManagerTestCases;
using smsc::test::config::ConfigUtil;
using smsc::test::sms::SmsUtil;
using namespace std;
using namespace smsc::test::core;
using namespace smsc::test::config;
using namespace smsc::test::sme;
using namespace smsc::test::util;

class TestSme : public ThreadedTask
{
	const string smscHost;
	const int smscPort;
	const Address smeAddr;
	const SmeInfo smeInfo;
	int count;
	bool paused;
	Event event;
	CheckList* chkList;
public:
	TestSme(const string& smscHost, int smscPort, const Address& smeAddr,
		const SmeInfo& smeInfo, CheckList* chkList);
	int getCount() { return count; }
	
	virtual int Execute();
	virtual const char* taskName() { return "TestSme"; }
	void pause() { paused = true; }
	void resume() { paused = false; event.Signal(); }
	void stop() { isStopping = true; event.Signal(); }
};

TestSme::TestSme(const string& _smscHost, int _smscPort, const Address& _smeAddr,
	const SmeInfo& _smeInfo, CheckList* _chkList)
: smscHost(_smscHost), smscPort(_smscPort), smeAddr(_smeAddr),
	smeInfo(_smeInfo), count(0), paused(false), chkList(_chkList) {}

int TestSme::Execute()
{
	SmeConfig conf;
	conf.host = smscHost;
	conf.port = smscPort;
	conf.sid = smeInfo.systemId;
	conf.timeOut = smeInfo.timeout;
	conf.password = smeInfo.password;
	SmppProtocolErrorTestCases tc(conf, smeAddr, chkList);
	while (!isStopping)
	{
		try
		{
			switch (rand1(8))
			{
				case 1:
					tc.invalidBindScenario(RAND_TC);
					break;
				case 2:
					tc.invalidPduScenario(RAND_TC);
					break;
				case 3:
					tc.equalSequenceNumbersScenario();
					break;
				case 4:
					tc.submitAfterUnbindScenario(RAND_TC);
					break;
				case 5:
					tc.nullPduScenario(RAND_TC);
					break;
				case 6:
					tc.bindUnbindScenario(RAND_TC);
					break;
				case 7:
					tc.invalidBindStatusScenario(RAND_TC);
					break;
				case 8:
					tc.enquireLinkScenario(RAND_TC);
					break;
				default:
					__unreachable__("Invalid scenario num");
			}
			__trace__("***Scenario completed***\n");
			sleep(1); //подождать, чтобы освободился systemId
			count++;
			if (paused)
			{
				event.Wait();
			}
		}
		catch(...)
		{
			__warning2__("exception in sme = %p", this);
		}
	}
	return 0;
}

vector<TestSme*> genConfig(int numSme, const string& smscHost, int smscPort,
	CheckList* chkList)
{
	vector<TestSme*> sme;
	SmeRegistry smeReg;
	AliasRegistry aliasReg;
	RouteRegistry routeReg;
	//системные sme
	ConfigUtil cfgUtil(&smeReg, &aliasReg, &routeReg);
	cfgUtil.setupSystemSme();
	//тестовые sme
	for (int i = 0; i < numSme; i++)
	{
		Address smeAddr;
		SmeInfo smeInfo;
		SmsUtil::setupRandomCorrectAddress(&smeAddr);
		SmeManagerTestCases::setupRandomCorrectSmeInfo(&smeInfo);
		smeReg.registerSme(smeAddr, smeInfo);
		//маршрут на самого себя
		RouteInfo route;
		route.source = smeAddr;
		route.dest = smeAddr;
		route.smeSystemId = smeInfo.systemId;
		route.enabling = true;
		RouteUtil::setupRandomCorrectRouteInfo(&route);
		routeReg.putRoute(route, NULL);
		sme.push_back(new TestSme(smscHost, smscPort, smeAddr, smeInfo, chkList));
	}
	//сохранение конфигов
	SmeConfigGen smeCfg(&smeReg, NULL);
	AliasConfigGen aliasCfg(&aliasReg, NULL);
	RouteConfigGen routeCfg(&routeReg, NULL);
	smeCfg.saveConfig("../conf/sme.xml");
	aliasCfg.saveConfig("../conf/aliases.xml");
	routeCfg.saveConfig("../conf/routes.xml");
	return sme;
}

void executeTest(const string& smscHost, int smscPort, CheckList* chkList)
{
	vector<TestSme*> sme;
	ThreadPool threadPool;
	//обработка команд консоли
	bool help = true;
	string cmdLine;
	while(help || getline(cin, cmdLine))
	{
		//хелп
		if (help)
		{
			help = false;
			cout << "conf <numSme> - generate config files" << endl;
			cout << "test <start|pause|resume> - start/pause/resume test execution" << endl;
			cout << "stat - print statistics" << endl;
			cout << "chklist - save checklist" << endl;
			cout << "quit - quit test" << endl;
			continue;
		}
		//обработка команд
		istringstream is(cmdLine);
		string cmd;
		is >> cmd;
		if (cmd == "conf")
		{
			int numSme = -1;
			is >> numSme;
			if (numSme < 0)
			{
				cout << "Required parameters missing" << endl;
				continue;
			}
			else
			{
				sme = genConfig(numSme, smscHost, smscPort, chkList);
				cout << "Config generated" << endl;
				continue;
			}
		}
		else if (cmd == "test")
		{
			is >> cmd;
			if (cmd == "start")
			{
				threadPool.preCreateThreads(sme.size());
				for (int i = 0; i < sme.size(); i++)
				{
					threadPool.startTask(sme[i]);
				}
				cout << "Started " << sme.size() << " sme" << endl;
				continue;
			}
			else if (cmd == "pause")
			{
				for (int i = 0; i < sme.size(); i++)
				{
					sme[i]->pause();
				}
				cout << "Paused " << sme.size() << " sme" << endl;
				continue;
			}
			else if (cmd == "resume")
			{
				for (int i = 0; i < sme.size(); i++)
				{
					sme[i]->resume();
				}
				cout << "Resumed " << sme.size() << " sme" << endl;
				continue;
			}
		}
		else if (cmd == "stat")
		{
			for (int i = 0; i < sme.size(); i++)
			{
				cout << "sme[" << i << "]: " << sme[i]->getCount() << endl;
			}
			continue;
		}
		else if (cmd == "chklist")
		{
			chkList->save();
			chkList->saveHtml();
			cout << "Checklists saved" << endl;
			continue;
		}
		else if (cmd == "quit")
		{
			is >> cmd;
			if (cmd == "checklist")
			{
				chkList->saveHtml();
				chkList->saveHtml();
				cout << "Checklist saved" << endl;
			}
			for (int i = 0; i < sme.size(); i++)
			{
				sme[i]->stop();
			}
			threadPool.shutdown();
			return;
		}
		help = true;
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
	SmppProtocolCheckList chkList;
	executeTest(smscHost, smscPort, &chkList);
}
