#include "util/debug.h"
#include "util/config/Manager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include <iostream>
#include <sys/timeb.h>

using namespace std;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace smsc::test::store;
using namespace smsc::test::util;

/**
 * Предназначен для стресс тестирования и тестирования бизнес циклов Message 
 * Store на длительных временных интервалах.
 * Выполняет все корректные и некорректные test cases по работе с Message Store 
 * кроме assert test cases.
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
	MessageStoreTestCases tc; //throws exception
	int ops;

public:
	MessageStoreBusinessCycleTestTask(int taskNum) : TestTask(taskNum), ops(0) {}

	virtual void executeCycle()
	{
		//создаю SM для попытки дальнейшего создание дублированного SM
		SMSId correctId;
		SMS correctSM;
		delete tc.storeCorrectSM(&correctId, &correctSM, RAND_TC); ops++;
		
		//создаю SM и сразу читаю
		for (int i = 0; i < 10; i++)
		{
			SMSId id;
			SMS sms;
			delete tc.storeCorrectSM(&id, &sms, RAND_TC); ops++;
			delete tc.loadExistentSM(id, sms); ops++;
		}
		
		//создаю и загружаю кривые SM
		delete tc.storeIncorrectSM(correctSM, ALL_TC); ops++;
		delete tc.loadExistentSM(correctId, correctSM); ops++;
		
		//создаю список не читая, а потом буду читать этот список
		//список большой специально для того, чтобы было большое количество 
		//одинаковых последовательных операций и вероятность ошибочного 
		//использования shared buffers возросла (если в коде есть подобные 
		//ошибки)
		const int listSize = 100;
		for (int j = 0; j < 5; j++)
		{
			SMSId id[listSize];
			SMS sms[listSize];
			for (int i = 0; i < listSize; i++)
			{
				delete tc.storeCorrectSM(&id[i], &sms[i], RAND_TC); ops++;
			}
			for (int i = 0; i < listSize; i++)
			{
				delete tc.loadExistentSM(id[i], sms[i]); ops++;
			}
		}
		delete tc.loadNonExistentSM(); ops++;
		//tc.setCorrectSMStatus(); ops++;
		//tc.createBillingRecord(); ops++;
		//tc.updateCorrectExistentSM(); ops++;
		//tc.deleteExistentSM(); ops++;
	}

	int getOps() const
	{
		return ops;
	}

	virtual ~MessageStoreBusinessCycleTestTask() {}
};

class MessageStoreBusinessCycleTestTaskManager
	: public TestTaskManager<MessageStoreBusinessCycleTestTask>
{
private:
	timeb t1;

public:
	MessageStoreBusinessCycleTestTaskManager()
	{
		ftime(&t1);
	}

	int getOps()
	{
		int ops = 0;
		for (int i = 0; i < tasks.size(); i++)
		{
			ops += tasks[i]->getOps();
		}
		return ops;
	}
};

int main(int argc, char* argv[])
{
    Manager::init("config.xml");
	if (argc != 2)
	{
		cout << "Usage: MessageStoreBusinessCycleTest <numThreads>" << endl;
		exit(0);
	}
	
	//запустить таски
	const int numThreads = atoi(argv[1]);
	MessageStoreBusinessCycleTestTaskManager tm;
	for (int i = 0; i < numThreads; i++)
	{
		tm.addTask(new MessageStoreBusinessCycleTestTask(i));
	}

	while (true)
	{
		char ch;
		cin >> ch;
		switch (ch)
		{
			case 'q':
				tm.stopTasks();
				cout << "Total operations = " << tm.getOps() << endl;
				exit(0);
				break;
			default:
				cout << "Total operations = " << tm.getOps() << endl;
		}
	}
	return 0;
}

