#ifndef _SCAG_UTIL_MEMORY_MEMORYMANAGER_H
#define _SCAG_UTIL_MEMORY_MEMORYMANAGER_H

#include "scag/util/Print.h"
#include "core/buffers/CyclicQueue.hpp"

namespace scag {
namespace util {
namespace memory {

struct MemoryManagerConfig {
    MemoryManagerConfig() : myalloc(true), prealloc(1000), maxsize_(255) /*, debug(0) */ {}

    inline unsigned char maxsize() const {
        return maxsize_;
    }

public:
    bool                myalloc;   // really use myalloc
    unsigned            prealloc;  // preallocation of the pointer queue
    // scag::util::Print*  debug;     // debugging (owned?)
private:
    unsigned char       maxsize_;   // maximum size treated by memory manager
};


class FixedMemoryManager
{
private:
    typedef smsc::core::synchronization::Mutex Mutex;
    typedef smsc::core::synchronization::MutexGuard MutexGuard;

public:
    FixedMemoryManager( const MemoryManagerConfig& cfg, unsigned sz ) :
    cfg_(cfg), objsize_(sz), queue_(cfg.prealloc), count_(0) {}

    ~FixedMemoryManager() {
        void* p;
        while ( queue_.Pop(p) ) {
            deallocobj(p);
        }
    }


    void print( scag::util::Print& pf ) const 
    {
        if ( pf.enabled() )
            pf.print( "mman@%p sz=%u has %u/%u elts", this, unsigned(objsize()), queue_.Count(), count_ );
    }


    void* allocate() 
    {
        void* res;
        if ( queue_.Count() ) { // first w/o locking
            register bool b;
            {
                MutexGuard mg(mtx_);
                b = queue_.Pop(res);
            }
            if ( b ) {
                // if (cfg_.debug && cfg_.debug->enabled())
                // cfg_.debug->print( "%p* sz=%u cnt=%u/%u", res, objsize(), queue_.Count(), count_ );
                return res;
            }
        }
        // allocate an object and return it
        res = ::operator new( objsize() );
        ++count_;
        // if (cfg_.debug && cfg_.debug->enabled())
        // cfg_.debug->print( "%p+ sz=%u cnt=%u/%u", res, objsize(), queue_.Count(), count_ );
        return res;
    }


    void  deallocate( void* p ) 
    {
        {
            MutexGuard mg(mtx_);
            queue_.Push( p );
        }
        // if (cfg_.debug && cfg_.debug->enabled())
        // cfg_.debug->print( "%p~ sz=%u cnt=%u/%u", p, objsize(), queue_.Count(), count_ );
    }

    inline size_t objsize() const {
        return objsize_;
    }

private:
    inline void deallocobj( void* p ) {
        --count_;
        ::operator delete(p);
        // if (cfg_.debug && cfg_.debug->enabled())
        // cfg_.debug->print( "%p- sz=%u cnt=%u/%u", p, objsize(), queue_.Count(), count_ );
    }


    FixedMemoryManager();
    FixedMemoryManager( const FixedMemoryManager& );
    FixedMemoryManager& operator = ( const FixedMemoryManager& );

private:
    const MemoryManagerConfig&                cfg_;
    size_t                                    objsize_;
    smsc::core::buffers::CyclicQueue< void* > queue_;
    Mutex                                     mtx_;
    unsigned                                  count_;
};


class MemoryManager
{
private:
    typedef smsc::core::synchronization::Mutex       Mutex;
    typedef smsc::core::synchronization::MutexGuard  MutexGuard;

public:
    /// return a global memory manager instance
    static MemoryManager& Instance();

    /// these two methods are for use from operator new and operator delete
    void* allocate( size_t sz ) {
        if ( cfg_.myalloc && sz <= cfg_.maxsize() ) {
            FixedMemoryManager* m = find( sz );
            return m->allocate();
        } else {
            return ::operator new(sz);
        }
    }

    void  deallocate( void* p, size_t sz ) {
        if ( cfg_.myalloc && sz <= cfg_.maxsize() ) {
            FixedMemoryManager* m = find( sz );
            m->deallocate(p);
        } else {
            ::operator delete(p);
        }
    }

    void print( scag::util::Print& pf ) const
    {
        if ( pf.enabled() ) {
            MutexGuard mg(mtx_);
            pf.print( "mman@%p has %u submanagers@%p", this, count_, managers_ );
            for ( unsigned i = 0; i < cfg_.maxsize(); ++i ) {
                unsigned idx = indices_[i];
                if ( idx != cfg_.maxsize() ) {
                    managers_[idx]->print( pf );
                }
            }
        }
    }

    inline const MemoryManagerConfig& cfg() const {
        return cfg_;
    }

    // NOTE: this may be dangerous when set on running allocator
    void setConfig( const MemoryManagerConfig& cfg ) {
        cfg_ = cfg;
    }

protected:
    MemoryManager() :
    seq_(0),
    bufsize_(4),
    managers_( new FixedMemoryManager*[bufsize_] ),
    count_(0)
    {
        for ( unsigned i = 0; i < cfg_.maxsize(); ++i ) indices_[i] = cfg_.maxsize();
        for ( unsigned i = 0; i < bufsize_; ++i ) managers_[i] = 0;
    }

