#include <getopt.h>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include "scag/util/Print.h"
#include "scag/util/singleton/Singleton2.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/buffers/FastMTQueue.hpp"
#include "core/threads/Thread.hpp"

static bool myalloc = false;
static bool dbg = false;
static unsigned doyield = 0;
static unsigned maxproc = 0;
static unsigned bunchsize = 10;

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using namespace smsc::core::threads;

unsigned thrid()
{
    unsigned t = unsigned(::pthread_self()) % 1000;
    return t;
}

class MemoryManager
{
public:
    MemoryManager( unsigned sz ) : objsize_(sz), count_(0) {}
    ~MemoryManager() {
        void* p;
        while ( queue_.Pop(p) ) {
            --count_;
            ::operator delete(p);
            if (dbg) printf( "%3.3u %p- sz=%u cnt=%u/%u\n", thrid(), p, objsize(), queue_.Count(), count_ );
        }
    }

    void print( scag::util::Print& pf ) const {
        pf.print( "manager@%p sz=%u has %u/%u elts", this, unsigned(objsize()), queue_.Count(), count_ );
    }

    void* allocate() {
        void* res;
        if ( queue_.Count() ) { // first w/o locking
            MutexGuard mg(mtx_);
            if ( queue_.Pop(res) ) {
                if (dbg) printf( "%3.3u %p* sz=%u cnt=%u/%u\n", thrid(), res, objsize(), queue_.Count(), count_ );
                return res;
            }
        }
        // allocate an object and return it
        res = ::operator new( objsize() );
        ++count_;
        if (dbg) printf( "%3.3u %p+ sz=%u cnt=%u/%u\n", thrid(), res, objsize(), queue_.Count(), count_ );
        return res;
    }

    void  deallocate( void* p ) {
        {
            MutexGuard mg(mtx_);
            queue_.Push( p );
        }
        if (dbg) printf( "%3.3u %p~ sz=%u cnt=%u/%u\n", thrid(), p, objsize(), queue_.Count(), count_ );
    }

    inline size_t objsize() const {
        return objsize_;
    }

private:
    MemoryManager();

private:
    // FastMTQueue queue_;
    size_t               objsize_;
    CyclicQueue< void* > queue_;
    Mutex                mtx_;
    unsigned             count_;
};


class MemoryDispatcher
{
private:
    static const unsigned char invalid = 255;

public:
    void* allocate( size_t sz ) {
        if ( sz <= invalid ) {
            MemoryManager* m = find( sz );
            return m->allocate();
        } else {
            return ::operator new(sz);
        }
    }
    void  deallocate( void* p, size_t sz ) {
        if ( sz <= invalid ) {
            MemoryManager* m = find( sz );
            m->deallocate( p );
        } else {
            ::operator delete(p);
        }
    }

    MemoryDispatcher() : 
    seq_(0),
    bufsize_(4),
    managers_( new MemoryManager*[bufsize_] ),
    count_(0)
    {
        for ( unsigned i = 0; i < invalid; ++i ) indices_[i] = invalid;
        for ( unsigned i = 0; i < bufsize_; ++i ) managers_[i] = 0;
    }

    ~MemoryDispatcher() {
        MutexGuard mg(mtx_);
        ++seq_;
        // dump statistics
        if (dbg) {
            scag::util::PrintFile pf( stdout );
            this->print( pf );
        }
        for ( unsigned i = 0; i < invalid; ++i ) indices_[i] = invalid;
        for ( ; count_ > 0; ) {
            delete managers_[--count_];
            managers_[count_] = 0;
        }
        delete [] managers_;
        managers_ = 0;
        ++seq_;
    }

