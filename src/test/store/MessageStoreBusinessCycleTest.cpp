#include "util/debug.h"
#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include <iostream>
#include <sys/timeb.h>

using namespace std;
using namespace smsc::sms;
using namespace smsc::store;
using namespace smsc::util::config;
using namespace smsc::test::store;
using namespace smsc::test::util;

/**
 * ������������ ��� ������ ������������ � ������������ ������ ������ Message 
 * Store �� ���������� ��������� ����������.
 * ��������� ��� ���������� � ������������ test cases �� ������ � Message Store 
 * ����� assert test cases.
 * ��������� ���������� ����� ����� ���� ���������:
 * <ul>
 * <li>����������� ������� � multi-thread ���������.
 * <li>�������������� ��������� ��� ��������� ���� � �.�.
 * <li>������������ �� ������������ ������.
 * </ul>
 */
class MessageStoreBusinessCycleTestTask : public TestTask
{
private:
	MessageStoreTestCases tc; //throws exception
	int ops;

public:
	MessageStoreBusinessCycleTestTask(int taskNum) : TestTask(taskNum), ops(0) {}

	virtual void executeCycle()
	{
		//������ SM ��� ������� ����������� �������� �������������� SM
		SMSId correctId;
		SMS correctSM;
		delete tc.storeCorrectSM(&correctId, &correctSM, RAND_TC); ops++;
		
		//������ SM � ����� �����
		for (int i = 0; i < 10; i++)
		{
			SMSId id;
			SMS sms;
			delete tc.storeCorrectSM(&id, &sms, RAND_TC); ops++;
			delete tc.loadExistentSM(id, sms); ops++;
		}
		
		//������ � �������� ������ SM
		delete tc.storeIncorrectSM(correctSM, ALL_TC); ops++;
		delete tc.loadExistentSM(correctId, correctSM); ops++;
		
		//������ ������ �� �����, � ����� ���� ������ ���� ������
		//������ ������� ���������� ��� ����, ����� ���� ������� ���������� 
		//���������� ���������������� �������� � ����������� ���������� 
		//������������� shared buffers �������� (���� � ���� ���� �������� 
		//������)
		const int listSize = 100;
		for (int j = 0; j < 5; j++)
		{
			SMSId id[listSize];
			SMS sms[listSize];
			for (int i = 0; i < listSize; i++)
			{
				delete tc.storeCorrectSM(&id[i], &sms[i], RAND_TC); ops++;
			}
			for (int i = 0; i < listSize; i++)
			{
				delete tc.loadExistentSM(id[i], sms[i]); ops++;
			}
		}
		delete tc.loadNonExistentSM(); ops++;
		//tc.setCorrectSMStatus(); ops++;
		//tc.createBillingRecord(); ops++;
		//tc.updateCorrectExistentSM(); ops++;
		//tc.deleteExistentSM(); ops++;
	}

	int getOps() const
	{
		return ops;
	}

	virtual ~MessageStoreBusinessCycleTestTask() {}
};

class MessageStoreBusinessCycleTestTaskManager
	: public TestTaskManager<MessageStoreBusinessCycleTestTask>
{
private:
	timeb t1;

public:
	MessageStoreBusinessCycleTestTaskManager()
	{
		ftime(&t1);
	}

	void printStatus()
	{
		for (int i = 0; i < tasks.size(); i++)
		{
			cout << tasks[i]->taskName() << ": ops = " << tasks[i]->getOps()
				<< endl;
		}
	}

	int getOps()
	{
		int ops = 0;
		for (int i = 0; i < tasks.size(); i++)
		{
			ops += tasks[i]->getOps();
		}
		return ops;
	}
};

void executeBusinessCycleTest(int numThreads)
{
	MessageStoreBusinessCycleTestTaskManager tm;
	for (int i = 0; i < numThreads; i++)
	{
		MessageStoreBusinessCycleTestTask* task =
			new MessageStoreBusinessCycleTestTask(i);
		tm.addTask(task);
	}

	while (true)
	{
		char ch;
		cin >> ch;
		switch (ch)
		{
			case 'q':
				tm.stopTasks();
				cout << "Total operations = " << tm.getOps() << endl;
				return;
			case 's':
				tm.printStatus();
				break;
			default:
				cout << "Total operations = " << tm.getOps() << endl;
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "Usage: MessageStoreBusinessCycleTest <numThreads>" << endl;
		exit(0);
	}
	
	//��������� �����
	const int numThreads = atoi(argv[1]);
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		executeBusinessCycleTest(numThreads);
		StoreManager::shutdown();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

