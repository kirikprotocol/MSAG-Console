#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "SmppBaseTestCases.hpp"
#include "SmppPduChecker.hpp"
#include "SmppTransmitterTestCases.hpp"
#include "SmppReceiverTestCases.hpp"
#include "SmscSmeTestCases.hpp"
#include "SmppProtocolTestCases.hpp"
#include "SmppProfilerTestCases.hpp"
#include "AbonentInfoTestCases.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/alias/AliasManagerTestCases.hpp"
#include "test/router/RouteManagerTestCases.hpp"
#include "test/core/ProfileUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/config/SmeConfigGen.hpp"
#include "test/config/AliasConfigGen.hpp"
#include "test/config/RouteConfigGen.hpp"
#include "test/config/ConfigUtil.hpp"
#include "test/util/TestTaskManager.hpp"
#include "test/util/TextUtil.hpp"
#include "SystemSmeCheckList.hpp"
#include "test/config/ConfigGenCheckList.hpp"
#include "util/debug.h"
#include <vector>
#include <sstream>
#include <iostream>

//#define LOAD_TEST

using smsc::sme::SmeConfig;
using smsc::smeman::SmeInfo;
using smsc::alias::AliasInfo;
using smsc::test::smpp::SmppUtil;
using smsc::test::sms::operator<<;
using smsc::test::sms::operator==;
using smsc::test::smeman::SmeManagerTestCases;
using smsc::test::alias::AliasManagerTestCases;
using smsc::test::router::RouteManagerTestCases;
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
SmppPduSender* pduSender;

/**
 * “естова€ sme.
 */
