#include <stdio.h>
#include <signal.h>
#include <string>
#include "logger/Logger.h"
#include "core/threads/Thread.hpp"


bool stopped=false;

class TestThread:public smsc::core::threads::Thread
{
  smsc::logger::Logger *log;
public:
  TestThread(smsc::logger::Logger* l):log(l){}
  int Execute()
  {
    int cnt1=0,cnt2=0,cnt3=0;
    std::string s;
    while(!stopped)
    {
      log->info("%d,%d,%d:%s",cnt1,cnt2,cnt3,s.c_str());
      cnt1++;
      cnt2+=cnt1;
      cnt3+=cnt2;
      s+='!';
      if(s.length()>10)s.erase(0,5);
    }
  }
};

void sigintdisp(int)
{
  stopped=true;
}

int main(int argc,char* argv[])
{
  signal(SIGINT,sigintdisp);
  smsc::logger::Logger::Init("log4cpp.properties");
  const int N=2;
  TestThread* tt[N];
  for(int i=0;i<N;i++)
  {
    char buf[64];
    sprintf(buf,"test.log.%d",i);
    tt[i]=new TestThread(new smsc::logger::Logger(smsc::logger::Logger::getInstance(buf)));
    tt[i]->Start();
  }
  for(int i=0;i<N;i++)
  {
    tt[i]->WaitFor();
    delete tt[i];
  }
  return 0;
}
