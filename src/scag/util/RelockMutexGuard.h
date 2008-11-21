#ifndef _SCAG_UTIL_RELOCKMUTEXGUARD_H
#define _SCAG_UTIL_RELOCKMUTEXGUARD_H

#include "core/synchronization/Mutex.hpp"

namespace scag {
namespace util {

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
    bool       locked_;
    mutable T& lock_;
    RelockMutexGuardTmpl( const RelockMutexGuardTmpl<T>& );
    RelockMutexGuardTmpl< T >& operator = ( const RelockMutexGuardTmpl<T>& );
};

typedef RelockMutexGuardTmpl< smsc::core::synchronization::Mutex >  RelockMutexGuard;

} // namespace util
} // namespace scag

namespace scag2 {
namespace util {
using scag::util::RelockMutexGuardTmpl;
using scag::util::RelockMutexGuard;
}
}

#endif /* !_SCAG_UTIL_UNLOCKMUTEXGUARD_H */
