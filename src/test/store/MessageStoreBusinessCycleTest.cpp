#include "util/debug.h"
#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include <iostream>
#include <map>

using namespace std;
using namespace smsc::sms;
using namespace smsc::store;
using namespace smsc::util::config;
using namespace smsc::test::store;
using namespace smsc::test::util;

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
	void doStat(const TCResult* res);
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
	void printOpsStatByTask();
	void printOpsStatByTC();
	int getOps();
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
 * ��������� ���������� ����� �����.
 */
class MessageStoreBusinessCycleTestStat
{
public:
	typedef vector<MessageStoreBusinessCycleTestTaskStat> TaskStatList;
	typedef map<const string, int> TCStatMap;

	static TaskStatList taskStat;
	static TCStatMap tcStat;
};

//MessageStoreBusinessCycleTestTask methods
MessageStoreBusinessCycleTestTask::MessageStoreBusinessCycleTestTask(int _taskNum)
	: TestTask("BusinessCycleTask", _taskNum), taskNum(_taskNum) {}

void MessageStoreBusinessCycleTestTask::executeCycle()
{
	SMSId id;
	SMS sms;

	//�������� SM
	doStat(tc.storeCorrectSM(&id, &sms, RAND_TC));
	
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
					doStat(tc.storeCorrectSM(&newId, &newSMS, id, sms, RAND_TC));
					doStat(tc.deleteExistentSM(newId));
				}
				break;
			case 2:
				doStat(tc.storeRejectDuplicateSM(sms));
				break;
			case 3:
				doStat(tc.storeIncorrectSM(RAND_TC));
				break;
			case 4:
				doStat(tc.setCorrectSMStatus(id, &sms, RAND_TC));
				break;
			case 5:
				doStat(tc.setIncorrectSMStatus(id));
				break;
			case 6:
				doStat(tc.replaceCorrectSM(id, &sms, RAND_TC));
				break;
			case 7:
				doStat(tc.replaceIncorrectSM(id, sms, RAND_TC));
				break;
			case 8:
				doStat(tc.replaceIncorrectSM2(id, sms, RAND_TC));
				break;
			default: //9..15
				doStat(tc.loadExistentSM(id, sms));
		}
	}

	//�������� SM
	doStat(tc.deleteExistentSM(id));

	//��������� ������� �������������� SM
	//��������� �������������� SM
	//������ �������������� SM
	//�������� �������������� SM
	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				doStat(tc.setNonExistentSMStatus(id, RAND_TC));
				break;
			case 2:
				doStat(tc.replaceNonExistentSM(id, RAND_TC));
				break;
			case 3:
				doStat(tc.loadNonExistentSM(id, RAND_TC));
				break;
			case 4:
				doStat(tc.deleteNonExistentSM(id, RAND_TC));
				break;
		}
	}
}

inline void MessageStoreBusinessCycleTestTask::onStopped()
{
	MessageStoreBusinessCycleTestStat::taskStat[taskNum].stopped = true;
}

void MessageStoreBusinessCycleTestTask::doStat(const TCResult* res)
{
	if (res)
	{
		(MessageStoreBusinessCycleTestStat::taskStat[taskNum].ops)++;
	    (MessageStoreBusinessCycleTestStat::tcStat[res->getId()])++;
	    delete res;
	}
}

//MessageStoreBusinessCycleTestTaskManager methods
bool MessageStoreBusinessCycleTestTaskManager::isStopped() const
{
	MessageStoreBusinessCycleTestStat::TaskStatList& taskStat = 
		MessageStoreBusinessCycleTestStat::taskStat;
	bool stopped = true;
	for (int i = 0; stopped && (i < taskStat.size()); i++)
	{
		stopped &= taskStat[i].stopped;
	}
	return stopped;
}

void MessageStoreBusinessCycleTestTaskManager::printOpsStatByTask()
{
	MessageStoreBusinessCycleTestStat::TaskStatList& taskStat = 
		MessageStoreBusinessCycleTestStat::taskStat;
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

void MessageStoreBusinessCycleTestTaskManager::printOpsStatByTC()
{
	MessageStoreBusinessCycleTestStat::TCStatMap& tcStat =
		MessageStoreBusinessCycleTestStat::tcStat;
	int totalOps = 0;
	for (MessageStoreBusinessCycleTestStat::TCStatMap::iterator it =
		 tcStat.begin(); it != tcStat.end(); it++)
	{
		const string& tcId = it->first;
		int ops = it->second;
		totalOps += ops;
		cout << tcId << ": ops = " << ops << endl;
	}
	cout << "Total ops = " << totalOps << endl;
	cout << "-----------------------------" << endl;
}

int MessageStoreBusinessCycleTestTaskManager::getOps()
{
	MessageStoreBusinessCycleTestStat::TaskStatList& taskStat = 
		MessageStoreBusinessCycleTestStat::taskStat;
	int totalOps = 0;
	for (int i = 0; i < taskStat.size(); i++)
	{
		totalOps += taskStat[i].ops;
	}
	return totalOps;
}

//MessageStoreBusinessCycleTestStat
MessageStoreBusinessCycleTestStat::TaskStatList
	MessageStoreBusinessCycleTestStat::taskStat =
	MessageStoreBusinessCycleTestStat::TaskStatList();
MessageStoreBusinessCycleTestStat::TCStatMap
	MessageStoreBusinessCycleTestStat::tcStat =
	MessageStoreBusinessCycleTestStat::TCStatMap();

//test body
void executeBusinessCycleTest(int numThreads)
{
	MessageStoreBusinessCycleTestStat::taskStat.resize(numThreads);
	MessageStoreBusinessCycleTestTaskManager tm;
	for (int i = 0; i < numThreads; i++)
	{
		MessageStoreBusinessCycleTestTask* task =
			new MessageStoreBusinessCycleTestTask(i);
		tm.addTask(task);
	}
	tm.startTimer();

	char ch = '*';
	while (true)
	{
		switch (ch)
		{
			case 'q':
				tm.stopTasks();
				cout << "Total time = " << tm.getExecutionTime() << endl;
				cout << "Total operations = " << tm.getOps() << endl;
				return;
			case 's':
				cout << "Time = " << tm.getExecutionTime() << endl;
				tm.printOpsStatByTask();
				tm.printOpsStatByTC();
				break;
			case 'p':
				cout << "Current pool size = " <<
					StoreManager::getPoolSize() << endl;
				cout << "Enter new pool size: ";
				unsigned newSize;
				cin >> newSize;
				StoreManager::setPoolSize(newSize);
				break;
			case 'a':
				cout << "Time = " << tm.getExecutionTime() << endl;
				cout << "Total ops = " << tm.getOps() << endl;
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
				break;
			default:
				cout << "'a' - show totals" << endl;
				cout << "'s' - show statistics" << endl;
				cout << "'p' - change pool settings" << endl;
				cout << "'q' - quit" << endl;
		}
		cin >> ch;
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

