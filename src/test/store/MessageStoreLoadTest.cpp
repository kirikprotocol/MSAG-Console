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
	int ops;
	MessageStoreTestCases tc; //throws exception

public:
	MessageStoreLoadTestTask(int taskNum) : TestTask(taskNum), ops(0) {};

	virtual void executeCycle()
	{
		SMSId id;
		SMS sms;
		for (int maxOps = 200 * (2 + ops / 200); ops < maxOps; ops++)
		{
			delete tc.storeCorrectSM(&id, &sms, RAND_TC);
			//tc.setCorrectSMStatus();
			//tc.createBillingRecord();
		}
		//tc.updateCorrectExistentSM(); ops++;
		//tc.deleteExistentSM(); ops++;
		//tc.loadExistentSM(); ops++;
	}

	int getOps() const
	{
		return ops;
	}

	virtual ~MessageStoreLoadTestTask() {}
};

class MessageStoreLoadTestTaskManager
	: public TestTaskManager<MessageStoreLoadTestTask>
{
private:
	timeb t1;
	int ops1;

public:
	void startTimer()
	{
		ftime(&t1);
		ops1 = getOps();
	}

	float getRate()
	{
		int ops2 = getOps();
		timeb t2; ftime(&t2);
		float dt = (t2.time - t1.time) + (t2.millitm - t1.millitm) / 1000.0;
		float rate = (ops2 - ops1) / dt;
		t1 = t2;
		ops1 = ops2;
		return rate;
	}

	int getOps() const
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
		cout << "Usage: MessageStoreLoadTest <numThreads>" << endl;
		exit(0);
	}
	
	//запустить таски
	const int numThreads = atoi(argv[1]);
	MessageStoreLoadTestTaskManager tm;
	for (int i = 0; i < numThreads; i++)
	{
		tm.addTask(new MessageStoreLoadTestTask(i));
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
				exit(0);
				break;
			default:
				cout << "Rate = " << tm.getRate()
					<< " messages/second" << endl;
		}
	}
	return 0;
}

