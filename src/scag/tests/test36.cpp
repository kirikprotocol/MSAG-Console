#include <stdlib.h>
#include <vector>
#include "informer/io/TmpBuf.h"
#include "informer/io/InfosmeException.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "core/buffers/FastMTQueue.hpp"
#include "logger/Logger.h"
#include "util/PtrDestroy.h"

using namespace eyeline::informer;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;

static const unsigned DBMMCHUNKLEN = 62;    // how many items per chunk
static const size_t   DBMMMAXSIZE  = 0x100; // max size of handled item
static const size_t   DBMMGENLOWMARK = 5;   // how many chunks in one list for gen to start

/// the struct holds a number of allocated pointers
struct MemoryChunk
{
    void*        ptr[DBMMCHUNKLEN];  // an array of chunk
    MemoryChunk* next;
    unsigned     incache;        // how many ptrs are in cache

    inline void* allocate() throw () {
        if (incache == 0) return 0;
        return ptr[incache--];
    }

    inline bool deallocate(void* p) throw() {
        if (incache >= DBMMCHUNKLEN) return false;
        ptr[incache++] = p;
        return true;
    }

    inline bool empty() const { return (incache == 0); }
    inline bool filled() const { return (incache >= DBMMCHUNKLEN); }

    void print( TmpBufBase<char>& buf, MemoryChunk* upto = 0 ) const {
        do {
            char tbuf[20];
            const int nch = sprintf(tbuf," %u",incache);
            if ( nch < 0 ) {
                throw ErrnoException(nch,"sprintf");
            }
            buf.append(tbuf,size_t(nch));
        } while ( next != upto );
    }
};


struct MemoryChunkOneSize
{
    unsigned     size;
    unsigned     chunks;  // how many chunks
    MemoryChunk* head;    // objects are taken from head
    MemoryChunk* tail;    // objects are returned to tail
    bool operator < ( const MemoryChunkOneSize& c ) const {
        return size < c.size;
    }
    bool operator < ( size_t sz ) const {
        return size < sz;
    }
    void print( TmpBufBase<char>& buf ) const {
        char tbuf[40];
        int nch = sprintf(tbuf,"sz=%u ch=%u [h:",size,chunks);
        if ( nch < 0 ) { throw ErrnoException(nch,"sprintf"); }
        buf.append(tbuf,size_t(nch));
        if (head) { head->print(buf,tail); }
        buf.append(" t:",3);
        if (tail && tail != head) { tail->print(buf,head); }
        buf.append("]",1);
    }
};


class MemoryDispatcher
{
    struct MemoryChunkMon {
        MemoryChunk* list;
        unsigned     chunks;
        bool          requested;
        EventMonitor mon;
        MemoryChunkMon() : list(0), chunks(0), requested(false) {}
        void print( TmpBufBase<char>& tbuf ) {
            char buf[100];
            sprintf(buf," req=%u ch=%u [",requested?1:0,chunks);
            tbuf.append(buf,strlen(buf));
            if (list) { list->print(tbuf); }
            tbuf.append("]",1);
        }
    };

    struct MemoryLimits {
        unsigned genlowmark;
        unsigned freehighmark;
    };

    inline void* allocate( size_t sz ) const {
        void* p = malloc(sz);
        smsc_log_debug(log_,"allocate(%u) = %p",unsigned(sz),p);
        return p;
    }

    inline void deallocate( void* ptr ) const {
        smsc_log_debug(log_,"deallocate(%p)",ptr);
        free(ptr);
    }

public:

    MemoryDispatcher() : stopping_(false), serial_(0)
    {
        limits_[0].genlowmark = 5;
        limits_[0].freehighmark = 200;
        limits_[1].genlowmark = 3;
        limits_[1].freehighmark = 20;
    }

