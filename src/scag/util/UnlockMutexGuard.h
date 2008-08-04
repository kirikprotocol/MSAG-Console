#ifndef _SCAG_UTIL_UNLOCKMUTEXGUARD_H
#define _SCAG_UTIL_UNLOCKMUTEXGUARD_H

#include "core/synchronization/Mutex.hpp"

namespace scag {
namespace util {

template < class T >
class UnlockMutexGuardTmpl 
{
public:
    UnlockMutexGuardTmpl( T& lock ) : lock_(lock) {
        lock_.Unlock();
    }
    ~UnlockMutexGuardTmpl() {
        lock_.Lock();
    }
protected:
    mutable T& lock_;
    UnlockMutexGuardTmpl( const UnlockMutexGuardTmpl<T>& );
    UnlockMutexGuardTmpl< T >& operator = ( const UnlockMutexGuardTmpl<T>& );
};

typedef UnlockMutexGuardTmpl< smsc::core::synchronization::Mutex >  UnlockMutexGuard;

} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_UNLOCKMUTEXGUARD_H */
