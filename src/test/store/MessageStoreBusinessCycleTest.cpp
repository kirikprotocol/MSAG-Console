#include "util/debug.h"
#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include <iostream>

using namespace std;
using namespace smsc::sms;
using namespace smsc::store;
using namespace smsc::util::config;
using namespace smsc::test::store;
using namespace smsc::test::util;

class MessageStoreBusinessCycleTestTaskHolder;

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
	MessageStoreBusinessCycleTestTaskHolder* holder;
	MessageStoreTestCases tc; //throws exception

public:
	MessageStoreBusinessCycleTestTask(
		MessageStoreBusinessCycleTestTaskHolder* _holder);
	virtual ~MessageStoreBusinessCycleTestTask() {}
	virtual void executeCycle();
	virtual void onStopped();
};

/**
 * Holder ��� ������ �����.
 */
class MessageStoreBusinessCycleTestTaskHolder
{
public:
	char* taskName;
	MessageStoreBusinessCycleTestTask* task;
	bool stopped;
	int ops;

public:
	MessageStoreBusinessCycleTestTaskHolder(int taskNum);
	virtual ~MessageStoreBusinessCycleTestTaskHolder()
	{
		delete[] taskName;
	}
	void stopTask()
	{
		task->stop();
	}
};

/**
 * ���� ��������.
 */
class MessageStoreBusinessCycleTestTaskManager
	: public TestTaskManager<MessageStoreBusinessCycleTestTaskHolder>
{
public:
	MessageStoreBusinessCycleTestTaskManager() {}
	void printStatus();
	int getOps();
};

//MessageStoreBusinessCycleTestTask methods
MessageStoreBusinessCycleTestTask::MessageStoreBusinessCycleTestTask(
	MessageStoreBusinessCycleTestTaskHolder* _holder)
	: TestTask(_holder->taskName), holder(_holder) {}

void MessageStoreBusinessCycleTestTask::executeCycle()
{
	//������ SM ��� ������� ����������� �������� �������������� SM
	SMSId correctId;
	SMS correctSM;
	delete tc.storeCorrectSM(&correctId, &correctSM, RAND_TC); holder->ops++;

	//������ SM � ����� �����
	for (int i = 0; i < 10; i++)
	{
		SMSId id;
		SMS sms;
		delete tc.storeCorrectSM(&id, &sms, RAND_TC); holder->ops++;
		delete tc.loadExistentSM(id, sms); holder->ops++;
	}

	//������ � �������� ������ SM
	delete tc.storeIncorrectSM(correctSM, ALL_TC); holder->ops++;
	delete tc.loadExistentSM(correctId, correctSM); holder->ops++;

	//������ ������ �� �����, � ����� ���� ������ ���� ������
	//������ ������� ���������� ��� ����, ����� ���� ������� ���������� 
	//���������� ���������������� �������� � ����������� ���������� 
	//������������� shared buffers �������� (���� � ���� ���� �������� 
	//������)
	const int listSize = 10;
	for (int j = 0; j < 3; j++)
	{
		SMSId id[listSize];
		SMS sms[listSize];
		for (int i = 0; i < listSize; i++)
		{
			delete tc.storeCorrectSM(&id[i], &sms[i], RAND_TC); holder->ops++;
		}
		for (int i = 0; i < listSize; i++)
		{
			delete tc.loadExistentSM(id[i], sms[i]); holder->ops++;
		}
	}
	delete tc.loadNonExistentSM(); holder->ops++;
	//tc.setCorrectSMStatus(); ops++;
	//tc.createBillingRecord(); ops++;
	//tc.updateCorrectExistentSM(); ops++;
	//tc.deleteExistentSM(); ops++;
}

inline void MessageStoreBusinessCycleTestTask::onStopped()
{
	holder->stopped = true;
}

//MessageStoreBusinessCycleTestTaskHolder methods
MessageStoreBusinessCycleTestTaskHolder::MessageStoreBusinessCycleTestTaskHolder(int taskNum)
	: stopped(false), ops(0)
{
	taskName = new char[25];
	sprintf(taskName, "BusinessCycleTask_%d", taskNum);
	task = new MessageStoreBusinessCycleTestTask(this);
}

//MessageStoreBusinessCycleTestTaskManager methods
void MessageStoreBusinessCycleTestTaskManager::printStatus()
{
	for (int i = 0; i < taskHolders.size(); i++)
	{
		const char* taskName = taskHolders[i]->taskName;
		int ops = taskHolders[i]->ops;
		cout << taskName << ": ops = " << ops << endl;
	}
}

int MessageStoreBusinessCycleTestTaskManager::getOps()
{
	int ops = 0;
	for (int i = 0; i < taskHolders.size(); i++)
	{
		ops += taskHolders[i]->ops;
	}
	return ops;
}

//test body
void executeBusinessCycleTest(int numThreads)
{
	MessageStoreBusinessCycleTestTaskManager tm;
	for (int i = 0; i < numThreads; i++)
	{
		MessageStoreBusinessCycleTestTaskHolder* taskHolder =
			new MessageStoreBusinessCycleTestTaskHolder(i);
		tm.addTask(taskHolder);
	}
	tm.startTimer();

	while (true)
	{
		char ch;
		cin >> ch;
		switch (ch)
		{
			case 'q':
				tm.stopTasks();
				cout << "Total time = " << tm.getExecutionTime() << endl;
				cout << "Total operations = " << tm.getOps() << endl;
				return;
			case 's':
				cout << "Time = " << tm.getExecutionTime() << endl;
				tm.printStatus();
				break;
			default:
				cout << "Time = " << tm.getExecutionTime() << endl;
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

