#include "util/debug.h"
#include "util/config/Manager.h"
#include "core/synchronization/Event.hpp"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include <iostream>
#include <map>

using namespace std;
using namespace smsc::test::util;
using smsc::sms::SMS;
using smsc::sms::SMSId;
using smsc::util::config::Manager;
using smsc::store::StoreManager;
using smsc::test::store::MessageStoreTestCases;
using smsc::core::synchronization::Event;

/**
 * ������������ ��� ������ ������������ � ������������ ������ ������ Message 
 * Store �� ���������� ��������� ����������.
 * ��������� ��� ���������� � ������������ ���� ����� �� ������ � Message Store 
 * ����� assert ���� ������.
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
	int taskNum;
	MessageStoreTestCases tc; //throws exception

public:
	MessageStoreBusinessCycleTestTask(int taskNum);
	virtual ~MessageStoreBusinessCycleTestTask() {}
	virtual void executeCycle();
	virtual void onStopped();

private:
	void process(const TCResult* res);
};

/**
 * ���� ��������.
 */
class MessageStoreBusinessCycleTestTaskManager
	: public TestTaskManager<MessageStoreBusinessCycleTestTask>
{
public:
	MessageStoreBusinessCycleTestTaskManager() {}
	virtual bool isStopped() const;
};

/**
 * ���������� �� ������.
 */
struct MessageStoreBusinessCycleTestTaskStat
{
	int ops;
	boolean stopped;
	MessageStoreBusinessCycleTestTaskStat()
		: ops(0), stopped(false) {}
};

/**
 * ��������� ������ ����� �����.
 */
class MessageStoreBusinessCycleTest
{
private:
	typedef vector<MessageStoreBusinessCycleTestTaskStat> TaskStatList;
	typedef map<const string, int> TCStatMap;

	static TaskStatList taskStat;
	static TCStatMap tcStat;

public:
	static int delay;
	static bool pause;

public:
	static void resize(int newSize);
	static void onStopped(int taskNum);
	static bool isStopped();
	static void process(int taskNum, const TCResult* res);
	static void printOpsStatByTask();
	static void printOpsStatByTC();
	static int getOps();
};

//MessageStoreBusinessCycleTestTask methods
MessageStoreBusinessCycleTestTask::MessageStoreBusinessCycleTestTask(int _taskNum)
	: TestTask("BusinessCycleTask", _taskNum), taskNum(_taskNum) {}

void MessageStoreBusinessCycleTestTask::executeCycle()
{
	//��������� ���� ����������/��������
	static Event evt;
	if (MessageStoreBusinessCycleTest::pause)
	{
		evt.Wait(1000);
		return;
	}
	if (MessageStoreBusinessCycleTest::delay)
	{
		evt.Wait(MessageStoreBusinessCycleTest::delay);
	}

	SMSId id;
	SMS sms;

	//�������� SM
	process(tc.storeCorrectSM(&id, &sms, RAND_TC));
	
	//���������� ����������� SM � ����������� �������� �� ��� ������������ SM
	//���������� �������������� SM � �������
	//���������� ������������� SM
	//��������� ����������� �������
	//������������ ��������� ������� SM
	//��������� SM
	//������������ ��������� SM
	//������ SM
	for (TCSelector s(RAND_SET_TC, 15); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				{
					SMSId newId;
					SMS newSMS;
					process(tc.storeCorrectSM(&newId, &newSMS, id, sms, RAND_TC));
					process(tc.setCorrectSMStatus(newId, &newSMS, RAND_TC));
					//process(tc.deleteExistentSM(newId));
				}
				break;
			case 2:
				process(tc.storeRejectDuplicateSM(sms));
				break;
			case 3:
				process(tc.storeIncorrectSM(RAND_TC));
				break;
			case 4:
				process(tc.setCorrectSMStatus(id, &sms, RAND_TC));
				break;
			case 5:
				process(tc.setIncorrectSMStatus(id));
				break;
			case 6:
				process(tc.replaceCorrectSM(id, &sms, RAND_TC));
				break;
			case 7:
				process(tc.replaceIncorrectSM(id, sms, RAND_TC));
				break;
			case 8:
				process(tc.replaceIncorrectSM2(id, sms, RAND_TC));
				break;
			default: //9..15
				process(tc.loadExistentSM(id, sms));
		}
	}

	/*
	//�������� SM
	process(tc.deleteExistentSM(id));

	//��������� ������� �������������� SM
	//��������� �������������� SM
	//������ �������������� SM
	//�������� �������������� SM
	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				process(tc.setNonExistentSMStatus(id, RAND_TC));
				break;
			case 2:
				process(tc.replaceNonExistentSM(id, RAND_TC));
				break;
			case 3:
				process(tc.loadNonExistentSM(id, RAND_TC));
				break;
			case 4:
				process(tc.deleteNonExistentSM(id, RAND_TC));
				break;
		}
	}
	*/
}

inline void MessageStoreBusinessCycleTestTask::onStopped()
{
	MessageStoreBusinessCycleTest::onStopped(taskNum);
}

inline void MessageStoreBusinessCycleTestTask::process(const TCResult* res)
{
	MessageStoreBusinessCycleTest::process(taskNum, res);
}

//MessageStoreBusinessCycleTestTaskManager methods
inline bool MessageStoreBusinessCycleTestTaskManager::isStopped() const
{
	return MessageStoreBusinessCycleTest::isStopped();
}

//MessageStoreBusinessCycleTest
bool MessageStoreBusinessCycleTest::pause = false;
int MessageStoreBusinessCycleTest::delay = 0;
MessageStoreBusinessCycleTest::TaskStatList
	MessageStoreBusinessCycleTest::taskStat =
	MessageStoreBusinessCycleTest::TaskStatList();
