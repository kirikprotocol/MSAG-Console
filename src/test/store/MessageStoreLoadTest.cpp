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
	MessageStoreTestCases* tc;

public:
	MessageStoreLoadTestTask(MessageStoreTestCases* tc, int taskNum);
	virtual ~MessageStoreLoadTestTask() {}
	virtual void executeCycle();
	virtual void onStopped();
	void updateStat();
};

/**
 * Таск менеджер.
 */
class MessageStoreLoadTestTaskManager
	: public TestTaskManager<MessageStoreLoadTestTask>
{
public:
	MessageStoreLoadTestTaskManager() {}
	virtual bool isStopped() const;
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
class MessageStoreLoadTest
{
private:
	typedef vector<MessageStoreLoadTestTaskStat> TaskStatList;
	static TaskStatList taskStat;
	static timeb t1;
	static int ops1;

public:
	static void init(int numThreads);
	static bool isStopped();
	static void onStopped(int taskNum);
	static void updateStat(int taskNum);
	static void printStatus();
	static float getRate();
	static int getOps();
};

//MessageStoreLoadTestTask methods
MessageStoreLoadTestTask::MessageStoreLoadTestTask(MessageStoreTestCases* _tc,
	int _taskNum) : TestTask("LoadTask", _taskNum), tc(_tc), taskNum(_taskNum)
{
	__require__(tc);
}

void MessageStoreLoadTestTask::executeCycle()
{
	for (int i = 0; i < 20; i++)
	{
		SMSId id;
		SMS sms;
		tc->storeCorrectSms(&id, &sms, RAND_TC);
		tc->changeExistentSmsStateEnrouteToFinal(id, &sms, RAND_TC);
		updateStat();
	}
}

inline void MessageStoreLoadTestTask::onStopped()
{
	MessageStoreLoadTest::onStopped(taskNum);
}

inline void MessageStoreLoadTestTask::updateStat()
{
	MessageStoreLoadTest::updateStat(taskNum);
}

//MessageStoreLoadTestTaskManager methods
inline bool MessageStoreLoadTestTaskManager::isStopped() const
{
	return MessageStoreLoadTest::isStopped();
}

//MessageStoreBusinessCycleTest
MessageStoreLoadTest::TaskStatList MessageStoreLoadTest::taskStat =
	MessageStoreLoadTest::TaskStatList();
timeb MessageStoreLoadTest::t1 = timeb();
int MessageStoreLoadTest::ops1 = 0;

inline void MessageStoreLoadTest::init(int numThreads)
{
	ftime(&t1);
	ops1 = getOps();
	taskStat.resize(numThreads);
}

bool MessageStoreLoadTest::isStopped()
{
	bool stopped = true;
	for (int i = 0; stopped && (i < taskStat.size()); i++)
	{
		stopped &= taskStat[i].stopped;
	}
	return stopped;
}

inline void MessageStoreLoadTest::onStopped(int taskNum)
{
	taskStat[taskNum].stopped = true;
}

inline void MessageStoreLoadTest::updateStat(int taskNum)
{
	taskStat[taskNum].ops++;
}

void MessageStoreLoadTest::printStatus()
{
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

float MessageStoreLoadTest::getRate()
{
	int ops2 = getOps();
	timeb t2; ftime(&t2);
	float dt = (t2.time - t1.time) + (t2.millitm - t1.millitm) / 1000.0;
	float rate = (ops2 - ops1) / dt;
	t1 = t2;
	ops1 = ops2;
	return rate;
}

int MessageStoreLoadTest::getOps()
{
	int totalOps = 0;
	for (int i = 0; i < taskStat.size(); i++)
	{
		totalOps += taskStat[i].ops;
	}
	return totalOps;
}

//test body
void executeLoadTest(int numThreads)
{
	MessageStoreLoadTest::init(numThreads);
	MessageStoreLoadTestTaskManager tm;
	MessageStoreTestCases tc(StoreManager::getMessageStore(), true); //throws exception
	for (int i = 0; i < numThreads; i++)
	{
		MessageStoreLoadTestTask* task =
			new MessageStoreLoadTestTask(&tc, i);
		tm.addTask(task);
	}
	tm.startTimer();

	string cmd;
	bool help = true;
	while (true)
	{
		//хелп
		if (help)
		{
			help = false;
			cout << "rate - print rate" << endl;
			cout << "stat - print statistics" << endl;
			cout << "quit - quit" << endl;
		}

		//обработка команд
		cin >> cmd;
		if (cmd == "quit")
		{
			tm.stopTasks();
			cout << "Total time = " << tm.getExecutionTime() << endl;
			cout << "Total operations = " << MessageStoreLoadTest::getOps() << endl;
			cout << "Average rate = " <<
				(MessageStoreLoadTest::getOps() / tm.getExecutionTime()) << endl;
			break;
		}
		else if (cmd == "stat")
		{
			cout << "Time = " << tm.getExecutionTime() << endl;
			MessageStoreLoadTest::printStatus();
		}
		else if (cmd == "rate")
		{
			cout << "Time = " << tm.getExecutionTime() << endl;
			cout << "Rate = " << MessageStoreLoadTest::getRate()
				<< " messages/second" << endl;
		}
		else
		{
			help = true;
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

