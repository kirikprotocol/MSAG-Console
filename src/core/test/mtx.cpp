#include <stdio.h>
#include "core/threads/Thread.hpp"
#include "core/synchronization/Mutex.hpp"
#include <unistd.h>

using namespace smsc::core::threads;
using namespace smsc::core::synchronization;

class A{
Mutex m;
public:
  void aaa(int n)
  {
    m.Lock();
    printf("%d\n",n);
    sleep(1);
    m.Unlock();
  }
};

A a;

class MyThread: public Thread{
  int num;
public:
  MyThread(int n):num(n){}
  virtual int Execute()
  {
    for(;;)
    {
      a.aaa(num);
    }
  }
};

int main()
{
  const int n=50;
  MyThread *t[n];
  for(int i=0;i<n;i++)
  {
    t[i]=new MyThread(i);
    t[i]->Start();
  }
  t[0]->WaitFor();
}
