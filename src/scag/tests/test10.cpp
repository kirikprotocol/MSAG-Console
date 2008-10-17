#include <getopt.h>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include "core/buffers/FastMTQueue.hpp"
#include "core/threads/Thread.hpp"
#include "scag/util/memory/MemoryPool.h"

static bool dbg = false;
static unsigned doyield = 0;
static unsigned maxproc = 0;
static unsigned bunchsize = 10;

using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;
using namespace smsc::core::threads;
using namespace scag::util::memory;

unsigned thrid()
{
    unsigned t = unsigned(::pthread_self()) % 1000;
    return t;
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
        return MemoryPool::Instance().allocate( sz );
    }
    static void* operator new( std::size_t sz, const std::nothrow_t& ) throw ()
    {
        ::abort();
        // return mdispatch().allocate(sz);
        return 0;
    }
    static void operator delete( void* p, std::size_t sz ) throw()
    {
        MemoryPool::Instance().deallocate(p,sz);
    }
    static void operator delete( void* p, const std::nothrow_t& ) throw ()
    {
        ::abort();
        // mdispatch().deallocate(p);
    }

private:
    typedef std::basic_string< char, std::char_traits< char >, StdAlloc<char> > string_type;

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
        if ( ::doyield && unsigned(queue_.Count()) > ::doyield ) Thread::Yield();
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
    MemoryPoolConfig::AllocType at(MemoryPoolConfig::OPNEW);

    struct option longopts[] = {
        { "help", 0, NULL, 'h' },
        { "verbose", 0, NULL, 'v' },
        { "alloctype", 1, NULL, 'a' },
        { "time", 1, NULL, 't' },
        { "workers", 1, NULL, 'w' },
        { "maxproc", 1, NULL, 'x' },
        { "bunch", 1, NULL, 'b' },
        { "yield", 1, NULL, 'y' },
        { NULL, 0, NULL, 0 }
    };

    do {
        int longindex;
        int r = getopt_long( argc, argv, "hva:t:w:x:b:y:", longopts, &longindex );
        if ( r == -1 ) break;
        switch (r) {
        case 'h' : {
            printf("%s [options]\n", argv[0]);
            printf(" -h | --help         This help\n" );
            printf(" -v | --verbose      Be verbose\n" );
            printf(" -a | --alloctype A  Use allocator A (choose from 'my','new','malloc')\n" );
            printf(" -t | --time TIME    Set execution time to TIME (sec)\n" );
            printf(" -w WORKERS\n" );
            printf(" --workers WORKERS   Set number of worker threads to WORKERS\n" );
            printf(" -x | --maxproc MAX  Set maximum processed items per worker to MAX (default is unlimited)\n" );
            printf(" -b | --bunch SIZE   Set bunchsize to SIZE (default is 100)\n" );
            printf(" -y | --yield        Do yield (default is no)\n" );
            ::exit(0);
            break;
        }
        case 'a' : {
            
            if ( strcmp(optarg,"my") == 0 ) {
                at = MemoryPoolConfig::SCAGDB;
            } else if ( strcmp(optarg,"new") == 0 ) {
                at = MemoryPoolConfig::OPNEW;
            } else if ( strcmp(optarg,"malloc") == 0 ) {
                at = MemoryPoolConfig::MALLOC;
            } else {
                fprintf(stderr, "wrong specification %s for allocation type\n", optarg );
                ::exit(-1);
            }
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

    {
        MemoryPoolConfig cfg;
        cfg.alloctype = at;
        MemoryPool::Instance().setConfig( cfg );
    }

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
        MemoryPool::Instance().print( pf );
    }
    unsigned processed = 0;
    for ( size_t i = 0; i < nwork; ++i ) {
        processed += workers[i]->processed();
        delete workers[i];
    }
    printf("%s alloc, %u work, %u msec, %u bunch: processed %u\n",
           at == MemoryPoolConfig::SCAGDB ? "my " :
           ( at == MemoryPoolConfig::OPNEW ? "new" : "mlc" ),
           nwork, testtime, ::bunchsize,
           processed );
    return 0;
}
