#ifndef TEST_UTIL_TEST_TASK_MANAGER
#define TEST_UTIL_TEST_TASK_MANAGER

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
	char* name;
	bool executeFlag;
	bool stopped;

public:
	TestTask(int taskNum)
		: executeFlag(true), stopped(false)
	{
		name = new char[15];
		sprintf(name, "Task_%d", taskNum);
	}

	virtual ~TestTask()
	{
		delete[] name;
	}

	virtual void executeCycle() = NULL; //abstract

	virtual int Execute()
	{
		while (executeFlag)
		{
			executeCycle();
		}
		stopped = true;
		return 0;
	}

	virtual const char* taskName()
	{
		return name;
	}

	void stop()
	{
		executeFlag = false;
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
		sleep(5);
		while (!isStopped())
		{
			cout << "Some tasks are still running. Stop (s) or wait (w) ?: ";
			char ch;
			cin >> ch;
			if (ch == 's')
			{
				throw exception();
			}
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
