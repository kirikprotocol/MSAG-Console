#ifndef _SCAG_COUNTER_EMBEDREFPTR_H
#define _SCAG_COUNTER_EMBEDREFPTR_H

namespace scag2 {
namespace counter {

/// a ref-counter gratefully adapted from boost.
/// NOTE: a function should exist:
/// bool T::changeUsage( bool inc ); -- return true when there is no more refs
template < class T > class EmbedRefPtr
{
public:
    inline EmbedRefPtr( T* c = 0 ) : ptr_(c) {
        if (ptr_) ptr_->changeUsage(true);
    }
    inline ~EmbedRefPtr() { if (ptr_ && ptr_->changeUsage(false)) delete ptr_; }
    inline EmbedRefPtr( const EmbedRefPtr& c ) : ptr_(c.ptr_) {
        if (ptr_) ptr_->changeUsage(true);
    }
    inline EmbedRefPtr& operator = ( const EmbedRefPtr& c ) {
        if ( c.ptr_ != ptr_ ) {
            if (ptr_ && ptr_->changeUsage(false)) delete ptr_;
            ptr_ = c.ptr_;
            if (ptr_) ptr_->changeUsage(true);
        }
        return *this;
    }

    inline T* operator -> () const {
        return ptr_;
    }
    inline T* get() const {
        return ptr_;
    }
    // inline T* release() {
    // if (ptr_) ptr_->changeUsage(false);
    // return ptr_;
    // }
private:
    T* ptr_;
};

}
}

#endif