    void print( scag::util::Print& pf ) const
    {
        pf.print( "dispatcher@%p has %u managers@%p", this, count_, managers_ );
        for ( unsigned i = 0; i < invalid; ++i ) {
            unsigned idx = indices_[i];
            if ( idx != invalid ) {
                managers_[idx]->print( pf );
            }
        }
    }

private:
    MemoryManager* find( size_t sz ) 
    {
        MemoryManager* res = 0;
        do {
            unsigned s = seq_;
            if ( (s % 2) ) break;
            assert( sz <= invalid );
            unsigned idx = unsigned(indices_[sz-1]);
            if ( idx == invalid ) break;
            MemoryManager* m = managers_[idx];
            if ( s != seq_ ) break;
            res = m;
        } while ( false );

        while ( res == 0 ) {
            // needs locking
            MutexGuard mg(mtx_);
            unsigned idx = unsigned(indices_[sz-1]);
            if ( idx != invalid ) {
                res = managers_[idx];
                break;
            }
            // not found
            unsigned newbufsize = ( count_ == bufsize_ ? 
                                    ( bufsize_ < 16 ? 16 : bufsize_*2 ) :
                                    bufsize_ );
            MemoryManager** newbuf = const_cast< MemoryManager** >(managers_);
            if ( newbufsize != bufsize_ ) {
                if (dbg) {
                    scag::util::PrintFile pf(stdout);
                    this->print(pf);
                }
                newbuf = new MemoryManager* [newbufsize];
                // if (dbg) printf("taking a new buffer @%p\n", newbuf );
                for ( unsigned i = 0; i < bufsize_; ++i ) {
                    newbuf[i] = managers_[i];
                }
                for ( unsigned i = bufsize_ ; i < newbufsize; ++i ) {
                    newbuf[i] = 0;
                }
                bufsize_ = newbufsize;
            }
            res = new MemoryManager( sz );
            newbuf[count_] = res;
            ++seq_;
            if ( newbuf != managers_ ) {
                std::swap( const_cast< MemoryManager**& >(managers_),newbuf);
                // if (dbg) printf( "deleting the old buffer @%p", newbuf );
                delete [] newbuf;
            }
            indices_[sz-1] = count_++;
            ++seq_;
            break;
        }
        assert( res->objsize() == sz );
        return res;
    }

private:
    volatile unsigned seq_;                // seqlock
    volatile unsigned char indices_[invalid];
    unsigned        bufsize_;       // the length of the mman buffer
    MemoryManager* volatile *managers_;    // the buffer of pointers to mmans

    unsigned        count_;         // current number of mmans
    Mutex           mtx_;           // write mutex
};


// --- singleton for MemoryDispatcher
inline unsigned GetLongevity( MemoryDispatcher* ) { return 0xffffffff; }
// bool mdispatchinit = false;
MemoryDispatcher& mdispatch() {
    /*
    if ( ! mdispatchinit ) {
        MutexGuard mg(mdispatchmtx);
        if ( ! mdispatchinit ) {
            
        }
    }
     */
    return scag2::util::singleton::SingletonHolder< MemoryDispatcher >::Instance();
}
// ---


template < class T >
class MyAlloc
{
public:
    // taken from josuttis book

    /// type definitions/
    typedef T        value_type;
    typedef T*       pointer;
    typedef const T* const_pointer;
    typedef T&       reference;
    typedef const T& const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;
    
    /// rebind allocator to type U/
    template <class U> struct rebind {
        typedef MyAlloc<U> other;
    };

    /// return address of values/
    pointer address (reference value) const {
        return &value;
    }
    const_pointer address (const_reference value) const {
        return &value;
    }

    //* constructors and destructor/
    /* - nothing to do because the allocator has no state/
     */
    MyAlloc() throw() {
    }
    MyAlloc(const MyAlloc&) throw() {
    }
    template <class U> MyAlloc (const MyAlloc<U>&) throw() {
    }
    ~MyAlloc() throw() {
    }

