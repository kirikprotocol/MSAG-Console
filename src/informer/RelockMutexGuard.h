#ifndef _INFORMER_RELOCKMUTEXGUARD_H
#define _INFORMER_RELOCKMUTEXGUARD_H

#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace informer {

template < class T >
class RelockMutexGuardTmpl 
{
public:
    RelockMutexGuardTmpl( T& lock ) : lock_(lock), locked_(false) {
        lock_.Lock(); locked_ = true;
    }
    ~RelockMutexGuardTmpl() {
        if (locked_) lock_.Unlock();
    }
    void Unlock() {
        if ( locked_ ) {
            locked_ = false;
            lock_.Unlock();
        }
    }
    void Lock() {
        if ( !locked_ ) {
            locked_ = true;
            lock_.Lock();
        }
    }
    inline bool isLocked() const { return locked_; }

protected:
    mutable T& lock_;
    bool       locked_;

    RelockMutexGuardTmpl( const RelockMutexGuardTmpl<T>& );
    RelockMutexGuardTmpl< T >& operator = ( const RelockMutexGuardTmpl<T>& );
};

typedef RelockMutexGuardTmpl< core::synchronization::Mutex >  RelockMutexGuard;

} // informer
} // smsc

#endif
