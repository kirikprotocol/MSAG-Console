#ifndef __SMSC_CORE_SYNCHRONIZATION_RWLOCK_HPP__
# define __SMSC_CORE_SYNCHRONIZATION_RWLOCK_HPP__ 1

# include <pthread.h>
# include <util/Exception.hpp>

namespace smsc{
namespace core{
namespace synchronization{

class RWLock {
public:
  RWLock() {
    if ( pthread_rwlock_init(&_lock, NULL) < 0 )
      throw smsc::util::SystemError("RWLock::RWLock::: call to pthread_rwlock_init failed");
  }

  void rlock() {
    if ( pthread_rwlock_rdlock(&_lock) )
      throw smsc::util::SystemError("RWLock::rlock::: call to pthread_rwlock_rdlock failed");
  }

  void wlock() {
    if ( pthread_rwlock_wrlock(&_lock) )
      throw smsc::util::SystemError("RWLock::wlock::: call to pthread_rwlock_wrlock failed");
  }

  void unlock() {
    if ( pthread_rwlock_unlock(&_lock) )
      throw smsc::util::SystemError("RWLock::unlock::: call to pthread_rwlock_unlock failed");
  }
private:
  RWLock(const RWLock& rhs);
  RWLock& operator=(const RWLock& rhs);

  pthread_rwlock_t _lock;
};

}}}

#endif
