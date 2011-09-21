#ifndef _SMSC_CORE_BUFFERS_PERTHREADDATA_H
#define _SMSC_CORE_BUFFERS_PERTHREADDATA_H

#include <pthread.h>

namespace smsc {
namespace core {
namespace buffers {


template < typename T > struct DynAllocPolicy
{
    inline static void destroy( T* t ) { delete t; }
};

/// the interface is analogous to that of auto_ptr<T>,

template < typename T, class AllocPolicy = DynAllocPolicy< T > > class PerThreadData
{
    static void destroyData( void* d ) {
        if (d) {
            AllocPolicy::destroy( static_cast<T*>(d) );
        }
    }

public:
    PerThreadData() {
        // NOTE: it is guaranteed that only one thread is inside the ctor
        pthread_key_create(&key_,destroyData);
    }

    ~PerThreadData() {
        pthread_key_delete(key_);
    }

    inline T* get() {
        void* v = pthread_getspecific(key_);
        return static_cast<T*>(v);
    }

    inline void reset( T* t ) {
        void* v = pthread_getspecific(key_);
        if ( v == t ) return;
        if ( v ) {
            AllocPolicy::destroy(static_cast<T*>(v));
        }
        pthread_setspecific(key_,t);
    }
    
    inline T* release() {
        void* v = pthread_getspecific(key_);
        if (!v) return 0;
        pthread_setspecific(key_,NULL);
        return static_cast<T*>(v);
    }

private:
    PerThreadData( const PerThreadData& );
    PerThreadData& operator = ( const PerThreadData& );

private:
    pthread_key_t key_;
};

} // buffers
} // core
} // smsc

#endif
