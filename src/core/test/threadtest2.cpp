#include "core/threads/ThreadPool.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

class MyTask:public smsc::core::threads::ThreadedTask{
public:
  virtual int Execute()
  {
    return Work();
  }
  virtual const char* taskName()
  {
    return "mytask";
  }
  virtual int Work()=0;

};

template <class T>
class TaskManager{
vector<T*> tsk;
smsc::core::threads::ThreadPool tp;
public:
  ~TaskManager(){}
  void addTask(T* t)
  {
    tsk.push_back(t);
    tp.startTask(t);

  }
};

class OneMoreTask: public MyTask{
int n;
public:
  OneMoreTask(int x):n(x){}
  virtual int Work()
  {
    printf("do some work!\n");
    return 0;
  }
};

class MyManager:public TaskManager<OneMoreTask>{
int a;
public:
  MyManager(int x):a(x){}
};

int main()
{
  MyManager m(1);
  m.addTask(new OneMoreTask(0));
  return 0;
}
