#ifndef TEST_STORE_MESSAGE_STORE_LOAD_TEST_TASK_MANAGER
#define TEST_STORE_MESSAGE_STORE_LOAD_TEST_TASK_MANAGER

#include "MessageStoreTestCases.hpp"
#include "core/threads/ThreadedTask.hpp"
#include "core/threads/ThreadPool.hpp"
#include <vector>
#include <sys/timeb.h>

namespace smsc  {
namespace test  {
namespace store {

class MessageStoreLoadTestTask : public smsc::core::threads::ThreadedTask
{
public:
	MessageStoreLoadTestTask(int taskNum);
	virtual int Execute();
	virtual const char* taskName();
	void stop();
	bool isStopped();
	int getOps();

private:
	int taskNum;
	int ops;
	bool flag;
	bool stopped;
	MessageStoreTestCases tc; //throws StoreException

};

class MessageStoreLoadTestTaskManager
{
public:
	~MessageStoreLoadTestTaskManager();
	void startTasks(int numTasks);
	void stopTasks() throw (exception);
	float getRate();
	int getOps();

private:
	bool isStopped();

private:
	smsc::core::threads::ThreadPool tp;
	std::vector<MessageStoreLoadTestTask*> tasks;
	timeb t1;
	int ops1;
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_LOAD_TEST_TASK_MANAGER */
