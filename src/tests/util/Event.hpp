#ifndef __SYNCHRONIZATION_SEMAPHOR__
#define __SYNCHRONIZATION_SEMAPHOR__

#include <thread.h>
#include <synch.h>
#include "Mutex.hpp"

namespace smsc {
  namespace test {
    namespace util {

      class Event {
      public:
        Event() {
          cond_init(&event,USYNC_THREAD,NULL);
          signaled=0;
        }
        ~Event() {
          cond_destroy(&event);
        }
        int Wait() {
          mutex.Lock();
          if(signaled) {
            signaled--;
            mutex.Unlock();
            return 0;
          }
          int retval=cond_wait(&event,&mutex.mutex);
          if(retval == 0) {
            signaled--;
          }
          mutex.Unlock();
          return retval;
        }
        int Wait(int timeout) {
          mutex.Lock();
          if(signaled) {
            signaled--;
            mutex.Unlock();
            return 0;
          }
          timestruc_t tv;
          clock_gettime(CLOCK_REALTIME,&tv);
          tv.tv_sec+=timeout/1000;
          tv.tv_nsec+=(timeout%1000)*1000000L;
          if(tv.tv_nsec>1000000000L) {
            tv.tv_sec++;
            tv.tv_nsec-=1000000000L;
          }
          int retval=cond_timedwait(&event,&mutex.mutex,&tv);
          if(retval == 0) {
            signaled--;
          }
          mutex.Unlock();
          return retval;
        }
        void Signal() {
          mutex.Lock();
          signaled++;
          cond_signal(&event);
          mutex.Unlock();
        }
        int isSignaled() {
          mutex.Lock();
          int retval=signaled;
          mutex.Unlock();
          return retval;
        }
        /*void SignalAll() {
          mutex.Lock();
          signaled=1;
          cond_broadcast(&event);
          mutex.Unlock();
        }*/
      protected:
        cond_t event;
        Mutex mutex;
        int signaled;
      };//Event

    };//namespace util
  };//namespace test
};//namespace smsc


#endif

