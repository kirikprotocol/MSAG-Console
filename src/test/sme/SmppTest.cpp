#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "test/sme/SmppTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include "util/debug.h"
#include <vector>
#include <sstream>
#include <iostream>

#define __print__(val) cout << #val << " = " << (val) << endl

using smsc::sme::SmeConfig;
using smsc::test::sms::operator<<;
using smsc::test::core::SmeRegistry;
using smsc::test::util::TestTask;
using namespace std;
using namespace smsc::sms;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads; //ThreadPool, ThreadedTask
using namespace smsc::test::sme;
using namespace smsc::test::sms;

SmeRegistry* smeReg = new SmeRegistry();

/**
 * Тестовая sme.
 */
class TestSme : public TestTask, SmppResponseSender
{
	int smeNum;
	SmppTestCases tc;
	bool boundOk;
	Event evt;

public:
	TestSme(int smeNum, const SmeConfig& config, const SmeSystemId& systemId,
		const Address& addr);
	virtual ~TestSme() {}
	virtual void executeCycle();
	virtual void onStopped();
	virtual bool sendDeliverySmResp(PduDeliverySm& pdu);
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
	bool stopped;
	TestSmeStat() : stopped(false) {}
};

/**
 * Статистика работы всего теста.
 */
class SmppFunctionalTest
{
	typedef vector<TestSmeStat> TestSmeList;
	
	static TestSmeList smeList;
	static Mutex mutex;

public:
	static void resize(int newSize);
	static void onStopped(int taskNum);
	static bool isStopped();
};

//TestSme
TestSme::TestSme(int _smeNum, const SmeConfig& config, const SmeSystemId& systemId,
	const Address& smeAddr)
	: TestTask("TestSme", _smeNum), smeNum(_smeNum),
	tc(config, systemId, smeAddr, this, smeReg, NULL, NULL, NULL), boundOk(false) {}

void TestSme::executeCycle()
{
	__trace__("TestSme::executeCycle()");
	//Bind sme зарегистрированной в smsc
	//Bind sme с неправильными параметрами
	if (!boundOk)
	{
		//boundOk = tc.bindCorrectSme(RAND_TC);
		tc.bindIncorrectSme(1);
		//__require__(boundOk);
		evt.Wait(3000);
	}
	//tc.getTransmitter().submitSm(true, RAND_TC);
	evt.Wait(1000);
}

void TestSme::onStopped()
{
	tc.unbind(); //Unbind для sme соединенной с smsc
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

//TestSmeTaskManager
bool TestSmeTaskManager::isStopped() const
{
	return SmppFunctionalTest::isStopped();
}

//SmppFunctionalTest
SmppFunctionalTest::TestSmeList
	SmppFunctionalTest::smeList =
	SmppFunctionalTest::TestSmeList();
	
inline void SmppFunctionalTest::resize(int newSize)
{
	smeList.clear();
	smeList.resize(newSize);
}

inline void SmppFunctionalTest::onStopped(int taskNum)
{
	//MutexGuard guard(mutex);
	smeList[taskNum].stopped = true;
}

bool SmppFunctionalTest::isStopped()
{
	//MutexGuard guard(mutex);
	bool stopped = true;
	for (int i = 0; i < smeList.size(); i++)
	{
		if(!smeList[i].stopped)
		{
			cout<< "Still running = " << i << endl;
		}
		stopped &= smeList[i].stopped;
	}
	return stopped;
}

void executeFunctionalTest(const string& smscHost, int smscPort)
{
	SmppFunctionalTest::resize(2);
	TestSmeTaskManager tm;
	tm.startTimer();
	int smeNum = 0;
	//fake sme
	{
		Address smeAddr;
		SmsUtil::setupRandomCorrectAddress(&smeAddr);
		SmeInfo sme;
		sme.systemId = "aaaaaaaa";
		smeReg->registerSme(smeAddr, sme, false);
	}
	//sme1
	{
		Address smeAddr;
		SmsUtil::setupRandomCorrectAddress(&smeAddr);
		SmeInfo smeInfo;
		smeInfo.systemId = "bbbbbbbb";
		smeReg->registerSme(smeAddr, smeInfo, false);
		SmeConfig config;
		config.host = smscHost;
		config.port = smscPort;
		config.sid = smeInfo.systemId;
		config.timeOut = 10;
		TestSme* sme = new TestSme(smeNum++, config, smeInfo.systemId, smeAddr); //throws Exception
		tm.addTask(sme);
	}
	//tm.stopTasks();
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
	return 0;
}

