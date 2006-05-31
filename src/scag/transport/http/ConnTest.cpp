#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <iconv.h>
#include "Managers.h"
#include "HttpCommand.h"
#include "HttpProcessor.h"

#define ICONV_BLOCK_SIZE 32

//#define LOG_HEADERS
//#define LOG_CONTENT
//#define LOG_TEXT
//#define LOG_QUERY

//void printHex(const char *buffer, long length);

using namespace scag::transport::http;
using namespace smsc::logger;
using namespace smsc::core::synchronization;

smsc::logger::Logger *logger;
Mutex mtx;
uint32_t cnt = 0;

extern "C" void* thread(void* a)
{
    char buf[200];
    for(int j = 0; j < 10000; j++)
    {
        Socket *s = new Socket();
        if (s->Init("sunfire", 5001, 100) || s->Connect())
        {
            printf("cant connect.");
            delete s;
            continue;
        }
//        const char str[] = "GET /w1251.html HTTP/1.1\r\nHost: 192.168.1.213\r\nConnection: close\r\n\r\n";
        const char str[] = "GET /1_2/192.168.1.213/w1251.html HTTP/1.1\r\nHost: sunfire\r\nConnection: close\r\n\r\n";

        if(s->Write(str, sizeof(str)) != sizeof(str))
        {
           printf("Error sending request");
           delete s;
           continue;
        }
//        shutdown(s->getSocket(), SHUT_RD);
        while (s->Read(buf, 200) > 0);
//        s->Abort();
        delete s;

        {
            MutexGuard mt(mtx);
            cnt++;
        }
   }
   return NULL;
}

int main() {
    int k = 0;
    sigset_t oset;
    sigset_t set;
    time_t t;
    uint32_t last_cnt = 0;
    Array<Socket*> a;
        
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, &oset);

    Logger::Init();
    logger = Logger::getInstance("scag.http");

    for(int j = 0; j < 60; j++)
        pthread_create(NULL, NULL, thread, (void*)j);

    time_t st = time(NULL);
    while(1)
    {
        sleep(1);
        {
            MutexGuard mt(mtx);
            time_t t = time(NULL);
            printf("Time: %d, Cnt: %d, Avg: %d, Last sec: %d\n", t - st, cnt, cnt/(t - st), cnt - last_cnt);
            last_cnt = cnt;
        }
    }
/*    for(int j = 0; j < 10000; j++)
    {
        time_t n = time(NULL);
        a.Empty();    
        for(int i = 0; i < 60; i++)
        {
            Socket *s = new Socket();
            if (s->Init("192.168.1.213", 80, 100) || s->Connect())
            {
                smsc_log_error(logger, "cant connect. %d", k);
                delete s;
            }
            else
            {
                a.Push(s);
                k++;
            }
        }
        for(int i = 0; i < a.Count(); i++)
        {
            const char* s = "GET /w1251.html HTTP/1.1\r\nHost: 192.168.1.213\r\nConnection: close\r\n\r\n";
            if(a[i]->Write(s, sizeof(s)) == -1)
                smsc_log_error(logger, "Error sending request %d", k);
        }
        for(int i = 0; i < a.Count(); i++)
        {
            a[i]->Close();
            delete a[i];
        }
        n = time(NULL) - n;
        if(n > 0)
            printf("Time: %d, Count=%d\n", n, k);
    }*/

    Logger::Shutdown();
    
    pthread_sigmask(SIG_SETMASK, &oset, NULL);
}
