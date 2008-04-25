#ifndef __SMSC_CORE_SYNCHRONIZATION_RWLOCKGUARD_HPP__
# define __SMSC_CORE_SYNCHRONIZATION_RWLOCKGUARD_HPP__ 1

# include <core/synchronization/RWLock.hpp>

namespace smsc{
namespace core{
namespace synchronization{

class ReadLockGuard {
public:
  ReadLockGuard(RWLock& lock) : _lock(lock) {
    _lock.rlock();
  }
  ~ReadLockGuard() {
    _lock.unlock();
  }
private:
  RWLock& _lock;
};

class WriteLockGuard {
public:
  WriteLockGuard(RWLock& lock) : _lock(lock) {
    _lock.wlock();
  }
  ~WriteLockGuard() {
    _lock.unlock();
  }
private:
  RWLock& _lock;
};

}}}

#endif
