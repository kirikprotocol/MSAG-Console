#include "util/debug.h"
#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include <iostream>
#include <sys/timeb.h>

using namespace std;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace smsc::store;
using namespace smsc::test::store;
using namespace smsc::test::util;

class MessageStoreLoadTestTaskHolder;

/**
 * Предназначен для измерения производительности Message Store.
 * Выполняет только корректные операции по работе с Message Store.
 * Результат выполнения теста представлен 2-мя цифрами:
 * <ul>
 * <li>Количество операций в секунду.
 * <li>Полное число созданных в БД записей.
 * </ul>
 */
class MessageStoreLoadTestTask : public TestTask
{
private:
	MessageStoreLoadTestTaskHolder* holder;
	MessageStoreTestCases tc; //throws exception

public:
	MessageStoreLoadTestTask(MessageStoreLoadTestTaskHolder* _holder);
	virtual ~MessageStoreLoadTestTask() {}
	virtual void executeCycle();
	virtual void onStopped();
};

/**
 * Holder для самого таска.
 */
class MessageStoreLoadTestTaskHolder
{
public:
	char* taskName;
	MessageStoreLoadTestTask* task;
	bool stopped;
	int ops;

public:
	MessageStoreLoadTestTaskHolder(int taskNum);
	virtual ~MessageStoreLoadTestTaskHolder()
	{
		delete[] taskName;
	}
	void stopTask()
	{
		task->stop();
	}
};

/**
 * Таск менеджер.
 */
class MessageStoreLoadTestTaskManager
	: public TestTaskManager<MessageStoreLoadTestTaskHolder>
{
private:
	timeb t1;
	int ops1;

public:
	void startTimer();
	void printStatus();
	float getRate();
	int getOps() const;
};

//MessageStoreLoadTestTask methods
MessageStoreLoadTestTask::MessageStoreLoadTestTask(
	MessageStoreLoadTestTaskHolder* _holder)
	: TestTask(_holder->taskName), holder(_holder) {}

void MessageStoreLoadTestTask::executeCycle()
{
	SMSId id;
	SMS sms;
	int maxOps = 10 * (2 + holder->ops / 10);
	for (; holder->ops < maxOps; holder->ops++)
	{
		delete tc.storeCorrectSM(&id, &sms, RAND_TC);
		//tc.setCorrectSMStatus();
		//tc.createBillingRecord();
	}
	//tc.updateCorrectExistentSM(); ops++;
	//tc.deleteExistentSM(); ops++;
	//tc.loadExistentSM(); ops++;
}

inline void MessageStoreLoadTestTask::onStopped()
{
	holder->stopped = true;
}

//MessageStoreLoadTestTaskHolder methods
MessageStoreLoadTestTaskHolder::MessageStoreLoadTestTaskHolder(int taskNum)
	: stopped(false), ops(0)
{
	taskName = new char[15];
	sprintf(taskName, "LoadTask_%d", taskNum);
	task = new MessageStoreLoadTestTask(this);
}

//MessageStoreLoadTestTaskManager methods
inline void MessageStoreLoadTestTaskManager::startTimer()
{
	ftime(&t1);
	ops1 = getOps();
}

void MessageStoreLoadTestTaskManager::printStatus()
{
	for (int i = 0; i < taskHolders.size(); i++)
	{
		const char* taskName = taskHolders[i]->taskName;
		int ops = taskHolders[i]->ops;
		cout << taskName << ": ops = " << ops << endl;
		//cout << i << ": ops = " << tasks[i]->getOps() << endl;
	}
}

float MessageStoreLoadTestTaskManager::getRate()
{
	int ops2 = getOps();
	timeb t2; ftime(&t2);
	float dt = (t2.time - t1.time) + (t2.millitm - t1.millitm) / 1000.0;
	float rate = (ops2 - ops1) / dt;
	t1 = t2;
	ops1 = ops2;
	return rate;
}

int MessageStoreLoadTestTaskManager::getOps() const
{
	int ops = 0;
	for (int i = 0; i < taskHolders.size(); i++)
	{
		ops += taskHolders[i]->ops;
	}
	return ops;
}

//test body
void executeLoadTest(int numThreads)
{
	MessageStoreLoadTestTaskManager tm;
	for (int i = 0; i < numThreads; i++)
	{
		MessageStoreLoadTestTaskHolder* taskHolder =
			new MessageStoreLoadTestTaskHolder(i);
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
				cout << "Total operations = " << tm.getOps() << endl;
				return;
			case 's':
				tm.printStatus();
				break;
			default:
				cout << "Rate = " << tm.getRate()
					<< " messages/second" << endl;
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "Usage: MessageStoreLoadTest <numThreads>" << endl;
		exit(0);
	}
	
	const int numThreads = atoi(argv[1]);
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		executeLoadTest(numThreads);
		StoreManager::shutdown();
		exit(0);
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
		exit(-1);
	}

	return 0;
}

