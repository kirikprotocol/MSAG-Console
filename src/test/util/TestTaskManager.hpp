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
	const char* name;
	bool executeFlag;

public:
	TestTask(const char* _name)
		: name(_name), executeFlag(true) {}

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

template<class H>
class TestTaskManager
{
protected:
	smsc::core::threads::ThreadPool tp;
	vector<H*> taskHolders;

public:
	virtual ~TestTaskManager()
	{
		for (int i = 0; i < taskHolders.size(); i++)
		{
			delete taskHolders[i];
		}
	}

	void addTask(H* taskHolder)
	{
		cout << "Starting task " << taskHolders.size() << " ... ";
		tp.startTask(taskHolder->task);
		taskHolders.push_back(taskHolder);
		cout << "Started" << endl;
	}

	void stopTasks()
	{
		for (int i = 0; i < taskHolders.size(); i++)
		{
			taskHolders[i]->stopTask();
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

private:
	bool isStopped() const
	{
		bool stopped = true;
		for (int i = 0; stopped && (i < taskHolders.size()); i++)
		{
			stopped &= taskHolders[i]->stopped;
		}
		return stopped;
	}
};

}
}
}

#endif /* TEST_UTIL_TEST_TASK_MANAGER */
