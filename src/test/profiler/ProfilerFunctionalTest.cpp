#include "ProfilerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "test/core/ProfileUtil.hpp"
#include "core/threads/ThreadPool.hpp"
#include "smeman/smeman.h"
#include "smeman/smeproxy.h"
#include "util/Logger.h"
#include "util/config/Manager.h"
#include "ProfilerCheckList.hpp"
#include <sstream>

using log4cpp::Category;
using smsc::util::Logger;
using smsc::profiler::Profile;
using smsc::smeman::SmeManager;
using smsc::smeman::ProxyMonitor;
using smsc::core::threads::ThreadPool;
using smsc::core::threads::ThreadedTask;
using smsc::test::sms::SmsUtil;
using namespace smsc::test::core; //ProfileRegistry, ProfileUtil
using namespace smsc::test::profiler; //ProfilerTestCases, ProfilerCheckList
using namespace smsc::test::util; //TCSelector, Deletor
using namespace std;

static Category& log = Logger::getCategory("ProfilerFunctionalTest");

class ProfilerMonitor : public ThreadedTask, public ProxyMonitor
{
	ProfilerTestCases* tc;
public:
	ProfilerMonitor(ProfilerTestCases* _tc) : tc(_tc) {}
	virtual int Execute();
	virtual const char* taskName()
	{
		return "ProfilerMonitor";
	}
};

class ProfilerFunctionalTest
{
	Profile defProfile;
	SmeManager smeMan;
	Profiler* profiler;
	ThreadPool threadPool;
	ProfileRegistry* profileReg;
	CheckList* chkList;
	ProfilerTestCases* tc;
	ProfilerMonitor* monitor;
	vector<Address> addr1;
	vector<Address> addr2;

public:
	ProfilerFunctionalTest(CheckList* chkList);
	~ProfilerFunctionalTest();
	void executeTest(int numAddr);

private:
	void reinit();
	void executeTestCases(const Address& address);
};

int ProfilerMonitor::Execute()
{
	while (true)
	{
		Wait();
		if (isStopping)
		{
			break;
		}
		__trace2__("ProfilerMonitor notified");
		tc->onCommand();
	}
}

ProfilerFunctionalTest::ProfilerFunctionalTest(CheckList* _chkList)
	: profiler(NULL), tc(NULL), monitor(NULL), chkList(_chkList)
{
	ProfileUtil::setupRandomCorrectProfile(defProfile);
	profileReg = new ProfileRegistry(defProfile);
	reinit();
}

ProfilerFunctionalTest::~ProfilerFunctionalTest()
{
	//delete profiler;
	delete profileReg;
	delete tc;
}

void ProfilerFunctionalTest::reinit()
{
	//Пересоздать profiler и убедиться, что все нормально сохраняется и
	//читается из БД. Мой profileReg оставить тот же самый
	if (profiler)
	{
		profiler->stop();
		//profiler->putCommand(SmscCommand()); //чтобы отработал stop()
		//delete profiler;
	}
	profiler = new Profiler(defProfile, &smeMan, "profiler");
	profiler->loadFromDB();
	if (tc)
	{
		delete tc;
	}
	tc = new ProfilerTestCases(profiler, profileReg, chkList);
	if (monitor)
	{
		monitor->stop();
	}
	monitor = new ProfilerMonitor(tc);
	profiler->attachMonitor(monitor);
	threadPool.startTask(monitor);
	threadPool.startTask(profiler);
}

void ProfilerFunctionalTest::executeTestCases(const Address& address)
{
	//Создание нового профиля удовлетворяющего условиям поиска, 1/5
	//Создание нового профиля неудовлетворяющего условиям поиска, 1/5
	//Обновление уже существующего профиля, 1/5
	//Поиск профиля для заданного адреса, 2/5
	Address addr;
	bool created = false;
	for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				addr = address;
				tc->createProfileMatch(addr, RAND_TC);
				created = true;
				break;
			case 2:
				addr = address;
				tc->createProfileNotMatch(addr, RAND_TC);
				created = true;
				break;
			case 3:
				if (created)
				{
					tc->updateProfile(addr);
				}
				break;
			default:
				tc->lookup(address);
		}
	}
}

void ProfilerFunctionalTest::executeTest(int numAddr)
{
	//Подготовка списка адресов
	addr1.resize(numAddr);
	addr2.resize(numAddr);
	for (int i = 0; i < numAddr; i++)
	{
		SmsUtil::setupRandomCorrectAddress(&addr1[i]);
		SmsUtil::setupRandomCorrectAddress(&addr2[i]);
	}
	addr1.push_back(Address(".0.0.0"));
	addr1.push_back(Address(".0.0.1"));
	addr2.push_back(Address(".0.0.79029246519"));
	//addr2.push_back(Address(".0.1.0"));
	//На каждый адрес регистрация случайного количества профилей
	for (int i = 0; i < addr1.size(); i++)
	{
		executeTestCases(addr1[i]);
	}
	for (int i = 0; i < addr2.size(); i++)
	{
		tc->putCommand(addr2[i], getDataCoding(RAND_TC), RAND_TC);
	}
	sleep(1);
	reinit();
	//Поиск профилей
	for (int i = 0; i < addr1.size(); i++)
	{
		tc->lookup(addr1[i]);
	}
	for (int i = 0; i < addr2.size(); i++)
	{
		tc->lookup(addr2[i]);
	}
	//На каждый адрес регистрация случайного количества профилей
	for (int i = 0; i < addr1.size(); i++)
	{
		executeTestCases(addr1[i]);
	}
	for (int i = 0; i < addr2.size(); i++)
	{
		tc->putCommand(addr2[i], getDataCoding(RAND_TC), RAND_TC);
	}
	sleep(1);
	reinit();
	//Поиск профилей
	for (int i = 0; i < addr1.size(); i++)
	{
		tc->lookup(addr1[i]);
	}
	for (int i = 0; i < addr2.size(); i++)
	{
		tc->lookup(addr2[i]);
	}
	//очистка памяти
	addr1.clear();
	addr2.clear();
}

/**
 * Выполняет тестирование Profile и
 * выводит результат по тест кейсам в checklist.
 */
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: ProfilerFunctionalTest <numCycles> <numAddr>" << endl;
		exit(0);
	}

	const int numCycles = atoi(argv[1]);
	const int numAddr = atoi(argv[2]);
	smsc::util::config::Manager::init("config.xml");
	try
	{
		//Manager::init("config.xml");
		ProfilerCheckList chkList;
		ProfilerFunctionalTest test(&chkList);
		for (int i = 0; i < numCycles; i++)
		{
			test.executeTest(numAddr);
		}
		__trace__("Before save checklist");
		chkList.save();
		chkList.saveHtml();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

