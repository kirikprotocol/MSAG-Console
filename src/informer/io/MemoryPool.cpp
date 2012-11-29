#include "MemoryPool.h"

#include <algorithm>
#include <assert.h>
#include "core/synchronization/AtomicCounter.hpp"

// Unfortunately at the great load the spool on atomic spinlock
// is consuming a lot of CPU.  Thus using mutexes for now.
// #ifdef ATOMICSUSEMUTEX
#ifndef MEMORYPOOLUSEMUTEX
#define MEMORYPOOLUSEMUTEX
#endif
// #endif

namespace eyeline {
namespace informer {

struct MemoryPool::MemoryCache 
{
        
#ifndef MEMORYPOOLUSEMUTEX
    typedef smsc::core::synchronization::FastestAtomic Fastest;
#endif

    struct Chunk {
        Chunk* next;
    };


    explicit MemoryCache( MemoryPool& p, size_t size ) :
    pool(p),
#ifndef MEMORYPOOLUSEMUTEX
    serial_(0),
#endif
    freeChain(0), sz(size)
    {
        smsc::logger::Logger* l = pool.log_;
        if (l) {
            smsc_log_debug(l,"ctor cache sz=%u",unsigned(sz));
        }
    }


    ~MemoryCache() {
        clean();
    }


    void clean()
    {
        unsigned cnt = 0;
        for ( Chunk* p = freeChain; p; p = freeChain ) {
            freeChain = p->next;
            ::operator delete(p);
            ++cnt;
        }
        smsc::logger::Logger* l = pool.log_;
        if (l) {
            smsc_log_info(l,"cache sz=%u usage=%u",unsigned(sz),cnt);
        }
    }


    void* allocate(size_t realsz)
    {
        void* res = 0;
#ifdef MEMORYPOOLUSEMUTEX
        {
            smsc::core::synchronization::MutexGuard mg(lock);
            if ( freeChain ) {
                res = freeChain;
                freeChain = freeChain->next;
            }
        }
#else
        do {
            const Fastest serial = serial_.get();
            if ( !(serial & 1) && serial == serial_.cas(serial,serial+1)) {
                if (freeChain) {
                    res = freeChain;
                    freeChain = freeChain->next;
                }
                serial_.set(serial+2);
                break;
            }
        } while (true);
#endif // !mutex

        bool cached;
        if (!res) {
            cached = false;
            res = ::operator new(sz);
        } else {
            cached = true;
        }

        smsc::logger::Logger* l = pool.log_;
        if (l) {
            smsc_log_debug(l,"cache sz=%u real=%u alloc=%p%s",
                           unsigned(sz),unsigned(realsz),res,
                           cached ? "(cached)" : "");
        }
        return res;
    }


    void deallocate(void* ptr) throw () 
    {
        if (!ptr) return;
        smsc::logger::Logger* l = pool.log_;
        if (l) {
            smsc_log_debug(l,"cache sz=%u deall=%p",
                           unsigned(sz),ptr); 
        }
        Chunk* head = reinterpret_cast<Chunk*>(ptr);
#ifdef MEMORYPOOLUSEMUTEX
        smsc::core::synchronization::MutexGuard mg(lock);
        head->next = freeChain;
        freeChain = head;
#else
        do {
            const Fastest serial = serial_.get();
            if (!(serial&1) && serial == serial_.cas(serial,serial+1)) {
                head->next = freeChain;
                freeChain = head;
                serial_.set(serial+2);
                break;
            }
        } while (true);
#endif // !mutex
    }

    public:
    MemoryPool&     pool;
#ifdef MEMORYPOOLUSEMUTEX
    smsc::core::synchronization::Mutex lock;
#else
    smsc::core::synchronization::AtomicCounter<Fastest> serial_;
#endif
    Chunk*          freeChain;
    size_t          sz;
};


// ===================================================


MemoryPool::MemoryPool( const size_t* sz,
                        smsc::logger::Logger* l ) :
log_(l), ncaches_(0)
{
    assert(sz && *sz && *sz >= sizeof(void*) );
    size_t prevsz = 0;
    for ( const size_t* p = sz; *p; ++p ) {
        assert( *p > prevsz );
        prevsz = *p;
        ++ncaches_;
    }
    SizeCache* c = new SizeCache[ncaches_];
    for ( size_t i = 0; i < ncaches_; ++i ) {
        c[i].maxsize = sz[i];
        c[i].cache = 0;
    }
    if (log_) {
        smsc_log_info(log_,"ctor pool maxsize=%u ncaches=%u\n",
                      unsigned(prevsz),unsigned(ncaches_));
    }
    smsc::core::synchronization::MutexGuard mg(cacheLock_);
    caches_ = c;
}


MemoryPool::~MemoryPool()
{
    if (log_) {
        smsc_log_info(log_,"dtor pool maxsize=%u ncaches=%u\n",
                      unsigned(caches_[ncaches_-1].maxsize),
                      unsigned(ncaches_));
    }
    SizeCache* c = 0;
    {
        smsc::core::synchronization::MutexGuard mg(cacheLock_);
        std::swap(c,caches_);
    }
    for ( size_t i = 0; i < ncaches_; ++i ) {
        delete c[i].cache;
    }
    ncaches_ = 0;
    delete [] c;
}


void MemoryPool::clean()
{
    for ( size_t i = 0; i < ncaches_; ++i ) {
        MemoryCache* mc;
        {
            smsc::core::synchronization::MutexGuard mg(cacheLock_);
            mc = caches_[i].cache;
        }
        if (mc) {
            mc->clean();
        }
    }
}


void* MemoryPool::allocate(SizeCache* c, size_t realsz)
{
    if (!c->cache) {
        smsc::core::synchronization::MutexGuard mg(cacheLock_);
        if (!caches_) { throw BadAlloc(); }
        c->cache = new MemoryCache(*this,c->maxsize);
    }
    return c->cache->allocate(realsz);
}


void* MemoryPool::allocate( size_t sz )
{
    SizeCache* c = sizeToCache(sz);
    if (!c) {
        if (!c) {
            smsc::logger::Logger* l = log_;
            if (l) { smsc_log_debug(l,"allocate sz=%u bigsize",unsigned(sz)); }
            return ::operator new(sz); 
        }
    }
    return allocate(c,sz);
}


void MemoryPool::deallocate(void* ptr, size_t sz) throw()
{
    SizeCache* c = sizeToCache(sz);
    if (!c) {
        smsc::logger::Logger* l = log_;
        if (l) {
            smsc_log_debug(l,"deallocate sz=%u bigsize",unsigned(sz)); 
        }
        ::operator delete(ptr);
        return;
    }
    assert(c->cache);
    c->cache->deallocate(ptr);
}


MemoryPool::SizeCache* MemoryPool::sizeToCache( size_t size )
{
    if (!caches_) {
        throw BadAlloc();
    }
    SizeCache* p = std::lower_bound(caches_,
                                    caches_+ncaches_,
                                    size,
                                    SizeCache::CompareSize());
    if ( size_t(p-caches_) >= ncaches_ ) return 0;
    return p;
}


}
}
