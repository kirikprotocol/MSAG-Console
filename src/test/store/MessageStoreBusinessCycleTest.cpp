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
 * ѕредназначен дл€ стресс тестировани€ и тестировани€ бизнес циклов Message 
 * Store на длительных временных интервалах.
 * ¬ыполн€ет все корректные и некорректные test cases по работе с Message Store 
 * кроме assert test cases.
 * –езультат выполнени€ теста может быть следующий:
 * <ul>
 * <li>ќбнаружение проблем в multi-thread окружении.
 * <li>¬осстановление коннектов при проблемах сети и т.п.
 * <li>“естирование на фрагментацию пам€ти.
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
 * Holder дл€ самого таска.
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
 * “аск менеджер.
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
	//создаю SM дл€ попытки дальнейшего создание дублированного SM
	SMSId correctId;
	SMS correctSM;
	delete tc.storeCorrectSM(&correctId, &correctSM, RAND_TC); holder->ops++;

	//создаю SM и сразу читаю
	for (int i = 0; i < 10; i++)
	{
		SMSId id;
		SMS sms;
		delete tc.storeCorrectSM(&id, &sms, RAND_TC); holder->ops++;
		delete tc.loadExistentSM(id, sms); holder->ops++;
	}

	//создаю и загружаю кривые SM
	delete tc.storeIncorrectSM(correctSM, ALL_TC); holder->ops++;
	delete tc.loadExistentSM(correctId, correctSM); holder->ops++;

	//создаю список не чита€, а потом буду читать этот список
	//список большой специально дл€ того, чтобы было большое количество 
	//одинаковых последовательных операций и веро€тность ошибочного 
	//использовани€ shared buffers возросла (если в коде есть подобные 
	//ошибки)
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
	
	//запустить таски
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

