#include "MessageStoreLoadTestTaskManager.hpp"
#include <iostream>
#include <unistd.h>

namespace smsc  {
namespace test  {
namespace store {

using namespace std;
using namespace smsc::sms;
using namespace smsc::core::threads;
using namespace smsc::test::util;

MessageStoreLoadTestTask::MessageStoreLoadTestTask(int _taskNum)
{
	taskNum = _taskNum;
	ops = 0;
	flag = true;
	stopped = false;
}

int MessageStoreLoadTestTask::Execute()
{
	while (flag)
	{
		SMSId id;
		SMS sms;
		ops++;

		for (int maxOps = 200 + ops / 200; ops < maxOps; ops++)
		{
			delete tc.storeCorrectSM(&id, &sms, RAND_TC);
			//tc.setCorrectSMStatus();
			//tc.createBillingRecord();
		}
		//tc.updateCorrectExistentSM(); ops++;
		//tc.deleteExistentSM(); ops++;
		//tc.loadExistentSM(); ops++;
	}
	stopped = true;
	return 0;
}

const char* MessageStoreLoadTestTask::taskName()
{
	return "";
}

void MessageStoreLoadTestTask::stop()
{
	flag = false;
}

bool MessageStoreLoadTestTask::isStopped()
{
	return stopped;
}

int MessageStoreLoadTestTask::getOps()
{
	return ops;
}

MessageStoreLoadTestTaskManager::~MessageStoreLoadTestTaskManager()
{
	for (int i = 0; i < tasks.size(); i++)
	{
		delete tasks[i];
	}
}

void MessageStoreLoadTestTaskManager::startTasks(int numTasks)
{
	tp.preCreateThreads(numTasks);
	sleep(1);
	for (int i = 0; i < numTasks; i++)
	{
		cout << "Starting task " << i << " ... ";
		try
		{
			tasks.push_back(new MessageStoreLoadTestTask(i));
		}
		catch(exception& e)
		{
			cout << "Failed" << endl;
			cout << e.what() << endl;
			exit(-1);
		}
		tp.startTask(tasks[i]);
		cout << "Started" << endl;
	}
	ops1 = 0;
	ftime(&t1);
}

void MessageStoreLoadTestTaskManager::stopTasks() throw (exception)
{
	for (int i = 0; i < tasks.size(); i++)
	{
		tasks[i]->stop();
	}
	sleep(1);
	for (int i = 0; !isStopped() && i < 10; i++)
	{
		sleep(1);
	}
	if (!isStopped())
	{
		throw exception();
	}
}

bool MessageStoreLoadTestTaskManager::isStopped()
{
	bool stopped = true;
	for (int i = 0; stopped && i < tasks.size(); i++)
	{
		stopped &= tasks[i]->isStopped();
	}
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

int MessageStoreLoadTestTaskManager::getOps()
{
	int ops = 0;
	for (int i = 0; i < tasks.size(); i++)
	{
		ops += tasks[i]->getOps();
	}
	return ops;
}

}
}
}

