#ifndef __SYNCHRONIZATION_MUTEX__
#define __SYNCHRONIZATION_MUTEX__

#include <thread.h>
#include <synch.h>

namespace smsc {
  namespace test {
    namespace util {


      class Event;

      class Mutex {
      public:
        Mutex() {
          mutex_init(&mutex,USYNC_THREAD,NULL);
        }
        ~Mutex() {
          mutex_destroy(&mutex);
        }
        void Lock() {
          mutex_lock(&mutex);
        }
        void Unlock() {
          mutex_unlock(&mutex);
        }
        bool TryLock() {
          return mutex_trylock(&mutex)==0;
        }
      protected:
        mutex_t mutex;
        friend class Event;
        Mutex(const Mutex&);
        void operator=(const Mutex&);
      };//Mutex

    }//namespace util
  }//namespace test
}//namespace smsc

#endif