MessageStoreBusinessCycleTest::TCStatMap
	MessageStoreBusinessCycleTest::tcStat =
	MessageStoreBusinessCycleTest::TCStatMap();
	
inline void MessageStoreBusinessCycleTest::resize(int newSize)
{
	taskStat.resize(newSize);
}

inline void MessageStoreBusinessCycleTest::onStopped(int taskNum)
{
	taskStat[taskNum].stopped = true;
}

inline bool MessageStoreBusinessCycleTest::isStopped()
{
	bool stopped = true;
	for (int i = 0; stopped && (i < taskStat.size()); i++)
	{
		stopped &= taskStat[i].stopped;
	}
	return stopped;
}

void MessageStoreBusinessCycleTest::process(int taskNum, const TCResult* res)
{
	if (res)
	{
		taskStat[taskNum].ops++;
	    tcStat[res->getId()]++;
	    delete res;
	}
}
	
void MessageStoreBusinessCycleTest::printOpsStatByTask()
{
	int totalOps = 0;
	for (int i = 0; i < taskStat.size(); i++)
	{
		int ops = taskStat[i].ops;
		totalOps += ops;
		cout << "BusinessTestTask_" << i << ": ops = " << ops << endl;
	}
	cout << "Total ops = " << totalOps << endl;
	cout << "-----------------------------" << endl;
}

void MessageStoreBusinessCycleTest::printOpsStatByTC()
{
	int totalOps = 0;
	for (TCStatMap::iterator it = tcStat.begin(); it != tcStat.end(); it++)
	{
		const string& tcId = it->first;
		int ops = it->second;
		totalOps += ops;
		cout << tcId << ": ops = " << ops << endl;
	}
	cout << "Total ops = " << totalOps << endl;
	cout << "-----------------------------" << endl;
}

int MessageStoreBusinessCycleTest::getOps()
{
	int totalOps = 0;
	for (int i = 0; i < taskStat.size(); i++)
	{
		totalOps += taskStat[i].ops;
	}
	return totalOps;
}

//test body
void executeBusinessCycleTest(int numThreads)
{
	MessageStoreBusinessCycleTest::resize(numThreads);
	MessageStoreBusinessCycleTestTaskManager tm;
	for (int i = 0; i < numThreads; i++)
	{
		MessageStoreBusinessCycleTestTask* task =
			new MessageStoreBusinessCycleTestTask(i);
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
			cout << "test <pause|resume> - pause/resume test execution" << endl;
			cout << "arc <start|stop> - start/stop archiver" << endl;
			cout << "stat - print statistics" << endl;
			cout << "rtstat - print runtime statistics" << endl;
			cout << "set pool <newSize> - change pool size" << endl;
			cout << "set delay <msec> - slow down test cycle execution" << endl;
			cout << "quit - stop test and quit" << endl;
		}

		//��������� ������
		cin >> cmd;
		if (cmd == "test")
		{
			cin >> cmd;
			if (cmd == "pause")
			{
				MessageStoreBusinessCycleTest::pause = true;
				cout << "Test paused successfully" << endl;
			}
			else if (cmd == "resume")
			{
				MessageStoreBusinessCycleTest::pause = false;
				cout << "Test resumed successfully" << endl;
			}
			else
			{
				help = true;
			}
		}
		else if (cmd == "arc")
		{
			cin >> cmd;
			if (cmd == "start")
			{
				try
				{
					StoreManager::startArchiver();
					cout << "Archiver started successfully" << endl;
				}
				catch (...)
				{
					cout << "Failed to start archiver" << endl;
				}
			}
			else if (cmd == "stop")
			{
				try
				{
					StoreManager::stopArchiver();
					cout << "Archiver stopped successfully" << endl;
				}
				catch (...)
				{
					cout << "Failed to stop archiver" << endl;
				}
			}
			else
			{
				help = true;
			}
		}
		else if (cmd == "stat")
		{
			cout << "Time = " << tm.getExecutionTime() << endl;
			MessageStoreBusinessCycleTest::printOpsStatByTask();
			MessageStoreBusinessCycleTest::printOpsStatByTC();
		}
		else if (cmd == "rtstat")
		{
			cout << "Time = " << tm.getExecutionTime() << endl;
			cout << "Total ops = " <<
				MessageStoreBusinessCycleTest::getOps() << endl;
			cout << "Queue length = " <<
				StoreManager::getPendingQueueLength() << endl;
			cout << "Connection pool size = " <<
				StoreManager::getPoolSize() << endl;
			cout << "Connections count = " <<
				StoreManager::getConnectionsCount() << endl;
			cout << "Busy connections count = " <<
				StoreManager::getBusyConnectionsCount() << endl;
			cout << "Idle connections count = " <<
				StoreManager::getIdleConnectionsCount() << endl;
			cout << "Archiver status = " <<
				(StoreManager::isArchivationInProgress() ? "running" : "idle") 
				<< endl;
		}
		else if (cmd == "set")
		{
			int newVal;
			cin >> cmd;
			cin >> newVal;
			if (cmd == "pool")
			{
				StoreManager::setPoolSize(newVal);
				cout << "Pool size = " << StoreManager::getPoolSize() << endl;
			}
			else if (cmd == "delay")
			{
				MessageStoreBusinessCycleTest::delay = newVal;
			}
			else
			{
				help = true;
			}
		}
		else if (cmd == "quit")
		{
			tm.stopTasks();
			cout << "Total time = " << tm.getExecutionTime() << endl;
			cout << "Total operations = " <<
				MessageStoreBusinessCycleTest::getOps() << endl;
			return;
		}
		else
		{
			help = true;
		}
	}
}

class CoreDumper
{
public:
	~CoreDumper()
	{
		throw "It's fucking core dumper!";
	}
};

//CoreDumper coreDumper;

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

