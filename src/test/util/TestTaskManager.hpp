#ifndef TEST_UTIL_TEST_TASK_MANAGER
#define TEST_UTIL_TEST_TASK_MANAGER

#include "core/threads/ThreadPool.hpp"
#include <iostream>
#include <vector>
#include <sys/timeb.h>

namespace smsc {
namespace test {
namespace util {

using namespace std;

class TestTask : public smsc::core::threads::ThreadedTask
{
private:
	char name[100];
	bool executeFlag;

public:
	TestTask(const char* _name)
		: executeFlag(true)
	{
		strcpy(name, _name);
	}

	TestTask(const char* className, int taskNum)
		: executeFlag(true)
	{
		sprintf(name, "%s_%d", className, taskNum);
	}

	virtual ~TestTask() {}

	virtual void executeCycle() = NULL; //abstract

	virtual void onStopped() = NULL; //abstract
	
	virtual int Execute()
	{
		while (executeFlag)
		{
			executeCycle();
		}
		onStopped();
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
};

template<class T>
class TestTaskManager
{
protected:
	smsc::core::threads::ThreadPool tp;
	vector<T*> tasks;
	timeb timeStart;

public:
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
		sleep(3);
		while (!isStopped())
		{
			cout << "Some tasks are still running. Quit (q) or wait (w) ?: ";
			char ch;
			cin >> ch;
			if (ch == 'q')
			{
				throw exception();
			}
		}
	}

	void startTimer()
	{
		ftime(&timeStart);
	}

	float getExecutionTime()
	{
		timeb curTime;
		ftime(&curTime);
		float dt = (curTime.time - timeStart.time) +
			(curTime.millitm - timeStart.millitm) / 1000.0;
		return dt;
	}

	virtual bool isStopped() const = NULL;
};

}
}
}

#endif /* TEST_UTIL_TEST_TASK_MANAGER */