    /// return maximum number of elements that can be allocated/
    size_type max_size () const throw() {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    /// allocate but don't initialize num elements of type T/
    pointer allocate (size_type num, const void* = 0) {
        /// print message and allocate memory with global new/
        // std::cerr << "allocate " << num << " element(s)"
        // << " of size " << sizeof(T) << std::endl;
        // pointer ret = (pointer)(::operator new(num*sizeof(T)));
        // std::cerr << " allocated at: " << (void*)ret << std::endl;
        pointer ret;
        if ( ::myalloc ) {
            ret = (pointer) mdispatch().allocate( num*sizeof(T) );
        } else {
            ret = (pointer) ::operator new( num*sizeof(T) );
        }
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
        if ( ::myalloc ) {
            mdispatch().deallocate( (void*)p, num*sizeof(T) );
        } else {
            ::operator delete( (void*)p );
        }
    }

};

template < class T, class U >
    bool operator == ( const MyAlloc< T >& a, const MyAlloc< U >& b ) {
    return true;
}
template < class T, class U >
    bool operator != ( const MyAlloc< T >& a, const MyAlloc< U >& b ) {
    return false;
}


class A
{
public:
    A( const char* n ) : name_(n) {
        if (dbg) printf("%3.3u %p created elt=%s@%p\n", thrid(), this, name_.c_str(), name_.c_str() );
    }
    ~A() {
        if (dbg) printf("%3.3u %p destroy elt=%s@%p\n", thrid(), this, name_.c_str(), name_.c_str() );
    }
    static void* operator new( std::size_t sz ) throw ( std::bad_alloc )
    {
        if ( myalloc ) {
            return mdispatch().allocate(sz);
        } else {
            return ::operator new(sz);
        }
    }
    static void* operator new( std::size_t sz, const std::nothrow_t& ) throw ()
    {
        ::abort();
        // return mdispatch().allocate(sz);
        return 0;
    }
    static void operator delete( void* p, std::size_t sz ) throw()
    {
        if ( myalloc ) {
            mdispatch().deallocate(p,sz);
        } else {
            ::operator delete(p);
        }
    }
    static void operator delete( void* p, const std::nothrow_t& ) throw ()
    {
        ::abort();
        // mdispatch().deallocate(p);
    }

private:
    typedef std::basic_string< char, std::char_traits< char >, MyAlloc<char> > string_type;

private:
    string_type name_;
};


inline A* makeelt( size_t i ) {
    char buf[40];
    snprintf( buf, sizeof(buf), "elt%u", unsigned(i) );
    return new A(buf);
}


class Worker : public smsc::core::threads::Thread
{
public:
    Worker( unsigned id ) : id_(id), stopping_(true), stop_(true), producer_(0), processed_(0) {}
    ~Worker() {
        // printf( "worker #%u has %u elts in queue\n", id_, queue_.Count() );
    }
    void setProducer( Worker& w ) {
        producer_ = &w;
    }
    virtual int Execute();
    void Stop() {
        // printf( "worker #%u stop sent\n", id_ );
        bool trig = false;
        {
            if ( ! stopping_ ) {
                trig = true;
                stopping_ = true;
            }
        }
        if ( trig && producer_ ) producer_->Stop();
    }

    int WaitFor() {
        int ret = Thread::WaitFor();
        thread = 0;
        return ret;
    }

    bool hasStopped() {
        return stop_;
    }

    FastMTQueue< A* >& queue() {
        return queue_;
    }

