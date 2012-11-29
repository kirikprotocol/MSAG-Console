#ifndef _INFORMER_STDALLOC_H
#define _INFORMER_STDALLOC_H

namespace eyeline {
namespace informer {

template < class T, class PoolProvider > class StdAlloc
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
        typedef StdAlloc<U, PoolProvider > other;
    };

    /// return address of values
    inline pointer address (reference value) const {
        return &value;
    }
    inline const_pointer address (const_reference value) const {
        return &value;
    }

    /// constructors and destructor
    /// - nothing to do because the allocator has no state
    inline StdAlloc() throw() {
    }

    template <class U> inline StdAlloc(const StdAlloc<U, PoolProvider>&) throw() {
    }

    inline ~StdAlloc() throw() {
    }

    /// return maximum number of elements that can be allocated
    inline size_type max_size () const throw() {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    /// allocate but don't initialize num elements of type T
    inline pointer allocate (size_type num, const void* = 0) {
        /// print message and allocate memory with global new/
        // std::cerr << "allocate " << num << " element(s)"
        // << " of size " << sizeof(T) << std::endl;
        // pointer ret = (pointer)(::operator new(num*sizeof(T)));
        // std::cerr << " allocated at: " << (void*)ret << std::endl;
        pointer ret;
        ret = (pointer) PoolProvider::getPool().allocate( num*sizeof(T) );
        return ret;
    }

    /// initialize elements of allocated storage p with value value/
    inline void construct (pointer p, const T& value) {
        /// initialize memory with placement new/
        new ((void*)p) T(value);
    }

    /// destroy elements of initialized storage p/
    inline void destroy (pointer p) {
        /// destroy objects by calling their destructor/
        p->~T();
    }

    /// deallocate storage p of deleted elements/
    inline void deallocate (pointer p, size_type num) {
        /// print message and deallocate memory with global delete/
        // std::cerr << "deallocate " << num << " element(s)"
        // << " of size " << sizeof(T)
        // << " at: " << (void*)p << std::endl;
        PoolProvider::getPool().deallocate( (void*)p, num*sizeof(T) );
    }

    template < class U >
        inline bool operator == ( const StdAlloc< U, PoolProvider >& other ) const {
            return true;
        }

    template < class U >
        inline bool operator != ( const StdAlloc< U, PoolProvider >& other ) const {
            return false;
        }
};

}
}

#endif