    /// get the chunk filled with objects of size sz,
    /// or empty chunk if sz=0.
    MemoryChunk* getChunk( size_t sz )
    {
        smsc_log_debug(log_,"getChunk(%u)",unsigned(sz));
        MemoryChunk* ret = 0;
        MemoryChunkMon* mc = &chunks_[sz];
        bool requested = false;
        do {
            if ( sz ) {
                if (mc->list) {
                    MutexGuard mg(mc->mon);
                    if ( mc->list ) {
                        // take the first chunk
                        ret = mc->list;
                        mc->list = ret->next;
                        if ( --mc->chunks < limits_[1].genlowmark ) {
                            // wake up the main thread
                            requested = mc->requested = true;
                        }
                        if (log_->isDebugEnabled()) {
                            TmpBuf<char,2048> buf;
                            mc->print(buf);
                            smsc_log_debug(log_,"sz=%u %s",
                                           unsigned(sz),buf.get());
                        }
                        break;
                    }
                } else if ( !mc->requested ) {
                    MutexGuard mg(mc->mon);
                    if (!mc->requested) {
                        requested = mc->requested = true;
                    }
                }
                if (log_->isDebugEnabled()) {
                    TmpBuf<char,2048> buf;
                    mc->print(buf);
                    smsc_log_debug(log_,"sz=%u %s",unsigned(sz),buf.get());
                }
                // there is no filled chunks
                mc = &chunks_[0];
            }
            if (mc->list) {
                MutexGuard mg(mc->mon);
                if (mc->list) {
                    ret = mc->list;
                    mc->list = ret->next;
                    if ( --mc->chunks < limits_[0].genlowmark ) {
                        requested = mc->requested = true;
                    }
                    if (log_->isDebugEnabled()) {
                        TmpBuf<char,2048> buf;
                        mc->print(buf);
                        smsc_log_debug(log_,"sz=0 %s",buf.get());
                    }
                    break;
                }
            } else if (!mc->requested) {
                MutexGuard mg(mc->mon);
                if (!mc->requested) {
                    requested = mc->requested = true;
                }
            }
            if (log_->isDebugEnabled()) {
                TmpBuf<char,2048> buf;
                mc->print(buf);
                smsc_log_debug(log_,"sz=0 %s",buf.get());
            }

            // create/fill a new filled chunks
            ret = new MemoryChunk;
            ret->incache = 0;
            if ( sz ) {
                // FIXME: fast fill the chunk (only one item)
                ret->ptr[0] = allocate(sz);
                ++ret->incache;
            }
        } while (false);
        if (requested) {
            const long oldserial = serial_;
            const long newserial = serial_ + long(pthread_self());
            serial_ = newserial;
            smsc_log_debug(log_,"serial: %lu -> %lu",oldserial,newserial);
            mon_.notify();
        }
        ret->next = 0;
        if (log_->isDebugEnabled()) {
            TmpBuf<char,2048> buf;
            ret->print(buf);
            smsc_log_debug(log_,"ret=%p %s",ret,buf.get());
        }
        return ret;
    }


    /// take chunks filled with items of size sz (or empty, if sz=0) back
    void takeChunks( size_t sz,
                     MemoryChunk* head,
                     MemoryChunk* tail,
                     unsigned chunks )
    {
        if (log_->isDebugEnabled()) {
            TmpBuf<char,2048> buf;
            head->print(buf,tail);
            smsc_log_debug(log_,"takeChunks(sz=%u,ch=%u,[%s])",
                           unsigned(sz),chunks,buf.get());
        }
        MemoryChunkMon* mc = &chunks_[sz];
        MutexGuard mg(mc->mon);
        tail->next = mc->list;
        mc->list = head;
        mc->chunks += chunks;
        if ( mc->chunks > limits_[sz?1:0].freehighmark ) {
            mc->requested = true;
            serial_ += long(pthread_self());
            mon_.notify();
        }
        if (log_->isDebugEnabled()) {
            TmpBuf<char,2048> buf;
            mc->print(buf);
            smsc_log_debug(log_,"sz=%u %s",unsigned(sz),buf.get());
        }
    }


