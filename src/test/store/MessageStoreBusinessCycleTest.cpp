#include "util/debug.h"
#include "util/config/Manager.h"
#include "core/synchronization/Event.hpp"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include <iostream>
#include <sstream>
#include <map>

using namespace std;
using namespace smsc::test::util;
using log4cpp::Category;
using smsc::util::Logger;
using smsc::sms::SMS;
using smsc::sms::SMSId;
using smsc::util::config::Manager;
using smsc::store::StoreManager;
using smsc::test::store::MessageStoreTestCases;
using smsc::core::synchronization::Event;

/**
 * Предназначен для стресс тестирования и тестирования бизнес циклов Message 
 * Store на длительных временных интервалах.
 * Выполняет все корректные и некорректные тест кейсы по работе с Message Store 
 * кроме assert тест кейсов.
 * Результат выполнения теста может быть следующий:
 * <ul>
 * <li>Обнаружение проблем в multi-thread окружении.
 * <li>Восстановление коннектов при проблемах сети и т.п.
 * <li>Тестирование на фрагментацию памяти.
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
 * Таск менеджер.
 */
class MessageStoreBusinessCycleTestTaskManager
	: public TestTaskManager<MessageStoreBusinessCycleTestTask>
{
public:
	MessageStoreBusinessCycleTestTaskManager() {}
	virtual bool isStopped() const;
};

/**
 * Статистика по таскам.
 */
struct MessageStoreBusinessCycleTestTaskStat
{
	int ops;
	boolean stopped;
	MessageStoreBusinessCycleTestTaskStat()
		: ops(0), stopped(false) {}
};

/**
 * Хранилище данных всего теста.
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

#define PREPARE_FOR_NEW_SMS \
	id.push_back(new SMSId()); \
	sms.push_back(new SMS());

void MessageStoreBusinessCycleTestTask::executeCycle()
{
	//проверить тест остановлен/замедлен
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

	vector<SMSId*> id;
	vector<SMS*> sms;

	//Сохранение правильного sms (только одно sms создаем в цикле)
	PREPARE_FOR_NEW_SMS
	process(tc.storeCorrectSms(id.back(), sms.back(), RAND_TC));
	
	//Сохранение правильного sms с параметрами похожими на уже существующий sms
	//Сохранение дублированного sms
	//Сохранение дублированного sms с отказом
	//Сохранение корректного sms с замещением уже существующего
	//Сохранение неправильного sms
	//Обновление статуса sms в состоянии ENROUTE
	//Корректное обновление существующего sms
	//Некорректное обновление существующего sms
	//Чтение существующего sms
	bool duplicatesOk = rand0(1); //взаимоисключающие тест кейсы
	for (TCSelector s(RAND_SET_TC, 15); s.check(); s++)
	{
		switch (s.value())
		{
			case 1: //только одно sms создаем в цикле
				PREPARE_FOR_NEW_SMS
				process(tc.storeCorrectSms(id.back(), sms.back(),
					*id[0], *sms[0], RAND_TC));
				break;
			case 2:
				if (duplicatesOk)
				{
					PREPARE_FOR_NEW_SMS
					process(tc.storeDuplicateSms(id.back(), sms.back(),
						*id[0], *sms[0]));
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					process(tc.storeRejectDuplicateSms(*sms[i]));
				}
				break;
			case 4:
				for (int i = 0; !duplicatesOk && i < id.size(); i++)
				{
					process(tc.storeReplaceCorrectSms(*id[i], sms[i]));
				}
				break;
			case 5:
				process(tc.storeIncorrectSms(RAND_TC));
				break;
			case 6:
				for (int i = 0; i < id.size(); i++)
				{
					process(tc.changeExistentSmsStateEnrouteToEnroute(*id[i],
						sms[i], RAND_TC));
				}
				break;
			case 7:
				for (int i = 0; i < id.size(); i++)
				{
					process(tc.replaceCorrectSms(*id[i], sms[i], RAND_TC));
				}
				break;
			case 8:
				for (int i = 0; i < id.size(); i++)
				{
					process(tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC));
				}
				break;
			default: //9..15
				for (int i = 0; i < id.size(); i++)
				{
					process(tc.loadExistentSms(*id[i], *sms[i]));
				}
		}
	}

	//Перевод sms из ENROUTE в финальное состояние
	for (int i = 0; i < id.size(); i++)
	{
		process(tc.changeExistentSmsStateEnrouteToFinal(*id[i], sms[i], RAND_TC));
	}
	
	//Сохранение sms с замещением существующего sms финальном состоянии
	//Перевод sms в финальном состоянии в любое другое состояние
	//Обновление sms в финальном состоянии
	//Чтение существующего sms
	for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
	{
		switch (s.value())
		{
			case 1: //только одно sms создаем в цикле
				PREPARE_FOR_NEW_SMS
				process(tc.storeReplaceSmsInFinalState(id.back(), sms.back(),
					*id[0], *sms[0]));
				break;
			case 2:
				for (int i = 0; i < id.size(); i++)
				{
					process(tc.changeFinalSmsStateToAny(*id[i], RAND_TC));
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					process(tc.replaceFinalSms(*id[i], *sms[i]));
				}
				break;
			default: //4..5
				for (int i = 0; i < id.size(); i++)
				{
					process(tc.loadExistentSms(*id[i], *sms[i]));
				}
		}
	}

	//Перевод несуществующего sms в любое другое состояние
	//Некорректное обновление несуществующего sms
	//Чтение несуществующего sms
	SMSId badId = 0xFFFFFFFFFFFFFFFF;
	for (TCSelector s(RAND_SET_TC, 3); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				process(tc.changeFinalSmsStateToAny(badId, RAND_TC));
				break;
			case 2:
				process(tc.replaceIncorrectSms(badId, *sms[0], RAND_TC));
				//process(tc.replaceFinalSms(badId, *sms[0]);
				break;
			case 3:
				process(tc.loadNonExistentSms(badId));
				break;
		}
	}

	//очистка памяти
	for (int i = 0; i < id.size(); i++)
	{
		delete id[i];
		delete sms[i];
	}
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
	static Category& log = Logger::getCategory("smsc.test.store.BusinessCycleTest");
	if (res)
	{
		//вывод в лог
		ostringstream os;
		os << *res << endl;
		log.debug("Task = %d, tc = %s", taskNum, os.str().c_str());
		//обновить статистику
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
		//хелп
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

		//обработка команд
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
				StoreManager::isArchiverStarted() << endl;
			cout << "Archivation status = " <<
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