    ~MemoryManager()
    {
        MutexGuard mg(mtx_);
        ++seq_;
        for ( unsigned i = 0; i < cfg_.maxsize(); ++i ) indices_[i] = cfg_.maxsize();
        for ( ; count_ > 0; ) {
            delete managers_[--count_];
            managers_[count_] = 0;
        }
        delete [] managers_;
        managers_ = 0;
        ++seq_;
    }

private:
    FixedMemoryManager* find( size_t sz ) 
    {
        FixedMemoryManager* res = 0;
        do {
            unsigned s = seq_;
            if ( (s % 2) ) break;
            unsigned idx = unsigned(indices_[sz-1]);
            if ( idx == cfg_.maxsize() ) break;
            FixedMemoryManager* m = managers_[idx];
            if ( s != seq_ ) break;
            res = m;
        } while ( false );

        while ( res == 0 ) {
            // needs locking
            MutexGuard mg(mtx_);
            unsigned idx = unsigned(indices_[sz-1]);
            if ( idx != cfg_.maxsize() ) {
                res = managers_[idx];
                break;
            }
            // not found
            unsigned newbufsize = ( count_ == bufsize_ ? 
                                    ( bufsize_ < 16 ? 16 : bufsize_*2 ) :
                                    bufsize_ );
            FixedMemoryManager** newbuf = const_cast< FixedMemoryManager** >(managers_);
            if ( newbufsize != bufsize_ ) {
                newbuf = new FixedMemoryManager* [newbufsize];
                for ( unsigned i = 0; i < bufsize_; ++i ) {
                    newbuf[i] = managers_[i];
                }
                for ( unsigned i = bufsize_ ; i < newbufsize; ++i ) {
                    newbuf[i] = 0;
                }
                bufsize_ = newbufsize;
            }
            res = new FixedMemoryManager( cfg_, sz );
            newbuf[count_] = res;
            ++seq_;
            if ( newbuf != managers_ ) {
                std::swap( const_cast< FixedMemoryManager**& >(managers_),newbuf);
                delete [] newbuf;
            }
            indices_[sz-1] = count_++;
            ++seq_;
            break;
        }
        // assert( res->objsize() == sz );
        return res;
    }

private:
    MemoryManager( const MemoryManager& );
    MemoryManager& operator = ( const MemoryManager& );

private:
    MemoryManagerConfig  cfg_;
    volatile unsigned    seq_;                // seqlock
    volatile unsigned char indices_[256];
    unsigned        bufsize_;       // the length of the mman buffer
    FixedMemoryManager* volatile *managers_;    // the buffer of pointers to mmans

    unsigned        count_;         // current number of mmans
    mutable Mutex   mtx_;           // write mutex

};


/// This one could be used as a replacement for std::allocator< T >.
/// The code basis is taken from n.josuttis example found in the wild.
/// Example:
/// <pre>
///  typedef std::string< char, std::char_traits< char >, StdAlloc<char> > string_type;
///  string_type s;
/// </pre>
template < class T, class BaseMemAlloc = MemoryManager >
    class StdAlloc
{
public:
    /// type definitions
    typedef T              value_type;
    typedef T*             pointer;
    typedef const T*       const_pointer;
    typedef T&             reference;
    typedef const T&       const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;
    
    /// rebind allocator to type U
    template <class U> struct rebind {
        typedef StdAlloc<U> other;
    };

    /// return address of values
    pointer address (reference value) const {
        return &value;
    }
    const_pointer address (const_reference value) const {
        return &value;
    }

    /// constructors and destructor
    /// - nothing to do because the allocator has no state
    StdAlloc() throw() {
    }
    StdAlloc(const StdAlloc&) throw() {
    }
    template <class U> StdAlloc(const StdAlloc<U>&) throw() {
    }
    ~StdAlloc() throw() {
    }

    /// return maximum number of elements that can be allocated
    size_type max_size () const throw() {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    /// allocate but don't initialize num elements of type T
    pointer allocate (size_type num, const void* = 0) {
        /// print message and allocate memory with global new/
        // std::cerr << "allocate " << num << " element(s)"
        // << " of size " << sizeof(T) << std::endl;
        // pointer ret = (pointer)(::operator new(num*sizeof(T)));
        // std::cerr << " allocated at: " << (void*)ret << std::endl;
        pointer ret;
        ret = (pointer) BaseMemAlloc::Instance().allocate( num*sizeof(T) );
        return ret;
    }

    /// initialize elements of allocated storage p with value value/
    void construct (pointer p, const T& value) {
        /// initialize memory with placement new/
        new ((void*)p) T(value);
    }

    /// destroy elements of initialized storage p/
    void destroy (pointer p) {
        /// destroy objects by calling their destructor/
        p->~T();
    }

    /// deallocate storage p of deleted elements/
    void deallocate (pointer p, size_type num) {
        /// print message and deallocate memory with global delete/
        // std::cerr << "deallocate " << num << " element(s)"
        // << " of size " << sizeof(T)
        // << " at: " << (void*)p << std::endl;
        BaseMemAlloc::Instance().deallocate( (void*)p, num*sizeof(T) );
    }

    template < class U >
        bool operator == ( const StdAlloc< U >& other ) const {
            return true;
        }

    template < class U >
        bool operator != ( const StdAlloc< U >& other ) const {
            return false;
        }
};

}
}
}

namespace scag2 {
namespace util {
namespace memory = scag::util::memory;
}
}

#endif /* !_SCAG_UTIL_MEMORY_MEMORYMANAGER_H */
