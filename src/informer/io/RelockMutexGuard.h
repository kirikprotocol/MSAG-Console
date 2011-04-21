#ifndef _INFORMER_RELOCKMUTEXGUARD_H
#define _INFORMER_RELOCKMUTEXGUARD_H

#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

template < class T >
class RelockMutexGuardTmpl 
{
public:
#ifdef CHECKCONTENTION
    RelockMutexGuardTmpl( T& lock, const char* from=0) : lock_(lock), locked_(false), from_(from) {
        lock_.Lock(from); locked_ = true;
    }
#else
    RelockMutexGuardTmpl( T& lock ) : lock_(lock), locked_(false) {
        lock_.Lock(); locked_ = true;
    }
#endif
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
#ifdef CHECKCONTENTION
            lock_.Lock(from_);
#else
            lock_.Lock();
#endif
        }
    }
    inline bool isLocked() const { return locked_; }

protected:
    T&          lock_;
    bool        locked_;
#ifdef CHECKCONTENTION
    const char* from_;
#endif

    RelockMutexGuardTmpl( const RelockMutexGuardTmpl<T>& );
    RelockMutexGuardTmpl< T >& operator = ( const RelockMutexGuardTmpl<T>& );
};

typedef RelockMutexGuardTmpl< smsc::core::synchronization::Mutex >  RelockMutexGuard;

} // informer
} // eyeline

#endif
