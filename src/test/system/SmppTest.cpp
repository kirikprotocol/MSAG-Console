#include "system/smsc.hpp"
#include "util/config/Manager.h"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/router/RouteManagerTestCases.hpp"
#include "system/smppio/SmppAcceptor.hpp"
#include "store/StoreManager.h"
#include "system/state_machine.hpp"
#include "admin/util/SignalHandler.h"
#include "test/core/MessageRegistry.hpp"
#include "test/core/RouteRegistry.hpp"
#include <vector>

namespace test {
namespace smsc {
namespace system {

using std::vector;
using smsc::sms::Address;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeSystemId;
using smsc::system::StateMachine;
using smsc::system::SmscSignalHandler;
using smsc::system::smppio::SmppAcceptor;
using smsc::store::StoreManager;
using smsc::util::config::Manager;
using smsc::admin::util::SignalHandler;
using smsc::test::core::TestRouteData;
using smsc::test::sms::SmsUtil;
using smsc::test::smeman::SmeManagerTestCases;
using smsc::test::router::RouteManagerTestCases;

/**
 * Тестовая sme.
 */
class TestSme : public TestTask
{
	BaseSme sme;
	SmeTestCases tc;
	int sentPduCount;
public:
	TestSme(const char* systemId, const Address& origAddr, const char* serviceType);
	virtual ~TestSme();
	virtual void executeCycle();
	virtual void onStopped();
	virtual void onMessage();

private:
	void process(const TCResult* res);
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
	int deliveredSms;
	int receivedDeliveryReceipts;
	int receivedIntermediateNotifications;
	int lostSms;
	int lostDeliveryReceipts;
	int lostIntermediateNotifications;
	Mutex lock;