    // a thread which creates and fills necessary chunks
    int Execute()
    {
        while (!stopping_) {

            long serial = serial_;
            bool needmore = false;
            for ( unsigned sz = 0; sz < DBMMMAXSIZE; ++sz ) {
                MemoryChunkMon* mc = &chunks_[sz];
                if (!mc->requested) continue;
                MemoryLimits& ml = limits_[sz?1:0];
                {
                    if ( mc->chunks < ml.genlowmark*2 ) {
                        // take a few chunks from empty list
                        MemoryChunk* c = 0;
                        if ( sz ) {
                            MemoryChunkMon& me = chunks_[0];
                            if ( me.list ) {
                                if (log_->isDebugEnabled()) {
                                    TmpBuf<char,2048> buf;
                                    me.print(buf);
                                    smsc_log_debug(log_,"take from empty %s",buf.get());
                                }
                                MutexGuard mg(me.mon);
                                if ( me.list ) {
                                    c = me.list;
                                    me.list = c->next;
                                }
                            }
                        }
                        if (!c) {
                            c = new MemoryChunk;
                            smsc_log_debug(log_,"create a new chunk %p",c);
                            c->incache = 0;
                        }
                        // fill the chunk
                        if (sz) {
                            for ( unsigned i = 0; i < DBMMCHUNKLEN; ++i ) {
                                c->ptr[i] = allocate(sz);
                            }
                            c->incache = DBMMCHUNKLEN;
                            smsc_log_debug(log_,"chunk %p filled",c);
                        }
                        MutexGuard mg(mc->mon);
                        c->next = mc->list;
                        mc->list = c;
                        ++mc->chunks;
                        if (log_->isDebugEnabled()) {
                            TmpBuf<char,2048> buf;
                            mc->print(buf);
                            smsc_log_debug(log_,"after inc sz=%u %s",unsigned(sz),buf.get());
                        }
                    } else if ( mc->chunks > ml.freehighmark ) {
                        // free some chunks
                        MemoryChunk* c = 0;
                        {
                            MutexGuard mg(mc->mon);
                            if ( mc->list ) {
                                c = mc->list;
                                mc->list = c->next;
                                --mc->chunks;
                            }
                            if (log_->isDebugEnabled()) {
                                TmpBuf<char,2048> buf;
                                mc->print(buf);
                                smsc_log_debug(log_,"del from sz=%u %s",
                                               unsigned(sz),buf.get());
                            }
                        }
                        if ( c ) {
                            for ( unsigned i = c->incache; i != 0; ) {
                                deallocate( c->ptr[--i] );
                            }
                            c->incache = 0;
                            if (sz) {
                                MemoryChunkMon& me = chunks_[0];
                                MutexGuard mg(me.mon);
                                c->next = me.list;
                                me.list = c;
                                ++me.chunks;
                                if (log_->isDebugEnabled()) {
                                    TmpBuf<char,2048> buf;
                                    me.print(buf);
                                    smsc_log_debug(log_,"attached to sz=0 %s",buf.get());
                                }
                            } else {
                                smsc_log_debug(log_,"dtor chunk %p",c);
                                delete c;
                            }
                        }
                    }
                }
                smsc::core::threads::Thread::Yield();
            }
            if (needmore) { continue; }
            MutexGuard mg(mon_);
            if ( serial_ != serial ) continue;
            mon_.wait(1000);
        }
        return 0;
    }

private:
    EventMonitor          mon_;
    MemoryChunkMon        chunks_[DBMMMAXSIZE];
    MemoryLimits          limits_[2];
    bool                  stopping_;
    volatile long         serial_;
    smsc::logger::Logger* log_;
};


struct MemoryPerThread
{
    typedef std::vector<MemoryChunkOneSize> ChunkVector;

    void* allocate( size_t sz ) {
        if (sz >= DBMMMAXSIZE) return ::operator new(sz);
        ChunkVector::iterator i = std::lower_bound(chunks_.begin(),chunks_.end(),sz);
        MemoryChunkOneSize* cs;
        if (i == chunks_.end()) {
            i = chunks_.insert(i,MemoryChunkOneSize());
            cs = &*i;
            cs->size = sz;
            cs->head = cs->tail = disp_->getChunk(sz);
            cs->chunks = 1;
            cs->head->next = cs->head;
        } else {
            cs = &*i;
        }
        if ( cs->head->empty() ) {
            if ( cs->head->next->empty() ) {
                // next is also empty
                MemoryChunk* tmp = disp_->getChunk(sz);
                ++cs->chunks;
                tmp->next = cs->head->next;
                cs->head->next = tmp;
                cs->head = tmp;
                // cs->tail = tmp->next;
            }
        }
        return cs->head->allocate();
    }


    void deallocate( void* ptr, size_t sz )
    {
        if (sz >= DBMMMAXSIZE) return ::operator delete(ptr);
        ChunkVector::iterator i = std::lower_bound(chunks_.begin(),chunks_.end(),sz);
        MemoryChunkOneSize* cs;
        if (i == chunks_.end()) {
            i = chunks_.insert(i,MemoryChunkOneSize());
            cs = &*i;
            cs->size = sz;
            cs->head = cs->tail = disp_->getChunk(0);
            cs->chunks = 1;
            cs->head->next = cs->head;
        } else {
            cs = &*i;
        }
        if (cs->tail->filled()) {
            if (cs->tail->next->filled()) {
                MemoryChunk* tmp = disp_->getChunk(0);
                ++cs->chunks;
                tmp->next = cs->tail->next;
                cs->tail->next = tmp;
                cs->tail = tmp;
                // cs->head = tmp->next;
            }
        }
        cs->tail->deallocate(ptr);
    }

private:
    MemoryDispatcher* disp_;
    ChunkVector       chunks_;
};


