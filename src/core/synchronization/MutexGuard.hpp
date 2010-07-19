/* ************************************************************************** *
 * Synchronization primitive(s): Mutex
 * ************************************************************************** */
#ifndef __SMSC_CORE_SYNCHRONIZATION_MUTEXGUARD_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_CORE_SYNCHRONIZATION_MUTEXGUARD_HPP__

namespace smsc {
namespace core {
namespace synchronization {

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


template <class _MutexTArg/* : public Mutex*/>
class MutexTryGuard_T {
private:
    bool isLocked;

protected:
    _MutexTArg & lock;

    MutexTryGuard_T(const MutexTryGuard_T &);
    void operator=(const MutexTryGuard_T &);

public:
    MutexTryGuard_T(_MutexTArg & use_lock)
    : isLocked(false), lock(use_lock)
    {
        isLocked = lock.TryLock();
    }
    ~MutexTryGuard_T()
    {
        if (isLocked)
            lock.Unlock();
    }

    bool tgtLocked(void) const { return isLocked; }
};

template <class _MutexTArg/* : public Mutex*/>
class ReverseMutexGuard_T {
protected:
    _MutexTArg & lock;

    ReverseMutexGuard_T(const ReverseMutexGuard_T &);
    void operator=(const ReverseMutexGuard_T &);

public:
    ReverseMutexGuard_T(_MutexTArg & use_lock)
      : lock(use_lock)
    {
        lock.Unlock();
    }
    ~ReverseMutexGuard_T()
    {
        lock.Lock();
    }
};

}//synchronization
}//core
}//smsc

#endif /* __SMSC_CORE_SYNCHRONIZATION_MUTEXGUARD_HPP__ */