	TestSmeStat() : deliveredSms(0), receivedDeliveryReceipts(0),
		receivedIntermediateNotifications(0), lostSms(0),
		lostDeliveryReceipts(0), lostIntermediateNotifications(0);
};

/**
 * Статистика работы всего теста.
 */
class SmppTest
{
	typedef map<const Address, TestSmeStat> SmeStatMap;
	SmeStatMap smeStatMap;

public:
	static void process(const Address& origAddr, const TCResult* res);
	static void process(const Address& origAddr, int lostSms,
		int lostDeliveryReceipts, int lostIntermediateNotifications)
	static void printStatBySme();
	static int getDeliveredSms();
};

/**
 * Подмененный SC.
 */
class TestSmsc : Smsc
{
	int numSme;
public:
	TestSmsc(int _numSme) : numSme(_numSme) {}
	void init();
};

/**
 * Стартовалка для SC.
 */
class SmscStarter : ThreadedTask
{
	int numSme;
public:
	SmscStarter(int _numSme) : numSme(_numSme) {}
	virtual const char* taskName();
	virtual int Execute();
};

//TestSme
TestSme::TestSme(const char* systemId, const Address& origAddr,
	const char* serviceType) : sme(systemId, origAddr, serviceType), tc(sme),
	sentPduCount(0);
{
	sme.setListener(this);
	//Bind для sme незарегистрированной в smsc.
	//Bind для sme зарегистрированной в smsc.
	if (rand0(1))
	{
		process(tc.bindNonRegisteredSme(RAND_TC));
	}
	process(tc.bindRegisteredSme(RAND_TC));
}

TestSme::~TestSme()
{
	process(tc.unbindBounded());
	if (rand0(1))
	{
		process(tc.unbindNonBounded());
	}
}

void TestSme::executeCycle()
{
	debug("*** start ***");
	//Отправка корректного sms другим sme
	process(tc.submitCorrectSms(routeReg, msgReg, RAND_TC));
	sentPduCount++;
	//Проверка неполученых подтверждений доставки, нотификаций и sms от других sme
	if (sentPduCount % 100 == 0)
	{
		process(processInvalidSms(msgReg));
	}
}

void TestSme::onMessage()
{
	//Получение подтверждений доставки, нотификаций и sms от других sme
	process(processSms(sms, routeReg, msgReg));
	SmppTest::process(sms.getOriginatingAddress(), res);
}

void TestSme::process(const TCResult* res)
{
	delete res;
}

//SmppTest
void SmppTest::process(const Address& origAddr, const TCResult* res)
{
	TestSmeStat& smeStat = smeStatMap[origAddr];
	MutextGuard(smeStat.mutex);
	if (res.getId() == "")
	{
		smeStat.deliveredSms++;
	}
	else if (res.getId() == "")
	{
		smeStat.receivedDeliveryReceipts++;
	}
	else if (res.getId() == "")
	{
		smeStat.receivedIntermediateNotifications++;
	}
}

void SmppTest::process(const Address& origAddr, int lostSms,
	int lostDeliveryReceipts, int lostIntermediateNotifications)
{
	TestSmeStat& smeStat = smeStatMap[origAddr];
	MutextGuard(smeStat.mutex);
	smeStat.lostSms += lostSms;
	smeStat.lostDeliveryReceipts += lostDeliveryReceipts;
	smeStat.lostIntermediateNotifications += lostIntermediateNotifications;
}

//TestSmsc
void TestSmsc::init()
{
	Manager::init("config.xml");
	cfgman = &cfgman->getInstance();
	router.assign(&smeman);

	//Настройка конфигурации
	SmeManagerTestCases tcSme(smeman);
	RouteManagerTestCases tcRoute;

	vector<SmeInfo*> sme;
	vector<Address*> addr;
	
	//Регистрация sme с пустым systemId
	//Регистрация sme с корректными параметрами
	bool emptySystemIdSme = false;
	for (int i = 0; i < numSme; i++)
	{
		addr.push_back(new Address());
		SmsUtil::setupRandomCorrectAddress(addr.back());
		sme.push_back(new SmeInfo());
		if (!emptySystemIdSme)
		{
			emptySystemIdSme = true;
			delete tcSme.addCorrectSmeWithEmptySystemId(sme.back());
		}
		else
		{
			delete tcSme.addCorrectSme(sme.back(), RAND_TC);
		}
	}

	//Добавление корректного маршрута
	//Добавление корректного маршрута с неправильными (непроверяемыми) значениями
	for (int i = 0; i < numSme; i++)
	{
		for (int j = 0; j < numSme; j++)
		{
			const Address& origAddr = *addr[i];
			const Address& destAddr = *addr[j];
			const SmeSystemId& smeSystemId = sme[rand0(numSme - 1)]->systemId;
			TestRouteData routeData;
			if (rand0(3))
			{
				delete tcRoute.addCorrectRoute(systemId, &routeData, RAND_TC);
			}
			else
			{
				delete tcRoute.addCorrectRoute2(systemId, &routeData, RAND_TC);
			}
			routeReg.putRoute(routeData);
		}
	}
	
	//Старт SC
	int numThreads = 5;
	tp.preCreateThreads(numThreads);
	tp.startTask(new SmppAcceptor(cfgman->getString("smpp.host"),
		cfgman->getInt("smpp.port"), &ssockman));
	StoreManager::startup(Manager::getInstance());
	store = smsc::store::StoreManager::getMessageStore();
	for (int i = 0; i < numThreads; i++)
	{
		tp.startTask(new StateMachine(eventqueue,store,this));
	}
	SignalHandler::registerShutdownHandler(new SmscSignalHandler(this));
}

//SmscStarter
const char* SmscStarter::taskName()
{
	return "SC starter";
}
int SmscStarter::Execute()
{
	TestSmsc *app = new TestSmsc(numSme);
	createSme(numSme);
	try
	{
		app->init();
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

void createSme(int numSme)
{
	for (int i = 0; i < numSme; i++)
	{
		Address origAddr;
		SmsUtil::setupRandomCorrectAddress(&origAddr);
		TestSme(const char* systemId, const Address& origAddr, const char* serviceType);
	}
}

}
}
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "Usage: TestSmsc <numSme>" << endl;
		exit(0);
	}
	const int numSme = atoi(argv[1]);
	ThreadPool pool;
	pool.startTask(new SmscStarter(numSme));
	pool.shutdown();

	return 0;
}

