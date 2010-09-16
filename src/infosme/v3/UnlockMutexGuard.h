#ifndef _INFOSME_V3_UNLOCKMUTEXGUARD_H
#define _INFOSME_V3_UNLOCKMUTEXGUARD_H

#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace infosme {

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

} // infosme
} // smsc

#endif
