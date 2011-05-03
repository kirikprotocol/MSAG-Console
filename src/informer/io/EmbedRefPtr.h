#ifndef _INFORMER_EMBEDREFPTR_H
#define _INFORMER_EMBEDREFPTR_H

#include <typeinfo>
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

/*
extern smsc::logger::Logger* erplog_;
inline void erpgetlog()
{
    if (!erplog_) {
        erplog_ = smsc::logger::Logger::getInstance("erp");
    }
}
 */

template <class T> class EmbedRefPtr
{
public:
    explicit EmbedRefPtr( T* x = 0 ) : x_(x) {
        if (x_) x_->ref();
        // erpgetlog();
        // if (x_) { smsc_log_debug(erplog_,"ctor(%s)@%p x=%p",thetypename(),this,x_); }
    }
    EmbedRefPtr( const EmbedRefPtr& p ) : x_(p.x_) {
        if (x_) x_->ref();
        // erpgetlog();
        // if (x_) { smsc_log_debug(erplog_,"ctor(%s)@%p x=%p",thetypename(),this,x_); }
    }
    EmbedRefPtr& operator = ( const EmbedRefPtr& p ) {
        reset(p.x_);
        return *this;
    }

    // NOTE: class U must be T or descendant of T
    template <class U>
    EmbedRefPtr( const EmbedRefPtr<U>& p ) : x_(const_cast<U*>(p.get())) {
        if (x_) x_->ref();
        // erpgetlog();
        // if (x_) { smsc_log_debug(erplog_,"ctor(%s)@%p x=%p",thetypename(),this,x_); }
    }
    template <class U>
    EmbedRefPtr< T >& operator = ( const EmbedRefPtr< U >& p ) {
        reset( static_cast<T*>(const_cast<U*>(p.get())) );
        return *this;
    }
    ~EmbedRefPtr() {
        if (x_) {
            // smsc_log_debug(erplog_,"dtor(%s)@%p x=%p",thetypename(),this,x_);
            x_->unref();
        }
    }

    // to test the validity of the ptr
    inline bool operator ! () const { return !x_; }
    inline bool operator == ( const EmbedRefPtr& p ) const { return x_ == p.x_; }

    inline const T* get() const { return x_; }
    inline T* get() { return x_; }
    inline const T* operator -> () const { return x_; }
    inline T* operator -> () { return x_; }

    inline T& operator * () { return *x_; }
    inline const T& operator * () const { return *x_; }

    inline void reset( T* x ) {
        if (x != x_) {
            // smsc_log_debug(erplog_,"reset(%s)@%p x=%p -> x=%p",thetypename(),this,x_,x);
            if (x_) x_->unref();
            x_ = x;
            if (x_) x_->ref();
        }
    }
protected:
    inline const char* thetypename() const {
        return typeid(T).name();
    }

protected:
    T* x_;
};

} // informer
} // smsc

#endif
