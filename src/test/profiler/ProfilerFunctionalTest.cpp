#include "ProfilerTestCases.hpp"
#include "util/Logger.h"
#include "test/sms/SmsUtil.hpp"
#include "ProfilerCheckList.hpp"
#include <sstream>

using log4cpp::Category;
using smsc::util::Logger;
using smsc::profiler::Profile;
using smsc::test::sms::SmsUtil;
using smsc::test::core::ProfileRegistry;
using namespace smsc::test::profiler; //ProfilerTestCases, ProfilerCheckList
using namespace smsc::test::util; //TCSelector, Deletor
using namespace std;

static Category& log = Logger::getCategory("ProfilerFunctionalTest");

class ProfilerFunctionalTest
{
	Profile defProfile;
	Profiler* profiler;
	ProfileRegistry* profileReg;
	CheckList* chkList;
	ProfilerTestCases* tc;
	vector<Address*> addr;

public:
	ProfilerFunctionalTest(CheckList* chkList);
	~ProfilerFunctionalTest();
	void executeTest(int numAddr);

private:
	void reinit();
	void executeTestCases(const Address& address);
};

ProfilerFunctionalTest::ProfilerFunctionalTest(CheckList* _chkList)
	: profiler(NULL), tc(NULL), chkList(_chkList)
{
	ProfilerTestCases::setupRandomCorrectProfile(defProfile);
	profileReg = new ProfileRegistry(defProfile);
	reinit();
}

ProfilerFunctionalTest::~ProfilerFunctionalTest()
{
	for_each(addr.begin(), addr.end(), Deletor<Address>());
	delete profiler;
	delete profileReg;
	delete tc;
}

void ProfilerFunctionalTest::executeTestCases(const Address& address)
{
	//Создание нового профиля удовлетворяющего условиям поиска, 1/5
	//Создание нового профиля неудовлетворяющего условиям поиска, 1/5
	//Обновление уже существующего профиля, 1/5
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

void ProfilerFunctionalTest::reinit()
{
	//Пересоздать profiler и убедиться, что все нормально сохраняется и
	//читается из БД. Мой profileReg оставить тот же самый
	if (profiler)
	{
		delete profiler;
	}
	profiler = new Profiler(defProfile);
	if (tc)
	{
		delete tc;
	}
	tc = new ProfilerTestCases(profiler, profileReg, chkList);
}

void ProfilerFunctionalTest::executeTest(int numAddr)
{
	//Подготовка списка адресов
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		SmsUtil::setupRandomCorrectAddress(addr.back());
	}
	//На каждый адрес регистрация случайного количества профилей
	for (int i = 0; i < numAddr; i++)
	{
		executeTestCases(*addr[i]);
	}
	reinit();
	//На каждый адрес регистрация случайного количества профилей
	for (int i = 0; i < numAddr; i++)
	{
		executeTestCases(*addr[i]);
	}
	reinit();
	//Поиск профилей
	for (int i = 0; i < numAddr; i++)
	{
		tc->lookup(*addr[i]);
	}
	//очистка памяти
	for_each(addr.begin(), addr.end(), Deletor<Address>());
	addr.clear();
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
	try
	{
		//Manager::init("config.xml");
		ProfilerCheckList chkList;
		for (int i = 0; i < numCycles; i++)
		{
			ProfilerFunctionalTest test(&chkList);
			test.executeTest(numAddr);
		}
		chkList.save();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

