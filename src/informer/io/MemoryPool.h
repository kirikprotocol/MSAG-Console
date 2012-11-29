#ifndef _INFORMER_MEMORYPOOL_H
#define _INFORMER_MEMORYPOOL_H

#include "core/synchronization/Mutex.hpp"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class MemoryPool
{
    struct BadAlloc : public std::bad_alloc {
        virtual const char* what() const throw () {
            return "memory pool is not initialized";
        }
    };

    struct MemoryCache;

public:
    struct SizeCache {
        size_t       maxsize;
        MemoryCache* cache;

        struct CompareSize {
            inline bool operator () ( const SizeCache& sc, size_t sz ) const {
                return sc.maxsize < sz;
            }
        };
    };


    explicit MemoryPool( const size_t* sz,
                         smsc::logger::Logger* l = 0 );


    ~MemoryPool();


    void setLogger( smsc::logger::Logger* logger )
    {
        smsc::core::synchronization::MutexGuard mg(cacheLock_);
        log_ = logger;
    }


    /// delete all data from pools
    /// NOTE: should be invoked only when there is no access to the pool
    void clean();


    void* allocate(SizeCache* c, size_t realsz);


    /// may throw
    void* allocate(size_t sz);


    void deallocate(void* ptr, size_t sz) throw ();


    SizeCache* sizeToCache( size_t size );

private:
    smsc::core::synchronization::Mutex cacheLock_;
    smsc::logger::Logger* log_;
    size_t     ncaches_;
    SizeCache* caches_;
};

}
}

#endif
