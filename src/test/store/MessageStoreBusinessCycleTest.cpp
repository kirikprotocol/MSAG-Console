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
 * “аск менеджер.
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
 * —татистика по таскам.
 */
struct MessageStoreBusinessCycleTestTaskStat
{
	int ops;
	boolean stopped;
	MessageStoreBusinessCycleTestTaskStat()
		: ops(0), stopped(false) {}
};

/**
 * ’ранилище статистики всего теста.
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
	//создаю SM дл€ попытки дальнейшего создание дублированного SM
	SMSId correctId;
	SMS correctSM;
	doStat(tc.storeCorrectSM(&correctId, &correctSM, RAND_TC));
	doStat(tc.storeIncorrectSM(correctSM, ALL_TC));

	//создаю SM, сразу читаю и удал€ю
	for (int i = 0; i < 10; i++)
	{
		SMSId id;
		SMS sms;
		doStat(tc.storeCorrectSM(&id, &sms, RAND_TC));
		doStat(tc.loadExistentSM(id, sms));
		doStat(tc.deleteExistentSM(id));
		doStat(tc.deleteNonExistentSM(id, RAND_TC));
		doStat(tc.loadNonExistentSM(id, RAND_TC));
	}

	//создаю и удал€ю кривые SM
	doStat(tc.storeIncorrectSM(correctSM, ALL_TC));
	doStat(tc.deleteExistentSM(correctId));

	//сначала создаю список, потом читаю этот список, потом удал€ю
	//список большой специально дл€ того, чтобы было большое количество 
	//одинаковых последовательных операций и веро€тность ошибочного 
	//использовани€ shared buffers возросла (если в коде есть подобные 
	//ошибки)
	const int listSize = 20;
	for (int j = 0; j < 3; j++)
	{
		SMSId id[listSize];
		SMS sms[listSize];
		for (int i = 0; i < listSize; i++)
		{
			doStat(tc.storeCorrectSM(&id[i], &sms[i], RAND_TC));
		}
		for (int i = 0; i < listSize; i++)
		{
			doStat(tc.loadExistentSM(id[i], sms[i]));
		}
		for (int i = 0; i < listSize; i++)
		{
			doStat(tc.deleteExistentSM(id[i]));
		}
		doStat(tc.loadNonExistentSM(id[0], RAND_TC));
		doStat(tc.deleteNonExistentSM(id[0], RAND_TC));
		doStat(tc.loadNonExistentSM(id[listSize - 1], RAND_TC));
		doStat(tc.deleteNonExistentSM(id[listSize - 1], RAND_TC));
	}
	//tc.setCorrectSMStatus(); ops++;
	//tc.createBillingRecord(); ops++;
	//tc.updateCorrectExistentSM(); ops++;
}

inline void MessageStoreBusinessCycleTestTask::onStopped()
{
	MessageStoreBusinessCycleTestStat::taskStat[taskNum].stopped = true;
}

void MessageStoreBusinessCycleTestTask::doStat(const TCResult* res)
{
	(MessageStoreBusinessCycleTestStat::taskStat[taskNum].ops)++;
	(MessageStoreBusinessCycleTestStat::tcStat[res->getId()])++;
	delete res;
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
				tm.printOpsStatByTask();
				tm.printOpsStatByTC();
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