struct Data
{
    static const int nsizes = 20;
    static const size_t sizes[nsizes];
    static smsc::logger::Logger* log_;

    Data( int i ) : data(0) {
        size_t sz = sizes[i%nsizes];
        if ( sz < 1 ) { sz = 16; }
        else if ( sz > 2000 ) { sz = 2000; }
        data = new char[ sz ];
        smsc_log_debug(log_,"ctor %p data(%u)=%p",this,unsigned(sz),data);
    }
    ~Data() {
        smsc_log_debug(log_,"dtor %p data=%p",this,data);
        delete [] data; 
    }
    char* data;
};

const int Data::nsizes;
smsc::logger::Logger* Data::log_ = 0;
const size_t Data::sizes[Data::nsizes] = {
    3, 6, 8, 2, 10,  22, 15, 8, 13, 8,
    6, 4, 2, 13, 8,  66, 99, 5, 8, 44
};


class Worker : public smsc::core::threads::Thread
{
public:
    Worker( const char* name,
            MemoryDispatcher& md,
            FastMTQueue<Data*>& queue ) :
    log_(smsc::logger::Logger::getInstance(name)),
    stopping_(false), md_(md), queue_(queue) {}
    virtual void stop() = 0;
protected:
    smsc::logger::Logger* log_;
    bool                  stopping_;
    MemoryDispatcher&     md_;
    FastMTQueue< Data* >& queue_;
};


class WThread : public Worker
{
public:
    WThread( MemoryDispatcher& md, FastMTQueue<Data*>& queue, int nruns ) :
    Worker("wrt",md,queue), nruns_(nruns) {}
    ~WThread() { WaitFor(); }
    void stop() { stopping_ = true; }
    int Execute() {
        smsc_log_debug(log_,"writer started");
        for ( int i = 0; i < nruns_; ++i ) {
            if ( stopping_ ) break;
            queue_.Push( new Data(i) );
        }
        smsc_log_debug(log_,"writer finished");
        return 0;
    }
private:
    int nruns_;
};


class RThread : public Worker
{
public:
    RThread( MemoryDispatcher& md, FastMTQueue<Data*>& queue ) :
    Worker("rdr",md,queue) {}
    ~RThread() { WaitFor(); }
    int Execute() {
        smsc_log_debug(log_,"reader started");
        while (true) {
            queue_.waitForItem();
            if (stopping_) break;
            Data* d;
            if (queue_.Pop(d)) {
                delete d;
            }
        }
        Data* d;
        while ( queue_.Pop(d) ) {
            delete d;
        }
        smsc_log_debug(log_,"reader finished");
        return 0;
    }
    void stop() {
        stopping_ = true;
        queue_.notify();
    }
};


int main()
{
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );
    Data::log_ = smsc::logger::Logger::getInstance("data");
    smsc::logger::Logger* mainlog = smsc::logger::Logger::getInstance("main");
    smsc_log_info(mainlog,"started");

    const size_t nth = 10;
    const int nruns = 1000;

    MemoryDispatcher     md;
    FastMTQueue< Data* > queue;

    typedef std::vector< Worker* > Workers;
    Workers wthreads, rthreads;

    wthreads.reserve(nth);
    rthreads.reserve(nth);
    for ( size_t i = 0; i < nth; ++i ) {
        rthreads.push_back( new RThread(md,queue));
        rthreads.back()->Start();
    }
    smsc_log_info(mainlog,"readers started");
    for ( size_t i = 0; i < nth; ++i ) {
        wthreads.push_back( new WThread(md, queue, nruns));
        wthreads.back()->Start();
    }
    smsc_log_info(mainlog,"writers started");
    for ( Workers::iterator i = wthreads.begin(), ie = wthreads.end();
          i != ie; ++i ) {
        (*i)->WaitFor();
    }
    smsc_log_info(mainlog,"writers finished");

    // all producers are stopped

    for ( Workers::iterator i = rthreads.begin(), ie = rthreads.end();
          i != ie; ++i ) {
        (*i)->stop();
    }
    smsc_log_info(mainlog,"readers notified");

    std::for_each( wthreads.rbegin(), wthreads.rend(), smsc::util::PtrDestroy() );
    std::for_each( rthreads.rbegin(), rthreads.rend(), smsc::util::PtrDestroy() );

    smsc_log_info(mainlog,"finish");
    return 0;
}
