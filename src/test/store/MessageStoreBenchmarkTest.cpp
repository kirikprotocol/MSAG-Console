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

void executeBenchmarkTest(const int size)
{
	MessageStoreTestCases tc; //throws exception
	SMSId id[size];
	SMSId newId[size];
	SMS sms[size];
	getTime(); //init timer

	//�������� SM
	SMS newSMS;
	for (int i = 0; i < size; i++)
	{
		delete tc.storeCorrectSM(&id[i], &sms[i], RAND_TC);
		delete tc.storeCorrectSM(&newId[i], &newSMS, id[i], sms[i], RAND_TC);
	}
	printResult("storeCorrectSM", 2 * size, getTime());
	
	//���������� �������������� SM � �������
	for (int i = 0; i < size; i++)
	{
		delete tc.storeRejectDuplicateSM(sms[i]);
	}
	printResult("storeRejectDuplicateSM", size, getTime());

	//���������� ������������� SM
	for (int i = 0; i < size; i++)
	{
		delete tc.storeIncorrectSM(RAND_TC);
	}
	printResult("storeIncorrectSM", size, getTime());

	//��������� ����������� �������
	for (int i = 0; i < size; i++)
	{
		delete tc.setCorrectSMStatus(id[i], &sms[i], RAND_TC);
	}
	printResult("setCorrectSMStatus", size, getTime());

	//������������ ��������� ������� SM
	for (int i = 0; i < size; i++)
	{
		delete tc.setIncorrectSMStatus(id[i]);
	}
	printResult("setIncorrectSMStatus", size, getTime());

	//��������� SM
	for (int i = 0; i < size; i++)
	{
		delete tc.replaceCorrectSM(id[i], &sms[i], RAND_TC);
	}
	printResult("replaceCorrectSM", size, getTime());

	//������������ ��������� SM
	for (int i = 0; i < size; i++)
	{
		delete tc.replaceIncorrectSM(id[i], sms[i], RAND_TC);
		delete tc.replaceIncorrectSM2(id[i], sms[i], RAND_TC);
	}
	printResult("replaceIncorrectSM", 2 * size, getTime());

	//������ SM
	for (int i = 0; i < size; i++)
	{
		delete tc.loadExistentSM(id[i], sms[i]);
	}
	printResult("loadExistentSM", size, getTime());

	//�������� SM
	for (int i = 0; i < size; i++)
	{
		delete tc.deleteExistentSM(id[i]);
		delete tc.deleteExistentSM(newId[i]);
	}
	printResult("deleteExistentSM", 2 * size, getTime());

	//��������� ������� �������������� SM
	for (int i = 0; i < size; i++)
	{
		delete tc.setNonExistentSMStatus(id[i], RAND_TC);
	}
	printResult("setNonExistentSMStatus", size, getTime());

	//��������� �������������� SM
	for (int i = 0; i < size; i++)
	{
		delete tc.replaceNonExistentSM(id[i], RAND_TC);
	}
	printResult("replaceNonExistentSM", size, getTime());

	//������ �������������� SM
	for (int i = 0; i < size; i++)
	{
		delete tc.loadNonExistentSM(id[i], RAND_TC);
	}
	printResult("loadNonExistentSM", size, getTime());

	//�������� �������������� SM
	for (int i = 0; i < size; i++)
	{
		delete tc.deleteNonExistentSM(id[i], RAND_TC);
	}

	//������ �����������
	printResult("deleteNonExistentSM", size, getTime());
}

/**
 * �������� ������������������ ���� ���� ������
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