    unsigned processed() const {
        return processed_;
    }

private:
    unsigned           id_;
    bool               stopping_;
    bool               stop_;
    Worker*            producer_;
    FastMTQueue< A* >  queue_;
    unsigned           processed_;
};


int Worker::Execute()
{
    stopping_ = false;
    stop_ = false;
    processed_ = 0;
    unsigned seed = ::pthread_self();
    while ( ! stop_ ) {

        if ( ::maxproc && processed_ > ::maxproc ) stopping_ = true;

        if ( ! stopping_ ) {
            unsigned r = unsigned(100*(rand_r(&seed)/(RAND_MAX+1.0)));
            for ( unsigned i = 0; i < r; ++i ) {
                queue_.Push( makeelt(i) );
            }
        } else {
            stop_ = true;
        }
        if ( ::doyield && queue_.Count() > ::doyield ) Thread::Yield();
        // reading all elements produced by the parent
        do {
            A* elt;
            bool notdone = ( stop_ && !producer_->hasStopped() );
            while ( producer_->queue().Pop(elt) ) {
                if (!stop_) ++processed_;
                delete elt;
            }
            if ( notdone ) continue;
            break;
        } while ( true );
    }
    return 0;
}


unsigned getu( const char* fail )
{
    char* endptr;
    unsigned u = ::strtoul( optarg, &endptr, 10 );
    if ( *endptr != '\0' ) {
        fprintf( stderr, "%s\n", fail );
        ::exit(-1);
    }
    return u;
}


int main( int argc, char** argv )
{
    unsigned testtime = 10000;
    unsigned nwork = 10;

    struct option longopts[] = {
        { "help", 0, NULL, 'h' },
        { "verbose", 0, NULL, 'v' },
        { "myalloc", 0, NULL, 'm' },
        { "time", 1, NULL, 't' },
        { "workers", 1, NULL, 'w' },
        { "maxproc", 1, NULL, 'x' },
        { "bunch", 1, NULL, 'b' },
        { "yield", 1, NULL, 'y' },
        { NULL, 0, NULL, 0 }
    };
    do {
        int longindex;
        int r = getopt_long( argc, argv, "hvmt:w:x:b:y:", longopts, &longindex );
        if ( r == -1 ) break;
        switch (r) {
        case 'h' : {
            printf("%s [options]\n", argv[0]);
            printf(" -h | --help        This help\n" );
            printf(" -v | --verbose     Be verbose\n" );
            printf(" -m | --myalloc     Use custom allocator (default is std alloc)\n" );
            printf(" -t | --time TIME   Set execution time to TIME (sec)\n" );
            printf(" -w WORKERS\n" );
            printf(" --workers WORKERS  Set number of worker threads to WORKERS\n" );
            printf(" -x | --maxproc MAX Set maximum processed items per worker to MAX (default is unlimited)\n" );
            printf(" -b | --bunch SIZE  Set bunchsize to SIZE (default is 100)\n" );
            printf(" -y | --yield       Do yield (default is no)\n" );
            ::exit(0);
            break;
        }
        case 'm' : {
            ::myalloc = true;
            break;
        }
        case 'b' : {
            ::bunchsize = ::getu("wrong bunch size specification");
            break;
        }
        case 't' : {
            testtime = ::getu("wrong time specification");
            break;
        }
        case 'v' : {
            ::dbg = true;
            break;
        }
        case 'w' : {
            nwork = ::getu("wrong worker threads specification");
            break;
        }
        case 'x' : {
            maxproc = ::getu("wrong maxproc specification");
            break;
        }
        case 'y' : {
            ::doyield = ::getu("wrong yield limit specification");
            break;
        }
        default : {
            fprintf( stderr, "unknown option\n");
            ::exit(-1);
        }
        }
    } while ( true );

    std::vector< Worker* > workers;
    workers.reserve(nwork);
    for ( size_t i = 0; i < nwork; ++i ) {
        workers.push_back( new Worker(i) );
        if ( i > 0 ) workers[i]->setProducer( * workers[i-1] );
    }
    workers[0]->setProducer( * workers.back() );

    for ( size_t i = 0; i < nwork; ++i ) {
        workers[i]->Start();
    }

    {
        EventMonitor x;
        MutexGuard mg(x);
        x.wait(testtime);
    }

    for ( size_t i = 0; i < nwork; ++i ) {
        workers[i]->Stop();
    }
    for ( size_t i = 0; i < nwork; ++i ) {
        workers[i]->WaitFor();
    }
    {
        scag::util::PrintFile pf( stderr );
        mdispatch().print( pf );
    }
    unsigned processed = 0;
    for ( size_t i = 0; i < nwork; ++i ) {
        processed += workers[i]->processed();
        delete workers[i];
    }
    printf("%s alloc, %u work, %u msec, %u bunch: processed %u\n",
           ::myalloc ? "my " : "std",
           nwork, testtime, ::bunchsize,
           processed );
    return 0;
}
