#include "Sender.h"
#include "StatisticsManager.h"
#include <stdlib.h>
#include <unistd.h>

namespace scag {
namespace stat {

using smsc::core::threads::Thread;

class Registrator : public Thread {
public:
	Registrator() : sm(0)
    {}
	virtual ~Registrator()
    {}
    virtual int Execute();
    void Stop() {isStopping = true; WaitFor();};
    void Start() {isStopping = false; Thread::Start();};
    void init(StatisticsManager * sm_) { sm = sm_;};
    void InitServer(std::string perfHost, int genPort, int svcPort, int scPort);
protected:
	StatisticsManager * sm;
    bool isStopping;
};

int Registrator::Execute()
{
    using namespace Counters;

    int counter = 0;
    int httpCounter = 0;
    int providerId = 1;

    while(!isStopping)
    {
        SmppStatEvent si;
        HttpStatEvent hs;

        if(++providerId == 5) providerId = 1;

        //=============== sme1 =====================
        if(++counter == 5) counter = 0;
        int count =  7. * ( (double)random() / 2147483648. ) + 1;
        for(int i = 0; i<= count - 1; i++){
            strcpy(si.smeId, "sme1");
            strcpy(si.routeId, "route1");
            si.smeProviderId = providerId;
            si.routeProviderId = 1;
            si.counter = counter;
            si.internal = false;
            si.errCode = 1;
            sm->registerEvent(si);
        }
        //==========================================

        count =  7. * ( (double)random() / 2147483648. ) + 1;
        for(int i = 0; i <= count - 1; i++){
            strcpy(si.smeId, "sme2");
            strcpy(si.routeId, "route3");
            si.smeProviderId = providerId;
            si.routeProviderId = 3;
            si.counter = counter;
            si.internal = true;
            si.errCode = 1;
            sm->registerEvent(si);
        }

        //=================== http1 ====================
        if(++httpCounter == 6) httpCounter = 0;
        hs.routeId = "route1";
        hs.serviceId = "http1";
        hs.serviceProviderId = providerId;
        hs.counter = httpCounter;
        hs.errCode = 1;
        count =  7. * ( (double)random() / 2147483648. ) + 1;
        for(int i = 0; i<= count - 1; i++){            
            sm->registerEvent(hs);
        }
        //==============================================

        

        useconds_t pause = 50000. * ( (double)random() / 2147483648. );
        if(pause < 5000 ) pause = 5000; 
        //printf("pause: %d sec.\n", pause);
        usleep(pause);
    }
    return 1;
}

Sender::Sender()
	: perfListener(0), isStopping(false)
{
}

Sender::~Sender()
{
}

int Sender::Execute()
{
    uint64_t cnt,last=0;
    timespec now={0,0},lasttime={0,0};
    double ut,tm,rate,avg;
    if(start.tv_sec==0)
      clock_gettime(CLOCK_REALTIME,&start);
    Event ev;
    timeshift=0;
    time_t perfStart=start.tv_sec;
    for(int i=0;i<60;i++)times[i]=start.tv_sec;
    int lastscnt=0;
    memset(smppPerfCnt,0,sizeof(smppPerfCnt));
    //memset(httpPerfCnt,0,sizeof(httpPerfCnt));
    uint64_t lastSmppPerfCnt[PERF_CNT_COUNT]={0,};
    //uint64_t lastHttpPerfCnt[PERF_HTTP_COUNT]={0,};
    //now.tv_sec=0;
    int i;

   

    // Makes statistics
    using namespace Counters;
    StatisticsManager * sm = dynamic_cast<StatisticsManager*>(perfListener);
    Registrator registrator;
    printf("Registrator is initing...\n");
    registrator.init(sm);
    printf("Registrator is inited\n");
    registrator.Start();
    printf("Registrator is started\n");
    


    //for(;;)
    while(!isStopping)
    {
      
      while(now.tv_sec==time(NULL))ev.Wait(10);

      //===========================
      // replace queue
      //cnt=queue.getCounter();

      int eqhash,equnl;

      //=======================
      // replace queue
      //queue.getStats(equnl);

      clock_gettime(CLOCK_REALTIME,&now);
      ut=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
         (start.tv_sec*1000.0+start.tv_nsec/1000000.0))/1000.0;
      tm=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
         (lasttime.tv_sec*1000.0+lasttime.tv_nsec/1000000.0))/1000;
      //rate=(cnt-last)/tm;
      //avg=cnt/ut;
      printf("UT:%.3lf         \r",ut);
      fflush(stdout);
      //last=cnt;
      lasttime=now;

