#include "Sender.h"
#include "StatisticsManager.h"
#include "scag/sessions/base/SessionManager2.h"
#include "scag/transport/smpp/base/SmppManager2.h"
#include "scag/transport/http/base/HttpManager.h"
#include "util/TimeSource.h"
#include <stdlib.h>
#include <unistd.h>

namespace scag2 {
namespace stat {

using smsc::core::threads::Thread;
using smsc::logger::Logger;
using namespace sessions;
using namespace transport::smpp;
using namespace transport::http;

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
private:
    void Start( int );
};

int Registrator::Execute()
{
    int counter = 0;
    int httpCounter = 0;

    // smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("reg");
    while(!isStopping)
    {
        SmppStatEvent si;
        HttpStatEvent hs;

        //=============== sme1 =====================
        if(++counter == 9) counter = 1;
        int count =  7. * ( (double)random() / 2147483648. ) + 1;
        for(int i = 0; i<= count - 1; i++){
            strcpy(si.srcId, "sme1");
            si.srcType = false;
            strcpy(si.dstId, "smsc1");
            si.dstType = true;
            strcpy(si.routeId, "route1");
            si.routeProviderId = 1;
            si.event = 17 + counter;
            si.errCode = 1;
            sm->registerEvent(si);
        }
        //==========================================
        count =  7. * ( (double)random() / 2147483648. ) + 1;
        for(int i = 0; i <= count - 1; i++){
            strcpy(si.srcId, "sme2");
            si.srcType = false;
            strcpy(si.dstId, "smsc2");
            si.dstType = true;
            strcpy(si.routeId, "route3");
            si.routeProviderId = 3;
            si.event = 17 + counter;
            si.errCode = 1;
            sm->registerEvent(si);
        }
        //=================== http1 ====================
        if(++httpCounter == 7) httpCounter = 1;
        hs.routeId = "1";
        hs.url = "yandex.ru/url1";
        hs.site = "yandex.ru";
        hs.serviceId = 1;
        hs.event = httpCounter;
        hs.errCode = 1;
        count =  httpCounter; //7. * ( (double)random() / 2147483648. ) + 1;
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
    : perfListener(0), isStopping(false)/*, logger(Logger::getInstance("Sender"))*/
{
    start.tv_sec=0;
    start.tv_nsec=0;
}

Sender::~Sender()
{
}

int Sender::Execute()
{
    // smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("stat.send");

    // uint64_t cnt,last=0;
    timespec now={0,0},lasttime={0,0};
    double ut,tm; // ,rate,avg;
    if(start.tv_sec==0)
      clock_gettime(CLOCK_REALTIME,&start);
    Event ev;
    timeshift=0;
    time_t perfStart=start.tv_sec;
    for(int i=0;i<60;i++)times[i]=start.tv_sec;
    int lastscnt=0;
    memset(smppPerfCnt,0,sizeof(smppPerfCnt));
    memset(httpPerfCnt,0,sizeof(httpPerfCnt));
    uint64_t lastSmppPerfCnt[PERF_CNT_COUNT]={0,};
    uint64_t lastHttpPerfCnt[PERF_HTTP_COUNT]={0,};
    //now.tv_sec=0;
    int i;
 

    // Makes statistics
#ifdef MSAG_FAKE_STAT
    StatisticsManager * sm = dynamic_cast<StatisticsManager*>(perfListener);
    Registrator registrator;
    printf("Registrator is initing...\n");
    registrator.init(sm);
    printf("Registrator is inited\n");
    registrator.Start();
    printf("Registrator is started\n");
#endif

    //for(;;)
    while(!isStopping)
    {
      // smsc_log_debug(logger,"stat.send rolling");
      while(now.tv_sec==time(NULL))ev.Wait(10);
      if(isStopping) break;
      //===========================
      // replace queue
      //cnt=queue.getCounter();

      // int eqhash,equnl;

      //=======================
      // replace queue
      //queue.getStats(equnl);

      clock_gettime(CLOCK_REALTIME,&now);
      ut=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
         (start.tv_sec*1000.0+start.tv_nsec/1000000.0))/1000.0;
      tm=((now.tv_sec*1000.0+now.tv_nsec/1000000.0)-
         (lasttime.tv_sec*1000.0+lasttime.tv_nsec/1000000.0))/1000;
//      smsc_log_debug(logger, "now.tv_sec=%d, now.tv_nsec=%d, start.tv_sec=%d, start.tv_nsec=%d", now.tv_sec, now.tv_nsec, start.tv_sec, start.tv_nsec);
      //rate=(cnt-last)/tm;
      //avg=cnt/ut;
//      printf("UT:%.3lf         \r",ut);
//      fflush(stdout);
      //last=cnt;
      lasttime=now;
      if(isStopping)break;

      uint64_t smppPerf[PERF_CNT_COUNT];
      uint64_t httpPerf[PERF_HTTP_COUNT];
      // success, error, reschedule
      perfListener->getSmppPerfData(smppPerf);
      perfListener->getHttpPerfData(httpPerf);
      PerformanceData d;
      d.smppCountersNumber=PERF_CNT_COUNT;

      for(i=0;i<PERF_CNT_COUNT;i++)
      {
        d.smppCounters[i].lastSecond=(int)(smppPerf[i]-lastSmppPerfCnt[i]);
        d.smppCounters[i].total=smppPerf[i];
      }
      d.httpCountersNumber=PERF_HTTP_COUNT;
      for(i=0;i<PERF_HTTP_COUNT;i++)
      {
        d.httpCounters[i].lastSecond=(int)(httpPerf[i]-lastHttpPerfCnt[i]);
        d.httpCounters[i].total=httpPerf[i];
      }

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
        for(i=0;i<PERF_HTTP_COUNT;i++)httpPerfCnt[i][idx]=0;
      }
      if(scnt!=lastscnt)
      {
        times[scnt]=now.tv_sec;
        lastscnt=scnt;
      }

      for(int j=0;j<PERF_CNT_COUNT;j++)
        d.smppCounters[j].average=0;

      for(int j=0;j<PERF_HTTP_COUNT;j++)
        d.httpCounters[j].average=0;

      int idx=timeshift;
      for(i=0;i<=scnt;i++,idx++)
      {
        if(idx>=60)idx=0;
        if(i==scnt)
        {

          for(int j=0;j<PERF_CNT_COUNT;j++)
            smppPerfCnt[j][idx]+=d.smppCounters[j].lastSecond;

          for(int j=0;j<PERF_HTTP_COUNT;j++)
            httpPerfCnt[j][idx]+=d.httpCounters[j].lastSecond;
        }

        for(int j=0;j<PERF_CNT_COUNT;j++)
          d.smppCounters[j].average+=smppPerfCnt[j][idx];
        for(int j=0;j<PERF_HTTP_COUNT;j++)
          d.httpCounters[j].average+=httpPerfCnt[j][idx];

      }

      int diff=now.tv_sec-times[timeshift];
      if(diff==0)diff=1;

      for(i=0;i<PERF_CNT_COUNT;i++)
        d.smppCounters[i].average/=diff;
      for(i=0;i<PERF_HTTP_COUNT;i++)
        d.httpCounters[i].average/=diff;

      d.now=now.tv_sec;
      d.uptime=now.tv_sec-start.tv_sec;

        // smsc_log_debug( logger, "going to get statistics");
        SessionManager::Instance().getSessionsCount( d.sessionCount,
                                                     d.sessionLoadedCount,
                                                     d.sessionLockedCount );
        SmppManager::Instance().getQueueLen(d.smppReqQueueLen, d.smppRespQueueLen, d.smppLCMQueueLen);
        // smsc_log_debug( logger, "stat is sessions: tot/lck=%u/%u, queues: req/resp/lcm=%u/%u/%u",
        // d.sessionCount, d.sessionLockedCount,
        // d.smppReqQueueLen, d.smppRespQueueLen, d.smppLCMQueueLen );

        HttpManager::Instance().getQueueLen(d.httpReqQueueLen, d.httpRespQueueLen, d.httpLCMQueueLen);
        d.httpReqQueueLen = 0;
        d.httpRespQueueLen = 0;
        d.httpLCMQueueLen = 0;

      perfListener->reportGenPerformance(&d);

      for(i=0;i<PERF_CNT_COUNT;i++)
        lastSmppPerfCnt[i]=smppPerf[i];
      for(i=0;i<PERF_HTTP_COUNT;i++)
        lastHttpPerfCnt[i]=httpPerf[i];

      perfListener->reportSvcPerformance();
      perfListener->reportScPerformance();
    }

#ifdef MSAG_FAKE_STAT
    //Statistics is made
    printf("Registrator stoping...\n");
    registrator.Stop();
    printf("Registrator stoped\n");
#endif

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

void Sender::reinitPrfSrvLogger()
{
    printf("sender, reinitPrfSrvLogger\n");
    perfServer.reinitLogger();
}

}
}
