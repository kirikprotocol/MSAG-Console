#ifndef __SMSC_CORE_SYNCHRONIZATION_MUTEXGUARD_HPP__
#define __SMSC_CORE_SYNCHRONIZATION_MUTEXGUARD_HPP__

namespace smsc{
namespace core{
namespace synchronization{

template <class T>
class MutexGuardTmpl{
public:
  MutexGuardTmpl(T& _lock):lock(_lock)
  {
    lock.Lock();
  }
  ~MutexGuardTmpl()
  {
    lock.Unlock();
  }
protected:
  T& lock;
  MutexGuardTmpl(const MutexGuardTmpl&);
  void operator=(const MutexGuardTmpl&);
};

}//synchronization
}//core
}//smsc

#endif
