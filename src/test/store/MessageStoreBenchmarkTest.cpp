#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/Util.hpp"
#include <iostream>
#include <sys/timeb.h>

using namespace std;
using namespace smsc::sms;
using namespace smsc::store;
using namespace smsc::test::store;
using namespace smsc::test::util;
using namespace smsc::util::config;

float getTime()
{
	static timeb prevTime;
    timeb curTime;
	ftime(&curTime);
	float dt = (curTime.time - prevTime.time) +
		(curTime.millitm - prevTime.millitm) / 1000.0;
	ftime(&prevTime);
	return dt;
}

void printResult(const char* name, int size, float time)
{
	cout << name << ": ";
	if (time > 0)
	{
		cout << (size / time) << endl;
	}
	else
	{
		cout << "super performance!" << endl;
	}
}
#define __prepare_for_new_sms__ \
	id.push_back(new SMSId()); \
	sms.push_back(new SMS());
	
#define __prepare_for_new_sms2__ \
	SMSId id1; \
	SMS sms1;
	
void executeBenchmarkTest(const int size)
{
	MessageStoreTestCases tc(StoreManager::getMessageStore(), true); //throws exception
	vector<SMSId*> id;
	vector<SMS*> sms;
	getTime(); //init timer

	//Сохранение правильного sms
	for (int i = 0; i < size; i++)
	{
		__prepare_for_new_sms__;
		tc.storeCorrectSms(id.back(), sms.back(), RAND_TC);
	}
	for (int i = 0; i < size; i++)
	{
		__prepare_for_new_sms2__;
		tc.storeSimilarSms(&id1, &sms1, *id[i], *sms[i], RAND_TC);
	}
	printResult("storeCorrectSms", 2 * size, getTime());
	//Сохранение дублированного sms
	for (int i = 0; i < size; i++)
	{
		__prepare_for_new_sms2__;
		tc.storeDuplicateSms(&id1, &sms1, *id[i], *sms[i]);
	}
	for (int i = 0; i < size; i++)
	{
		tc.storeRejectDuplicateSms(*sms[i]);
	}
	printResult("storeDuplicateSms", 2 * size, getTime());
	//Сохранение sms с замещением существующего
	for (int i = 0; i < size; i++)
	{
		tc.storeReplaceCorrectSms(id[i], sms[i]);
	}
	for (int i = 0; i < size; i++)
	{
		__prepare_for_new_sms2__;
		tc.storeReplaceSmsInFinalState(&id1, &sms1, *sms[i]);
	}
	printResult("storeReplaceSms", 2 * size, getTime());
	//установка статуса
	for (int i = 0; i < size; i++)
	{
		tc.changeExistentSmsStateEnrouteToEnroute(*id[i], sms[i], RAND_TC);
	}
	for (int i = 0; i < size; i++)
	{
		tc.changeExistentSmsStateEnrouteToFinal(*id[i], sms[i], RAND_TC);
	}
	for (int i = 0; i < size; i++)
	{
		tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
	}
	printResult("changeSmsState", 3 * size, getTime());
	//обновление sms
	for (int i = 0; i < size; i++)
	{
		tc.replaceCorrectSms(*id[i], sms[i], RAND_TC);
	}
	for (int i = 0; i < size; i++)
	{
		tc.replaceFinalSms(*id[i], *sms[i]);
	}
	printResult("replaceSms", 2 * size, getTime());
	//чтение sms
	for (int i = 0; i < size; i++)
	{
		tc.loadExistentSms(*id[i], *sms[i]);
	}
	printResult("loadSms", size, getTime());
	//удалить объекты
	for_each(id.begin(), id.end(), Deletor<SMSId>());
	for_each(sms.begin(), sms.end(), Deletor<SMS>());
}

/**
 * Измеряет производительность всех тест кейсов
 */
int main(int argc, char* argv[])
{
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		executeBenchmarkTest(1000);
		StoreManager::shutdown();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

