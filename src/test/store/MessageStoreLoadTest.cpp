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
 * ������������ ��� ��������� ������������������ Message Store.
 * ��������� ������ ���������� �������� �� ������ � Message Store.
 * ��������� ���������� ����� ����������� 2-�� �������:
 * <ul>
 * <li>���������� �������� � �������.
 * <li>������ ����� ��������� � �� �������.
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
 * ���� ��������.
 */
class MessageStoreLoadTestTaskManager
	: public TestTaskManager<MessageStoreLoadTestTask>
{
public:
	MessageStoreLoadTestTaskManager() {}
	virtual bool isStopped() const;
};

/**
 * ���������� �� ������.
 */
struct MessageStoreLoadTestTaskStat
{
	int ops;
	boolean stopped;
	MessageStoreLoadTestTaskStat()
		: ops(0), stopped(false) {}
};

/**
 * ��������� ���������� ����� �����.
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
	static void doStat(int taskNum, const TCResult* res);
	static void printStatus();
	static float getRate();
	static int getOps();
};

//MessageStoreLoadTestTask methods
MessageStoreLoadTestTask::MessageStoreLoadTestTask(int _taskNum)
	: TestTask("LoadTask", _taskNum), taskNum(_taskNum) {}

void MessageStoreLoadTestTask::executeCycle()
{
	for (int i = 0; i < 20; i++)
	{
		SMSId id;
		SMS sms;
		doStat(tc.storeCorrectSms(&id, &sms, RAND_TC));
		doStat(tc.changeExistentSmsStateEnrouteToFinal(id, &sms, RAND_TC));
	}
}

inline void MessageStoreLoadTestTask::onStopped()
{
	MessageStoreLoadTest::onStopped(taskNum);
}

inline void MessageStoreLoadTestTask::doStat(const TCResult* res)
{
	MessageStoreLoadTest::doStat(taskNum, res);
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

inline void MessageStoreLoadTest::doStat(int taskNum, const TCResult* res)
{
	if (res && strcmp(res->getId(), TC_STORE_CORRECT_SMS) == 0)
	{
		taskStat[taskNum].ops++;
	}
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
	for (int i = 0; i < numThreads; i++)
	{
		MessageStoreLoadTestTask* task =
			new MessageStoreLoadTestTask(i);
		tm.addTask(task);
	}
	tm.startTimer();

	string cmd;
	bool help = true;
	while (true)
	{
		//����
		if (help)
		{
			help = false;
			cout << "'rate' - print rate" << endl;
			cout << "'stat' - print statistics" << endl;
			cout << "'quit' - quit" << endl;
		}

		//��������� ������
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

