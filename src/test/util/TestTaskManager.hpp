#ifndef TEST_UTIL_TEST_TASK_MANAGER
#define TEST_UTIL_TEST_TASK_MANAGER

#include "core/threads/ThreadedTask.hpp"
#include "core/threads/ThreadPool.hpp"
#include <iostream>
#include <vector>

namespace smsc {
namespace test {
namespace util {

using namespace std;

class TestTask : public smsc::core::threads::ThreadedTask
{
private:
	int taskNum;
	bool flag;
	bool stopped;

public:
	TestTask(int _taskNum)
		: taskNum(_taskNum), flag(true), stopped(false) {}

	virtual ~TestTask() {}

	virtual void executeCycle() = 0; //abstract

	virtual int Execute()
	{
		while (flag)
		{
			executeCycle();
		}
		stopped = true;
		return 0;
	}

	const char* taskName()
	{
		char* buf = new char[15];
		sprintf(buf, "Task %d", taskNum);
		return buf;
	}

	void stop()
	{
		flag = false;
	}

	bool isStopped() const
	{
		return stopped;
	}
};

template<class T>
class TestTaskManager
{
protected:
	smsc::core::threads::ThreadPool tp;
	vector<T*> tasks;

public:
	virtual ~TestTaskManager()
	{
	  /*  for (int i = 0; i < tasks.size(); i++)
		{
			delete tasks[i];
		} */
	}

	void addTask(T* task)
	{
		cout << "Starting task " << tasks.size() << " ... ";
		tp.startTask(task);
		tasks.push_back(task);
		cout << "Started" << endl;
	}

	void stopTasks()
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

private:
	bool isStopped() const
	{
		bool stopped = true;
		for (int i = 0; stopped && i < tasks.size(); i++)
		{
			stopped &= tasks[i]->isStopped();
		}
	}
};

}
}
}

#endif /* TEST_UTIL_TEST_TASK_MANAGER */
