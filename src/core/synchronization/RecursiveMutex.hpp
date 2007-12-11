#pragma ident "$Id$"
#ifndef __CORE_SYNCHRONIZATION_RECURSIVEMUTEX_HPP__
#define __CORE_SYNCHRONIZATION_RECURSIVEMUTEX_HPP__

#include <string>
#include <stdexcept>

#include "core/synchronization/Mutex.hpp"

namespace smsc{
namespace core{
namespace synchronization{

class Event;

class RecursiveMutex : public Mutex {
public:
  RecursiveMutex()
  {
    if ( pthread_mutexattr_init(&attr) ||
         pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) )
      throw std::runtime_error("RecursiveNutex:: pthread_mutexattr_init was failed");

    pthread_mutex_init(&mutex,&attr);
  }
  ~RecursiveMutex()
  {
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attr);
  }

protected:
  pthread_mutexattr_t attr;

  RecursiveMutex(const RecursiveMutex&);
  void operator=(const RecursiveMutex&);
};

typedef MutexGuardTmpl<RecursiveMutex> RecursiveMutexGuard;

}//synchronization
}//core
}//smsc

#endif /* __CORE_SYNCHRONIZATION_RECURSIVEMUTEX_HPP__ */

