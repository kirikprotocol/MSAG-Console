#ifndef __DBENTITYSTORAGE_RWLOCKGUARD_HPP__
# define __DBENTITYSTORAGE_RWLOCKGUARD_HPP__ 1

# include <pthread.h>

class ReadLockGuard {
public:
  ReadLockGuard(pthread_rwlock_t& lock) : _lock(lock) {
    pthread_rwlock_rdlock(&_lock);
  }
  ~ReadLockGuard() {
    pthread_rwlock_unlock(&_lock);
  }
private:
  pthread_rwlock_t& _lock;
};

class WriteLockGuard {
public:
  WriteLockGuard(pthread_rwlock_t& lock) : _lock(lock) {
    pthread_rwlock_wrlock(&_lock);
  }
  ~WriteLockGuard() {
    pthread_rwlock_unlock(&_lock);
  }
private:
  pthread_rwlock_t& _lock;
};

#endif