class TestSme : public TestTask, SmppResponseSender
{
	int smeNum;
	SmppFixture* fixture;
	SmppSession* session; //удал€етс€ в fixture
	SmppPduChecker pduChecker;
	SmppBaseTestCases baseTc;
	SmppTransmitterTestCases transmitterTc;
	SmppReceiverTestCases receiverTc;
	SmscSmeTestCases smscSmeTc;
	SmppProtocolTestCases protocolTc;
	SmppProfilerTestCases profilerTc;
	AbonentInfoTestCases abonentInfoTc;
	time_t nextCheckTime;
	bool boundOk;
	Event evt;
	int idx;
	vector<int> seq;

public:
	TestSme(int smeNum, const SmeConfig& config, SmppFixture* fixture);
	virtual ~TestSme() { delete fixture; }
	virtual void executeCycle();
	virtual void onStopped();
	PduHandler* getDeliveryReceiptHandler() { return &smscSmeTc; }
	PduHandler* getProfilerAckHandler() { return &profilerTc; }
	PduHandler* getAbonentInfoAckHandler() { return &abonentInfoTc; }

private:
	virtual pair<uint32_t, time_t> sendDeliverySmResp(PduDeliverySm& pdu);
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
TestSme::TestSme(int num, const SmeConfig& config, SmppFixture* _fixture)
	: TestTask("TestSme", num), smeNum(num), nextCheckTime(0),
	fixture(_fixture), pduChecker(_fixture), baseTc(config, _fixture),
	receiverTc(_fixture), transmitterTc(_fixture), smscSmeTc(_fixture),
	protocolTc(_fixture), profilerTc(_fixture), abonentInfoTc(_fixture),
	boundOk(false), idx(0)
{
	session = new SmppSession(config, &receiverTc);
	fixture->session = session;
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
					boundOk = baseTc.bindCorrectSme();
					if (!boundOk)
					{
						cout << "Bound failed" << endl;
						exit(0);
					}
					break;
				case 2: //неправильный bind
					baseTc.bindIncorrectSme(RAND_TC);
					break;
				case 3: //unbind
					baseTc.unbind();
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
#ifdef LOAD_TEST
		seq.push_back(201);
#else
		seq.insert(seq.end(), 20, 1);
		seq.insert(seq.end(), 10, 2);
		seq.push_back(51);
		seq.push_back(52);
		seq.push_back(53);
		seq.push_back(61);
		seq.push_back(62);
		seq.push_back(71);
		//seq.push_back(100);
#ifdef ASSERT
		seq.push_back(101);
#endif //ASSERT
#endif //LOAD_TEST
		random_shuffle(seq.begin(), seq.end());
	}
	idx = idx < seq.size() ? idx : 0;
	switch (seq[idx++])
	{
		//smsc
		case 1: //правильна€ sms
			protocolTc.submitSmCorrect(rand0(1), RAND_TC);
			break;
		case 2: //неправильна€ sms
			protocolTc.submitSmIncorrect(rand0(1), RAND_TC);
			break;
		case 3:
			protocolTc.replaceSmCorrect(rand0(1), RAND_TC);
			break;
		case 4:
			protocolTc.replaceSmIncorrect(rand0(1), RAND_TC);
			break;
		//profiler
		case 51: //обновление настроек кодировки
			profilerTc.updateCodePageCorrect(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 52: //обновление настроек уведомлени€ о доствке
			profilerTc.updateReportOptionsCorrect(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 53: //обновление профил€ некорректными данными
			profilerTc.updateProfileIncorrect(rand0(1), getDataCoding(RAND_TC));
			break;
		//abonent info
		case 61: //корректный запрос на abonent info
			abonentInfoTc.queryAbonentInfoCorrect(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		case 62: //некорректный запрос на abonent info
			abonentInfoTc.queryAbonentInfoIncorrect(rand0(1), getDataCoding(RAND_TC), RAND_TC);
			break;
		//smsc sme
		case 71: //отправка pdu smsc sme
			smscSmeTc.submitSm(rand0(1));
			break;
		case 100:
			protocolTc.sendInvalidPdu(rand0(1), RAND_TC);
			break;
		case 101: //asserts
			protocolTc.submitSmAssert(RAND_TC);
			break;
		/*
		case 102:
			tc.replaceSmAssert(RAND_TC);
			break;
		*/
		case 201: //исключительно правильна€ sms
			protocolTc.submitSmCorrect(rand0(1), 1);
			break;
		default:
			__unreachable__("Invalid seq number");
	}
	updateStat();
}

void TestSme::onStopped()
{
	baseTc.unbind(); //Unbind дл€ sme соединенной с smsc
	baseTc.unbind(); //Unbind дл€ sme несоединенной с smsc
	SmppFunctionalTest::onStopped(smeNum);
	cout << "TestSme::onStopped(): sme = " << smeNum << endl;
}

pair<uint32_t, time_t> TestSme::sendDeliverySmResp(PduDeliverySm& pdu)
{
	//на delivery receipt и сообщение от профайлера ответить ok
	Address addr;
	SmppUtil::convert(pdu.get_message().get_source(), &addr);
	if (fixture->pduHandler.count(addr))
	{
		return protocolTc.sendDeliverySmRespOk(pdu, rand0(1), false);
	}
	//на остальное ответить как придетс€
#ifdef LOAD_TEST
	return protocolTc.sendDeliverySmRespOk(pdu, rand0(1), false);
#else
	switch (rand1(3))
	{
		case 1:
			return protocolTc.sendDeliverySmRespError(pdu, rand0(1), false, RAND_TC);
		case 2:
			return protocolTc.sendDeliverySmRespRetry(pdu, rand0(1), RAND_TC);
		default:
			return protocolTc.sendDeliverySmRespOk(pdu, rand0(1), false);
	}
#endif //LOAD_TEST
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

vector<TestSme*> genConfig(int transceivers, int transmitters,
	int receivers, int notConnected, const string& smscHost, int smscPort)
{
	__require__(transceivers > 0 && transmitters > 0 &&
		receivers > 0 && notConnected > 0);
	__cfg_addr__(smscAddr);
	__cfg_addr__(smscAlias);
	__cfg_str__(smscSystemId);
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	__cfg_str__(profilerSystemId);
	__cfg_addr__(abonentInfoAddr);
	__cfg_addr__(abonentInfoAlias);
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

	int numSme = transceivers + transmitters + receivers;
	int numAddr = numSme + notConnected;
	vector<Address*> addr;
	vector<Address*> alias;
	vector<SmeInfo*> smeInfo;
	addr.reserve(numAddr);
	alias.reserve(numAddr);
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
	for (int i = 0; i < numAddr; i++)
	{
		alias.push_back(new Address());
		SmsUtil::setupRandomCorrectAddress(alias.back(),
			MAX_ADDRESS_VALUE_LENGTH / 2, MAX_ADDRESS_VALUE_LENGTH);
		smeReg->registerAddress(*alias.back());
		__trace2__("register alias: alias = %s", str(*alias.back()).c_str());
	}
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			for (TCSelector s(RAND_SET_TC, 3); s.check(); s++)
			{
				AliasInfo aliasInfo;
				aliasInfo.addr = *addr[i];
				aliasInfo.alias = *alias[j];
				switch (s.value())
				{
					case 1:
						tcAlias.addCorrectAliasMatch(&aliasInfo, RAND_TC);
						break;
					case 2:
						tcAlias.addCorrectAliasNotMatchAddress(&aliasInfo, RAND_TC);
						break;
					case 3:
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
		if (!cfgUtil.checkAlias(*addr[i]))
		{
			__unreachable__("Alias constraint d(a(A)) = A violated");
		}
	}
	//tcAlias->commit();
	//регистраци€ маршрутов sme <-> sme
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			for (TCSelector s(RAND_SET_TC, 3); s.check(); s++)
			{
				RouteInfo route;
				route.source = *addr[i];
				route.dest = *addr[j];
				route.smeSystemId = smeInfo[rand0(numAddr - 1)]->systemId;
				route.enabling = rand0(2);
				switch (s.value())
				{
					case 1:
						tcRoute.addCorrectRouteMatch(&route, NULL, RAND_TC);
						break;
					case 2:
						tcRoute.addCorrectRouteNotMatch(&route, NULL, RAND_TC);
						break;
					case 3:
						tcRoute.addCorrectRouteNotMatch2(&route, NULL, RAND_TC);
						break;
					default:
						__unreachable__("Invalid route test case");
				}
			}
		}
	}
	//регистраци€ маршрутов:
	//	sme <-> profiler
	//	sme <-> abonent info
	//	sme <-> smsc sme (delivery receipts)
	for (int i = 0; i < numAddr; i++)
	{
		cfgUtil.setupDuplexRoutes(*addr[i], smeInfo[i]->systemId,
			profilerAddr, profilerSystemId);
		cfgUtil.setupDuplexRoutes(*addr[i], smeInfo[i]->systemId,
			abonentInfoAddr, abonentInfoSystemId);
		cfgUtil.setupDuplexRoutes(*addr[i], smeInfo[i]->systemId,
			smscAddr, smscSystemId);
	}
	//маршруты между системными sme
	cfgUtil.setupSystemSmeRoutes();
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
		config.password = smeInfo[i]->password;
		//config.systemType;
		//config.origAddr;
		SmeType smeType = i < transceivers ? SME_TRANSCEIVER :
			(i < transceivers + transmitters ? SME_TRANSMITTER : SME_RECEIVER);
		SmppFixture* fixture = new SmppFixture(smeType, *smeInfo[i], *addr[i],
			NULL, smeReg, aliasReg, routeReg, profileReg, smppChkList);
		sme.push_back(new TestSme(i, config, fixture)); //throws Exception
		fixture->pduHandler[smscAddr] = sme.back()->getDeliveryReceiptHandler();
		fixture->pduHandler[smscAlias] = sme.back()->getDeliveryReceiptHandler();
		fixture->pduHandler[profilerAddr] = sme.back()->getProfilerAckHandler();
		fixture->pduHandler[profilerAlias] = sme.back()->getProfilerAckHandler();
		fixture->pduHandler[abonentInfoAddr] = sme.back()->getAbonentInfoAckHandler();
		fixture->pduHandler[abonentInfoAlias] = sme.back()->getAbonentInfoAckHandler();
		fixture->pduSender = pduSender;
		smeReg->bindSme(smeInfo[i]->systemId, smeType);
	}
	__trace__("*** Route table ***");
	//печать таблицы маршрутов sme->sme
	int numRoutes = 0;
	int numBound = 0;
	for (int i = 0; i < numAddr; i++)
	{
		const vector<const Address*>& addrList = smeReg->getAddressList();
		for (int j = 0; j < addrList.size(); j++)
		{
			cfgUtil.checkRoute(*addr[i], smeInfo[i]->systemId,
				*addrList[j], &numRoutes, &numBound);
		}
	}
	//печать таблицы маршрутов sme<->profiler
	for (int i = 0; i < numSme; i++)
	{
		cfgUtil.checkRoute2(*addr[i], smeInfo[i]->systemId, profilerAlias);
	}
	//печать таблицы маршрутов sme<->abonent info
	for (int i = 0; i < numSme; i++)
	{
		cfgUtil.checkRoute2(*addr[i], smeInfo[i]->systemId, abonentInfoAlias);
	}
	//печать таблицы маршрутов smsc->sme
	for (int i = 0; i < numSme; i++)
	{
		cfgUtil.checkRoute(smscAddr, smscSystemId, *addr[i]);
	}
	//печать таблицы маршрутов sme->smsc
	for (int i = 0; i < numSme; i++)
	{
		cfgUtil.checkRoute(*addr[i], smeInfo[i]->systemId, smscAlias);
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
	//чистка
	for (int i = 0; i < numAddr; i++)
	{
		delete addr[i];
		delete smeInfo[i];
		delete alias[i];
	}
	return sme;
}

void printRouteArcBillInfo(const char* fileName)
{
	__cfg_addr__(smscAddr);
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	__cfg_addr__(abonentInfoAddr);
	__cfg_addr__(abonentInfoAlias);
	__cfg_addr__(dbSmeAddr);
	__cfg_addr__(dbSmeAlias);
	__cfg_addr__(dbSmeInvalidAddr);

	ofstream report(fileName);
	vector<const Address*> addrList = smeReg->getAddressList();
	addrList.push_back(&smscAddr);
	addrList.push_back(&profilerAddr);
	addrList.push_back(&profilerAlias);
	addrList.push_back(&abonentInfoAddr);
	addrList.push_back(&abonentInfoAlias);
	addrList.push_back(&dbSmeAddr);
	addrList.push_back(&dbSmeAlias);
	addrList.push_back(&dbSmeInvalidAddr);
	for (int i = 0; i < addrList.size(); i++)
	{
		for (int j = 0; j < addrList.size(); j++)
		{
			const Address& srcAddr = *addrList[i];
			const Address& destAlias = *addrList[j];
			const Address destAddr = aliasReg->findAddressByAlias(destAlias);
			const RouteHolder* routeHolder = routeReg->lookup(srcAddr, destAddr);
			if (!routeHolder)
			{
				continue;
			}
			const SmeInfo* srcSmeInfo = smeReg->getSme(srcAddr);
			if (!srcSmeInfo)
			{
				continue;
			}
			const RouteInfo& route = routeHolder->route;
			char buf[512];
			sprintf(buf, "%-30s %-30s %c %19d %-20s %-112d %-112d %-15s %-15s",
				str(srcAddr).c_str(), str(destAlias).c_str(),
				route.archived ? 'Y' : 'N', route.billing ? 1 : 0,
				route.routeId.c_str(), route.serviceId,
				route.priority, srcSmeInfo->systemId.c_str(),
				route.smeSystemId.c_str());
			report << buf << endl;
		}
	}
}

void executeFunctionalTest(const string& smscHost, int smscPort)
{
	vector<TestSme*> sme;
	ThreadPool threadPool;
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
			cout << "conf <transceivers> [transmitters = 1] [receivers = 1] [notConnected = 1] - generate config files" << endl;
			cout << "test <start|pause|resume> - pause/resume test execution" << endl;
			cout << "stat - print statistics" << endl;
			cout << "chklist - save checklist" << endl;
			cout << "route info - save route arc/billing info" << endl;
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
			int transceivers = -1; //required
			int transmitters = 1, receivers = 1, notConnected = 1; //optional
			is >> transceivers;
			is >> transmitters;
			is >> receivers;
			is >> notConnected;
			if (transceivers < 0)
			{
				cout << "Required param <transceivers> is missing" << endl;
			}
			else if (transceivers > 0 && transmitters > 0 && receivers > 0 && notConnected > 0)
			{
				sme = genConfig(transceivers, transmitters,
					receivers, notConnected, smscHost, smscPort);
				cout << "Config generated: transceivers = " << transceivers <<
					", transmitters = " << transmitters <<
					", receivers = " << receivers <<
					", notConnected = " << notConnected << endl;
			}
			else
			{
				cout << "All params must be greater than 0" << endl;
			}
		}
		else if (cmd == "test")
		{
			is >> cmd;
			if (cmd == "start")
			{
				threadPool.startTask(pduSender);
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
		else if (cmd == "route")
		{
			is >> cmd;
			if (cmd == "info")
			{
				printRouteArcBillInfo("routes.txt");
				cout << "Routes info saved to file routes.txt" << endl;
			}
			else
			{
				help = true;
			}
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
			threadPool.shutdown();
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
	smppChkList = new SystemSmeCheckList();
	configChkList = new ConfigGenCheckList();
	pduSender = new SmppPduSender();
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
	//delete pduSender;
	return 0;
}

