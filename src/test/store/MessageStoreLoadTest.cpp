#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "test/sms/SmsUtil.hpp"
#include "test/util/Util.hpp"
#include "test/util/TestTaskManager.hpp"
#include <iostream>
#include <sys/timeb.h>

using namespace std;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace smsc::store;
using namespace smsc::test::util;
using namespace smsc::test::sms;

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
	MessageStore* msgStore;
	static const int numSms = 10;
	SMS sms[numSms];
	Descriptor dst;
	int num;

public:
	MessageStoreLoadTestTask(MessageStore* msgStore, int taskNum);
	virtual ~MessageStoreLoadTestTask() {}
	virtual void executeCycle();
	virtual void onStopped();
	void updateStat();
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
	static void updateStat(int taskNum);
	static void printStatus();
	static float getRate();
	static int getOps();
};

//MessageStoreLoadTestTask methods
MessageStoreLoadTestTask::MessageStoreLoadTestTask(MessageStore* _msgStore,
	int _taskNum) : TestTask("LoadTask", _taskNum), msgStore(_msgStore),
	taskNum(_taskNum), num(0)
{
	__require__(msgStore);
	//����������� sms-��
	//��������� ������ �� SMPP_MESSAGE_PAYLOAD
	static const uint64_t mask = BODY_ALL ^ BODY_MSG_PAYLOAD;
	for (int i = 0; i < numSms; i++)
	{
		SmsUtil::setupRandomCorrectSms(&sms[i], mask);
	}
	SmsUtil::setupRandomCorrectDescriptor(&dst);
	dst.setSmeNumber(rand0(65535));
}

void MessageStoreLoadTestTask::executeCycle()
{
	try
	{
		num = (num + 1) % numSms;
		//create
		SMSId smsId = msgStore->getNextId();
		msgStore->createSms(sms[num], smsId, CREATE_NEW);
		//load
		SMS tmp;
		msgStore->retriveSms(smsId, tmp);
		//change state
		msgStore->changeSmsStateToDelivered(smsId, dst);
		//stat
		updateStat();
	}
	catch (...)
	{
		__warning__("Message store error");
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
	for (int i = 0; i < numThreads; i++)
	{
		MessageStoreLoadTestTask* task =
			new MessageStoreLoadTestTask(StoreManager::getMessageStore(), i);
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
			cout << "rate - print rate" << endl;
			cout << "stat - print statistics" << endl;
			cout << "quit - quit" << endl;
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

