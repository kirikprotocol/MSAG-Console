#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <core/network/Socket.hpp>
#include <core/synchronization/Mutex.hpp>
#include <logger/Logger.h>

using namespace smsc::logger;
using namespace smsc::core::synchronization;
using namespace smsc::core::network;

smsc::logger::Logger *logger;
Mutex mtx;
uint32_t cnt = 0, err = 0;

#define SERVER "sunfire"
#define PORT 5001
#define URL_ "+79039126430_111/192.168.1.213/w1251.html?abonent=+79039126430&usr=12"
#define THREADS_CNT 1
#define REQUESTS_CNT 1
/*
https://eyeline.mobi/images/home_page_9mm.jpg
http://eyeline.mobi/images/home_page_9mm.jpg
#define SERVER "sunfire"
#define PORT 5001
#define URL_ "/1_2/192.168.1.213/w1251.html"
#define THREADS_CNT 60
#define REQUESTS_CNT 10000*/


extern "C" void* thread(void* a)
{
    char buf[200];
    for(int j = 0; j < REQUESTS_CNT; j++)
    {
        Socket *s = new Socket();
        if (!s->Init(SERVER, PORT, 100) && !s->Connect())
        {
            const char str[] = "GET "URL_" HTTP/1.1\r\nHost: "SERVER"\r\nConnection: close\r\nCookie: $Version=0;WHOISD-ADDRESS=+79039126430;WHOISD-USR=14;test_cookie=213;expires=234;test_cookie2=asdf;\r\nWHOISD-ADDRESS:+79039126430\r\nWHOISD-USR:14\r\n\r\n";

            if(s->Write(str, sizeof(str)) == sizeof(str))
            {
//        shutdown(s->getSocket(), SHUT_RD);
                while (s->Read(buf, 200) > 0);
                s->Abort();
                delete s;

                {
                    MutexGuard mt(mtx);
                    cnt++;
                }
                continue;
           }
       }
       s->Abort();
       delete s;
       {
            MutexGuard mt(mtx);
            err++;
       }
   }
   return NULL;
}

int main() {
    sigset_t oset;
    sigset_t set;
    time_t t;
    uint32_t last_cnt = 0;
        
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, &oset);

    Logger::Init();
    logger = Logger::getInstance("scag.http");

    for(int j = 0; j < THREADS_CNT; j++)
        pthread_create(NULL, NULL, thread, (void*)j);

    time_t st = time(NULL);
    while(1)
    {
        sleep(1);
        {
            MutexGuard mt(mtx);
            time_t t = time(NULL);
            printf("Time: %d, Cnt: %d, Avg: %d, Last sec: %d, Err: %d\n", t - st, cnt, cnt/(t - st), cnt - last_cnt, err);
            last_cnt = cnt;
        }
    }

    Logger::Shutdown();
    
    pthread_sigmask(SIG_SETMASK, &oset, NULL);
}