      if(isStopping)break;

      uint64_t smppPerf[PERF_CNT_COUNT];
      //uint64_t httpPerf[PERF_HTTP_COUNT];
      // success, error, reschedule
      perfListener->getSmppPerfData(smppPerf);
      //perfListener->getHttpPerfData(httpPerf);
      PerformanceData d;
      d.smppCountersNumber=PERF_CNT_COUNT;
      for(i=0;i<PERF_CNT_COUNT;i++)
      {
        d.smppCounters[i].lastSecond=(int)(smppPerf[i]-lastSmppPerfCnt[i]);
        d.smppCounters[i].total=smppPerf[i];
      }
      /*d.httpCountersNumber=PERF_HTTP_COUNT;
      for(i=0;i<PERF_HTTP_COUNT;i++)
      {
        d.httpCounters[i].lastSecond=(int)(httpPerf[i]-lastHttpPerfCnt[i]);
        d.httpCounters[i].total=httpPerf[i];
      }*/


      int scnt=(now.tv_sec-perfStart)/60;
      
      if(scnt<0)scnt=0;
      if(scnt>=60)
      {
        timeshift++;
        if(timeshift>=60)timeshift=0;
        perfStart=times[timeshift];
        scnt=59;
        int idx=timeshift-1;
        if(idx<0)idx=59;
        times[idx]=now.tv_sec;

        for(i=0;i<PERF_CNT_COUNT;i++)smppPerfCnt[i][idx]=0;
        //for(i=0;i<PERF_HTTP_COUNT;i++)httpPerfCnt[i][idx]=0;
      }
      if(scnt!=lastscnt)
      {
        times[scnt]=now.tv_sec;
        lastscnt=scnt;
      }

      for(int j=0;j<PERF_CNT_COUNT;j++)
      {
        d.smppCounters[j].average=0;
      }
      /*for(int j=0;j<PERF_HTTP_COUNT;j++)
      {
        d.httpCounters[j].average=0;
      }*/

      int idx=timeshift;
      for(i=0;i<=scnt;i++,idx++)
      {
        if(idx>=60)idx=0;
        if(i==scnt)
        {

          for(int j=0;j<PERF_CNT_COUNT;j++)
          {
            smppPerfCnt[j][idx]+=d.smppCounters[j].lastSecond;
          }
          /*for(int j=0;j<PERF_HTTP_COUNT;j++)
          {
            httpPerfCnt[j][idx]+=d.httpCounters[j].lastSecond;
          }*/

        }

        for(int j=0;j<PERF_CNT_COUNT;j++)
        {
          d.smppCounters[j].average+=smppPerfCnt[j][idx];
        }
        /*for(int j=0;j<PERF_HTTP_COUNT;j++)
        {
          d.httpCounters[j].average+=httpPerfCnt[j][idx];
        }*/

      }
      int diff=now.tv_sec-times[timeshift];
      if(diff==0)diff=1;

      for(i=0;i<PERF_CNT_COUNT;i++)
      {
        d.smppCounters[i].average/=diff;
      }
      /*for(i=0;i<PERF_HTTP_COUNT;i++)
      {
        d.httpCounters[i].average/=diff;
      }*/

      d.now=now.tv_sec;
      d.uptime=now.tv_sec-start.tv_sec;

      d.sessionCount=0;

      perfListener->reportGenPerformance(&d);

      for(i=0;i<PERF_CNT_COUNT;i++)
      {
        lastSmppPerfCnt[i]=smppPerf[i];
      }
      /*for(i=0;i<PERF_HTTP_COUNT;i++)
      {
        lastHttpPerfCnt[i]=httpPerf[i];
      }*/

      perfListener->reportSvcPerformance();
      perfListener->reportScPerformance();
    }

    //Statistics is made
    printf("Registrator stoping...\n");
    registrator.Stop();
    printf("Registrator stoped\n");
    return 1;
}

void Sender::Start()
{
    perfServer.Start();
    isStopping = false;
    Thread::Start();
}

void Sender::Stop()
{
    isStopping = true;
    perfServer.Stop();
    perfServer.WaitFor();
    WaitFor();
}

void Sender::init(PerformanceListener* pl, PerformanceServer* ps)
{
    perfListener = pl;
    perfServer.init(ps);
}

void Sender::InitServer(std::string perfHost, int genPort, int svcPort, int scPort)
{
    printf("sender, svcPort: %d\n", svcPort);
    perfServer.InitServer(perfHost, genPort, svcPort, scPort);
}

}
}