#ifndef _INFORMER_EMBEDREFPTR_H
#define _INFORMER_EMBEDREFPTR_H

namespace eyeline {
namespace informer {

template <class T> class EmbedRefPtr
{
public:
    EmbedRefPtr( T* x = 0 ) : x_(x) { if (x_) x_->ref(); }
    EmbedRefPtr( const EmbedRefPtr& p ) : x_(p.x_) { if (x_) x_->ref(); }
    EmbedRefPtr& operator = ( const EmbedRefPtr& p ) {
        reset(p.x_);
        return *this;
    }
    ~EmbedRefPtr() {
        if (x_) x_->unref();
    }
    
    inline const T* get() const { return x_; }
    inline T* get() { return x_; }
    inline const T* operator -> () const { return x_; }
    inline T* operator -> () { return x_; }

private:
    inline void reset( T* x ) {
        if (x_ != x) {
            if (x_) x_->unref();
            x_ = x;
            if (x_) x_->ref();
        }
    }

private:
    T* x_;
};

} // informer
} // smsc

#endif
