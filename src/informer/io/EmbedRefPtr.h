#ifndef _INFORMER_EMBEDREFPTR_H
#define _INFORMER_EMBEDREFPTR_H

namespace eyeline {
namespace informer {

template <class T> class EmbedRefPtr
{
public:
    EmbedRefPtr( T* x = 0 ) : x_(x) { if (x_) x_->ref(); }
    // NOTE: class U must be T or descendant of T
    template <class U> EmbedRefPtr( const EmbedRefPtr<U>& p ) : x_(const_cast<U*>(p.get())) {
        if (x_) x_->ref(); 
    }
    template <class U> EmbedRefPtr& operator = ( const EmbedRefPtr<U>& p ) {
        if (x_ != p.get()) {
            if (x_) x_->unref();
            x_ = const_cast<U*>(p.get());
            if (x_) x_->ref();
        }
        return *this;
    }
    ~EmbedRefPtr() {
        if (x_) x_->unref();
    }
    
    inline const T* get() const { return x_; }
    inline T* get() { return x_; }
    inline const T* operator -> () const { return x_; }
    inline T* operator -> () { return x_; }

protected:
    T* x_;
};

} // informer
} // smsc

#endif
