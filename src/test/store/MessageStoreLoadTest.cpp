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
	int taskNum;
	MessageStoreTestCases tc; //throws exception

public:
	MessageStoreLoadTestTask(int taskNum);
	virtual ~MessageStoreLoadTestTask() {}
	virtual void executeCycle();
	virtual void onStopped();

private:
	void doStat(const TCResult* res);
};

/**
 * Таск менеджер.
 */
class MessageStoreLoadTestTaskManager
	: public TestTaskManager<MessageStoreLoadTestTask>
{
private:
	timeb t1;
	int ops1;

public:
	MessageStoreLoadTestTaskManager() {}
	virtual bool isStopped() const;
	void startTimer();
	void printStatus();
	float getRate();
	int getOps() const;
};

/**
 * Статистика по таскам.
 */
struct MessageStoreLoadTestTaskStat
{
	int ops;
	boolean stopped;
	MessageStoreLoadTestTaskStat()
		: ops(0), stopped(false) {}
};

/**
 * Хранилище статистики всего теста.
 */
class MessageStoreLoadTestStat
{
public:
	typedef vector<MessageStoreLoadTestTaskStat> TaskStatList;

	static TaskStatList taskStat;
};

//MessageStoreLoadTestTask methods
MessageStoreLoadTestTask::MessageStoreLoadTestTask(int _taskNum)
	: TestTask("LoadTask", _taskNum), taskNum(_taskNum) {}

void MessageStoreLoadTestTask::executeCycle()
{
	SMSId id;
	SMS sms;
	for (int i = 0; i < 20; i++)
	{
		doStat(tc.storeCorrectSM(&id, &sms, RAND_TC));
		//tc.setCorrectSMStatus();
		//tc.createBillingRecord();
	}
	//tc.updateCorrectExistentSM(); ops++;
	//tc.deleteExistentSM(); ops++;
	//tc.loadExistentSM(); ops++;
}

inline void MessageStoreLoadTestTask::onStopped()
{
	MessageStoreLoadTestStat::taskStat[taskNum].stopped = true;
}

void MessageStoreLoadTestTask::doStat(const TCResult* res)
{
	(MessageStoreLoadTestStat::taskStat[taskNum].ops)++;
	delete res;
}

//MessageStoreLoadTestTaskManager methods
inline void MessageStoreLoadTestTaskManager::startTimer()
{
	TestTaskManager<MessageStoreLoadTestTask>::startTimer();
	ftime(&t1);
	ops1 = getOps();
}

bool MessageStoreLoadTestTaskManager::isStopped() const
{
	MessageStoreLoadTestStat::TaskStatList& taskStat = 
		MessageStoreLoadTestStat::taskStat;
	bool stopped = true;
	for (int i = 0; stopped && (i < taskStat.size()); i++)
	{
		stopped &= taskStat[i].stopped;
	}
	return stopped;
}

void MessageStoreLoadTestTaskManager::printStatus()
{
	MessageStoreLoadTestStat::TaskStatList& taskStat = 
		MessageStoreLoadTestStat::taskStat;
	int totalOps = 0;
	for (int i = 0; i < taskStat.size(); i++)
	{
		int ops = taskStat[i].ops;
		totalOps += ops;
		cout << "LoadTask_" << i << ": ops = " << ops << endl;
	}
	cout << "Total ops = " << totalOps << endl;
	cout << "------------------------" << endl;
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
	MessageStoreLoadTestStat::TaskStatList& taskStat = 
		MessageStoreLoadTestStat::taskStat;
	int totalOps = 0;
	for (int i = 0; i < taskStat.size(); i++)
	{
		totalOps += taskStat[i].ops;
	}
	return totalOps;
}

//MessageStoreBusinessCycleTestStat
MessageStoreLoadTestStat::TaskStatList MessageStoreLoadTestStat::taskStat =
	MessageStoreLoadTestStat::TaskStatList();

//test body
void executeLoadTest(int numThreads)
{
	MessageStoreLoadTestStat::taskStat.resize(numThreads);
	MessageStoreLoadTestTaskManager tm;
	for (int i = 0; i < numThreads; i++)
	{
		MessageStoreLoadTestTask* task =
			new MessageStoreLoadTestTask(i);
		tm.addTask(task);
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
				cout << "Average rate = " <<
					(tm.getOps() / tm.getExecutionTime()) << endl;
				return;
			case 's':
				cout << "Time = " << tm.getExecutionTime() << endl;
				tm.printStatus();
				break;
			default:
				cout << "Time = " << tm.getExecutionTime() << endl;
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

