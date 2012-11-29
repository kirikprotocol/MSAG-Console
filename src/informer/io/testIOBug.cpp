/* The command used to compile (on sunos):
   CC -D_REENTRANT -D_FILE_OFFSET_BITS=64 -m64 \
      -library=stlport4 -g -o testIOBug testIOBug.cpp -lpthread -lrt
*/

// This program is to emulate the corruption of the filesystem
// appeared during the operation of some real application.
//
// There are several writing threads in the program.
// They are writing record by record into a
// designated number of sequential files using write(2) syscall.
// They also issuing periodic fsync(2) to flush the files
// as it's done in the real application.
// 
// Each record content is a text uniquely determined by
// the start time of the file, the number of the record since the beginning
// of the file, and the number of the file.  The records ends
// with the LF (\n) byte.
// 
// After the file is written one of reading (checking) thread
// reopens the file in read-only mode and reads the file record by record.
// It compares the content of the record from the file with
// that calculated by the same algorithm mentioned above.
// 
// In case of discrepancy the program prints an error message and stops.
// 
// Also, to emulate the real application behaviour there are a number
// of threads which opens the files being written in read-only mode
// and read the whole file to the end w/o checks.
//
// The options of the program (default in parentheses):
//  --help         print the list of options
//  --wthreads  N  number of writing threads (100)
//  --files     N  number of simultaneous files being written (100)
//  --recsize   N  the size of the record (300)
//  --speed     N  number of bunchs per second per writing thread (200)
//  --rolltime  N  how many seconds to write the file (500)
//  --rthreads  N  number of reading (checking) threads (1)
//  --interread N  the speed of intermediate read (2000)
//  --bunchsize N  number of records per bunch (50)

#include <inttypes.h> // int64_t
#include <sys/time.h> // getimeofday
#include <time.h>     // gmtime_r
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>   // opendir
#include <assert.h>

typedef int64_t msectime_type;
typedef unsigned long long ulonglong;
typedef struct tm  tm_type;

// --- helper functions

/// gets current time in milliseconds
msectime_type currentTimeMillis()
{
    struct timeval tv;
    gettimeofday(&tv,0);
    return msectime_type(tv.tv_sec)*1000 + tv.tv_usec/1000;
}


/// convert millisecond time into a ulonglong with format: YYYYMMDDHHMMSSmmm
ulonglong timeToYmd( msectime_type tmp, tm_type* tmb = 0 )
{
    tm_type tx;
    if (!tmb) tmb = &tx;
    if (!tmp) return 0;
    const time_t t(tmp/1000);
    gmtime_r(&t,tmb);
    return ( ( ( ( ( ulonglong(tmb->tm_year+1900) * 100 +
                     tmb->tm_mon+1 ) * 100 +
                   tmb->tm_mday ) * 100 +
                 tmb->tm_hour ) * 100 +
               tmb->tm_min ) * 100 +
             tmb->tm_sec ) * 1000 + tmp % 1000;
}


typedef struct stat stat_type;
/// delete the directory and all its subdirectory.
/// NOTE: the directory must exists.
/// NOTE: this function is not thread-safe
bool rmdirs( const char* dirname )
{
    assert(dirname && strlen(dirname) > 0);
    DIR* dirp = opendir(dirname);
    if (!dirp) {
        fprintf(stderr,"cannot opendir %s, errno=%u\n",dirname,errno);
        return false;
    }
    struct DirGuard {
        DirGuard(DIR* d) : dp(d) {}
        ~DirGuard() { closedir(dp); }
        DIR* dp;
    } dirguard(dirp);
    dirent* dp;
    std::string dirpfx(dirname);
    if ( dirpfx[dirpfx.size()-1] != '/' ) {
        dirpfx += '/';
    }
    while ( (dp = readdir(dirp)) ) {
        if ( 0 != strcmp(dp->d_name,".") &&
             0 != strcmp(dp->d_name,"..") ) {
            const std::string fn = dirpfx + dp->d_name;
            stat_type st;
            if ( -1 == stat(fn.c_str(),&st) ) {
                fprintf(stderr,"cannot stat %s, errno=%u\n",fn.c_str(),errno);
                return false;
            }
            if ( 0 != (S_IFDIR & st.st_mode) ) {
                // directory
                if (!rmdirs(fn.c_str())) {
                    fprintf(stderr,"cannot rmdirs %s\n",fn.c_str());
                    return false;
                }
            } else if ( -1 == ::unlink(fn.c_str()) ) {
                fprintf(stderr,"cannot unlink %s, errno=%u\n",fn.c_str(),errno);
                return false;
            }
        }
    }
    if ( -1 == ::rmdir(dirname) ) {
        fprintf(stderr,"cannot rmdir %s, errno=%u\n",dirname,errno);
        return false;
    }
    return true;
}


// --- the list of words to be combined into a record
const char* vocabulary[] = {
#include "words.icc"
0
};



// --- helper classes

class Condition;

/// a wrapper around pthread_mutex_t, locking primitive
class Mutex
{
    friend class Condition;
public:
    Mutex() {
        pthread_mutex_init(&mutex_,0);
    }
    ~Mutex() {
        pthread_mutex_destroy(&mutex_);
    }
    void lock() {
        pthread_mutex_lock(&mutex_);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }
private:
    Mutex( const Mutex& );
    Mutex& operator = ( const Mutex& );
private:
    pthread_mutex_t mutex_;
};


/// a condition class to be used in EventMonitor, see below
class Condition
{
public:
    Condition() {
        pthread_cond_init(&event_,0);
    }
    ~Condition() {
        pthread_cond_destroy(&event_);
    }

    void notify() {
        pthread_cond_signal(&event_);
    }

    /// mutex must be locked
    void wait( Mutex& m ) {
        pthread_cond_wait(&event_,&m.mutex_);
    }

    /// mutex must be locked
    /// return true if event has occured
    bool wait( Mutex& m, msectime_type abstime ) {
        struct timespec ts;
        ts.tv_sec = abstime / 1000;
        ts.tv_nsec = (abstime % 1000) * 1000000;
        const int r = pthread_cond_timedwait(&event_,&m.mutex_,&ts);
        return (r == 0);
    }

private:
    Condition( const Condition& );
    Condition& operator = ( const Condition& );
private:
    pthread_cond_t event_;
};


/// event monitor class for convenient wait for
/// notification from a different thread.
class EventMonitor : public Mutex
{
public:
    EventMonitor() {}
    void notify() {
        cond_.notify();
    }
    void wait() {
        cond_.wait(*this);
    }
    bool wait( msectime_type abstime ) {
        return cond_.wait(*this,abstime);
    }
private:
    EventMonitor( const EventMonitor& );
    EventMonitor& operator = ( const EventMonitor& );
private:
    Condition cond_;
};


/// locking primitive guaranteeing unlock after leaving scope
class MutexGuard
{
public:
    MutexGuard( Mutex& m ) : m_(m) {
        m_.lock();
    }
    ~MutexGuard() {
        m_.unlock();
    }
private:
    MutexGuard( const MutexGuard& );
    MutexGuard& operator = ( const MutexGuard& );
private:
    Mutex& m_;
};


/// cyclic queue
template <typename T> class CyclicQueue
{
public:
    CyclicQueue() : data_(0), head_(0), tail_(0), eod_(0), count_(0) {}

    ~CyclicQueue() {
        if (data_) { delete [] data_; }
    }

    T& push( const T& t ) 
    {
        if (count_ >= size_t(eod_ - data_)) {
            resize( count_ * 2 );
        }
        if ( head_ >= eod_ ) {
            head_ = data_;
        }
        T& ret = *head_ = t;
        ++head_;
        ++count_;
        return ret;
    }


    bool pop( T& t ) 
    {
        if (!count_) return false;
        if (tail_==eod_) {
            tail_ = data_;
        }
        t = *tail_;
        ++tail_;
        --count_;
        return true;
    }


    bool empty() const { return !count_; }

    void resize( size_t sz ) 
    {
        if ( sz == 0 ) { sz = 16; }
        if ( sz < count_ ) return;
        T* nd = new T[sz];
        head_ = nd;
        for ( size_t i = count_; i > 0; --i ) {
            if ( tail_ == eod_ ) {
                tail_ = data_;
            }
            *head_ = *tail_;
            ++head_;
            ++tail_;
        }
        tail_ = nd;
        delete [] data_;
        data_ = nd;
        eod_ = data_ + sz;
    }

private:
    T*     data_;
    T*     head_;
    T*     tail_;
    T*     eod_;
    size_t count_;
};



extern "C" void* threadRunner( void* obj );

/// a base class representing a thread of execution.
class Thread
{
    friend void* threadRunner(void*);
public:
    virtual ~Thread() {}

    /// start the thread
    void start() {
        if (tid_) return;
        if ( 0 != pthread_create(&tid_,0,&threadRunner,this) ) {
            // tid_ = 0;
            abort();
        }
    }

    /// wait until the thread finishes.
    void join() {
        if (!tid_) return;
        // printf("joining thread %lu\n",long(tid_));
        if ( 0 != pthread_join(tid_,0) ) {
            fprintf(stderr,"thread cannot be joined, errno=%u\n",errno);
            abort();
        }
        tid_ = 0;
    }


    /// issue the cancel signal to the thread.
    void cancel() {
        if (!tid_) return;
        // printf("cancelling thread %lu\n",long(tid_));
        if ( 0 != pthread_cancel(tid_) ) {
            fprintf(stderr,"thread cannot be cancelled, errno=%d\n",errno);
            abort();
        }
        // printf("cancelled thread %lu\n",long(tid_));
        tid_ = 0;
    }

protected:
    Thread() : tid_(0) {}
    virtual void execute() = 0;

private:
    pthread_t tid_;
};


extern "C" void* threadRunner( void* obj )
{
    Thread* t = reinterpret_cast< Thread* >(obj);
    t->execute();
    return 0;
};


/// thread with a stop() method.
class StoppableThread : public Thread
{
public:
    virtual void stop() = 0;
};


/// a group of threads.
class ThreadGroup
{
public:
    ThreadGroup() {}
    ~ThreadGroup() { stop(); }

    /// start thread taking ownership
    void startThread(StoppableThread* t )
    {
        if (!t) return;
        threads_.push_back(t);
        t->start();
    }

    void stop() {
        for ( std::vector< StoppableThread* >::iterator i = threads_.begin(),
              ie = threads_.end(); i != ie; ++i ) {
            (*i)->stop();
        }
        for ( std::vector< StoppableThread* >::iterator i = threads_.begin(),
              ie = threads_.end(); i != ie; ++i ) {
            delete *i;
        }
        threads_.clear();
    }

private:
    std::vector< StoppableThread* > threads_;
};


/// I am not sure if random() is thread-safe so wrap it here
Mutex randMutex;
long getRandom()
{
    MutexGuard mg(randMutex);
    return random();
}


/// --- the components of the program

/// the configuration of the program
struct Config
{
    Config() :
        store( "./testiobug-store/" ),
        rthreads(1), wthreads(100), nfiles(100),
        speed(200), rolltime(500), recsize(300),
        nwords(0), maxword(0), 
        interspeed(2000),
        bunchsize(50),
        fsyncrate(17),
        records_(0),
        start_(currentTimeMillis()),
        lastprint_(start_)
    {
        for ( const char** p = vocabulary; *p; ++p ) {
            const size_t wsz = strlen(*p);
            if ( wsz > maxword ) maxword = wsz;
            ++nwords;
        }
        if ( !nwords ) {
            fprintf(stderr,"no words supplied\n");
            abort();
        }
    }


    /// checks the speed and consume quant of work.
    /// @return 0 or the time until what to wait.
    /// also keep the statistics on the number of bunches written.
    msectime_type consumeQuant( msectime_type now )
    {
        size_t recs = 0;
        size_t needrec;
        msectime_type elapsed;
        bool needprint = false;
        {
            MutexGuard mg(lock_);
            elapsed = now - start_;
            needrec = speed * elapsed / 1000;
            if ( records_ > needrec ) {
                return start_ + records_*1000/speed;
            }
            recs = ++records_;
            if ( now - lastprint_ > 10000 ) {
                needprint = true;
                lastprint_ = now;
            }
        }
        if ( needprint ) {
            printf("elapsed time %u sec, %llu records (%5.1f%%) written\n",
                   unsigned(elapsed/1000),
                   ulonglong(recs)*bunchsize,
                   double(recs*1000/needrec)/10);
        }
        return 0;
    }

public:
    std::string store;
    unsigned    rthreads;   // how many reading threads
    unsigned    wthreads;   // how many writing threads
    unsigned    nfiles;     // number of files being written simultaneously
    unsigned    speed;      // speed to write, bunches/sec
    unsigned    rolltime;   // time to roll files, sec
    unsigned    recsize;    // record size hint (actual size may be a little bigger)
    unsigned    nwords;     // number of words in the vocabulary
    size_t      maxword;    // the length of the longest word
    unsigned    interspeed; // the speed of interread
    unsigned    bunchsize;  // the number of records in a bunch
    unsigned    fsyncrate;  // fsync per how many bunches

private:
    Mutex         lock_;
    size_t        records_;
    msectime_type start_;
    msectime_type lastprint_;
};


/// the configuration is kept in this instance
Config cfg;


/// the identifier of the file (starttime + number of the directory)
struct FileId
{
    FileId() {}
    FileId( unsigned dirid, msectime_type st ) : dirId(dirid), startTime(st) {}

    unsigned dirId;
    msectime_type startTime;
};


/// a class to arrange interaction b/w workdir and reading queues
class ReadingQueue
{
public:
    ReadingQueue() : stopping_(false) {}

    void push( const FileId& fi ) {
        MutexGuard mg(mon_);
        if (stopping_) {
            return;
        }
        queue_.push(fi);
        mon_.notify();
        // printf("file (%u,%llu) pushed\n",
        // fi.dirId,timeToYmd(fi.startTime));
    }


    bool pop( FileId& fi )
    {
        do {
            MutexGuard mg(mon_);
            if ( !queue_.empty() ) {
                const bool res = queue_.pop(fi);
                // printf("file (%u,%llu) popped\n",
                // fi.dirId, timeToYmd(fi.startTime));
                return res;
            }
            if (stopping_) return false;
            const msectime_type now = currentTimeMillis();
            // printf("waiting for a new file\n");
            mon_.wait(now+2000);
        } while (true);
    }

    void stop() {
        MutexGuard mg(mon_);
        stopping_ = true;
        mon_.notify();
    }

private:
    EventMonitor mon_;
    CyclicQueue< FileId > queue_;
    bool stopping_;
};


/// a directory which has an identification number.
/// files being written are opened in the directory.
/// after the file is finished it is passed via readingqueue
/// into one of reading thread.
class WorkDir
{
public:
    explicit WorkDir( ReadingQueue& rq, unsigned id ) :
        id_(id), rq_(&rq), fd_(-1), wbuf_(0) 
    {
        char dbuf[30];
        sprintf(dbuf,"%06u",id);
        std::string dirname = cfg.store + dbuf;
        if (-1 == mkdir(dirname.c_str(),0755) ) {
            fprintf(stderr,"cannot mkdir %s: %d\n",dirname.c_str(),errno);
            abort();
        }
    }

    ~WorkDir() {
        if (wbuf_) { delete [] wbuf_; }
    }


    /// construct a file name by its start time.
    const std::string& makeFilePath( std::string& st, msectime_type start ) const
    {
        char buf[100];
        snprintf(buf,sizeof(buf),"%06u/%010llu.log",id_,timeToYmd(start));
        st = cfg.store;
        st.append(buf);
        return st;
    }


    /// construct a record based on file start time and record number.
    /// @param start  the start time of the file
    /// @param recnum the record number
    /// @param b      is the buffer to hold the contents of the buffer,
    ///               NOTE: it must be large enough.
    /// @return the size of the filled buffer
    size_t makeRecord( msectime_type start, size_t recnum,
                       char* b )
    {
        msectime_type p = start + id_*recnum;
        size_t pos = size_t(sprintf(b,"%u",unsigned(recnum)));
        b += pos;
        do {
            *b++ = ' ';
            ++pos;
            const size_t nw = size_t(p) % cfg.nwords;
            const char* word = vocabulary[nw];
            const size_t wsz = strlen(word);
            memcpy(b,word,wsz);
            b += wsz;
            pos += wsz;
            if ( pos >= cfg.recsize ) {
                *b++ = '\n';
                ++pos;
                *b = '\0';
                break;
            }
            p += start + recnum;
        } while ( true );
        return pos;
    }


    /// issue fsync(2) on the opened file
    void fsync()
    {
        MutexGuard mg(writeLock_);
        if (fd_ != -1) {
            ::fsync(fd_);
        }
    }


    /// add a record into the file being written.
    /// 1. if the file is not opened then create it and reset the record counter;
    /// 2. construct a record;
    /// 3. write the record;
    /// 4. if the file write time is expired the close the file.
    void addRecord()
    {
        {
            MutexGuard mg(writeLock_);

            const msectime_type now = currentTimeMillis();

            if ( fd_ == -1 ) {
                // open a new file
                std::string fn;
                fd_ = open(makeFilePath(fn,now).c_str(),O_WRONLY|O_CREAT|O_TRUNC,0644);
                if (-1 == fd_) {
                    fprintf(stderr,"cannot create file %s: %d\n",fn.c_str(),errno);
                    abort();
                }
                fstart_ = now;
                recnum_ = 0;
            }

            if (!wbuf_) {
                wbuf_ = new char[cfg.recsize+cfg.maxword*3+80];
            }
            const size_t towrite = makeRecord(fstart_,recnum_,wbuf_);
            const ssize_t written = write(fd_,wbuf_,towrite);
            if ( written == -1 ) {
                fprintf(stderr,"write failed fd=%d towrite=%llu errno=%u\n",
                        fd_,ulonglong(towrite),errno);
                abort();
            }
            if ( towrite != size_t(written) ) {
                fprintf(stderr,"write failed towrite=%llu written=%llu\n",
                        ulonglong(towrite), ulonglong(written));
                abort();
            }
            ++recnum_;
            if ( now >= fstart_ + cfg.rolltime*1000LL ) {
                rollFile();
            }
        }
    }


    /// issue a stop to finish current file
    void stop()
    {
        MutexGuard mg(writeLock_);
        rollFile();
    }


    /// read the file optionally checking it.
    /// @param fstart the starting time of the file.
    /// @param speed  the limit on reading speed (records/sec) or 0 for unlimited.
    /// @param check  if true then check each record contents.
    size_t readTheFile( msectime_type fstart,
                        unsigned  speed,
                        bool check = true )
    {
        std::string fn;
        int fd = open(makeFilePath(fn,fstart).c_str(),O_RDONLY);
        if ( fd == -1 ) {
            fprintf(stderr,"cannot ropen file %s: %d\n",
                    fn.c_str(),errno);
            abort();
        }
        // printf("reading the file %s\n",fn.c_str());
            
        const size_t rsize = std::min(cfg.recsize*16,4096U) & 0xffffff00U;
        char* rbuf = new char[rsize];
        char* record = new char[cfg.recsize+cfg.maxword*3+80];
        
        size_t pos = 0;
        size_t recnum = 0;
        const msectime_type startTime = currentTimeMillis();
        do {
            const size_t rsz = rsize - pos;
            if (rsz == 0) {
                fprintf(stderr,"too small buffer\n");
                abort();
            }
            const ssize_t wasread = read(fd,rbuf+pos,rsz);
            if ( -1 == wasread ) {
                fprintf(stderr,"cannot read from %s: %d\n",
                        fn.c_str(),errno);
                abort();
            } else if ( 0 == wasread ) {
                break;
            }
            pos += wasread;

            char* p = rbuf;
            while ( pos > 0 ) {
                char* eol = reinterpret_cast<char*>(memchr(p,'\n',pos));
                if ( !eol ) {
                    break;
                }
                ++eol;
                const size_t recsz = eol - p;

                if ( check ) {
                    // compare records
                    const size_t origlen = makeRecord(fstart,recnum,record);
                    if ( recsz != origlen ) {
                        fprintf(stderr,"DISCREPANCE FOUND: file %s rec=%llu len=%llu origlen=%llu rec='%.*s' origrec='%.*s'\n",
                                fn.c_str(),ulonglong(recnum),
                                ulonglong(recsz),
                                ulonglong(origlen),
                                unsigned(recsz), p,
                                unsigned(origlen), record);
                        exit(-2);
                    }
                    if ( memcmp(p,record,recsz) != 0 ) {
                        fprintf(stderr,"DISCREPANCE FOUND: file %s rec=%llu diff contents rec='%.*s' origrec='%.*s'\n",
                                fn.c_str(),ulonglong(recnum),
                                unsigned(recsz), p,
                                unsigned(origlen), record);
                        exit(-2);
                    }
                }

                ++recnum;
                while ( speed ) {
                    EventMonitor mon;
                    const msectime_type now = currentTimeMillis();
                    const size_t needrec = (now - startTime)*speed/1000;
                    if ( recnum <= needrec ) {
                        break;
                    }
                    MutexGuard mg(mon);
                    mon.wait(startTime + needrec*1000/speed);
                }
                pos -= recsz;
                p = eol;
            }

            if ( pos > 0 ) {
                // move the tail
                char* o = rbuf;
                for ( size_t i = 0; i < pos; ++i ) {
                    *o++ = *p++;
                }
            }

        } while (true);

        delete [] rbuf;
        delete [] record;
        close(fd);
        if ( pos > 0 && check ) {
            fprintf(stderr,"file %s is GARBLED at the tail\n",fn.c_str());
            exit(-2);
        }
        return recnum;
    }


    /// perform reading w/o check of file being written.
    void interRead()
    {
        msectime_type fstart;
        {
            MutexGuard mg(writeLock_);
            if (-1 == fd_) { return; }
            fstart = fstart_;
        }
        std::string fn;
        // printf("interreading %s\n",makeFilePath(fn,fstart).c_str());
        // const size_t recs = 
        readTheFile(fstart,cfg.interspeed,false);
        // printf("file %s interread records=%llu\n",fn.c_str(),ulonglong(recs));
    }


private:
    /// close current file.
    void rollFile()
    {
        if ( fd_ != -1 ) {
            std::string fn;
            printf("file %s written records=%llu\n",
                   makeFilePath(fn,fstart_).c_str(),
                   ulonglong(recnum_));
            close(fd_);
            fd_ = -1;
            rq_->push( FileId(id_,fstart_) );
        }
    }


private:
    Mutex         writeLock_;
    unsigned      id_;
    ReadingQueue* rq_;
    msectime_type fstart_;  // a time when the file was started
    int           fd_;      // current file id or -1
    size_t        recnum_;  // record number in current file
    char*         wbuf_;
};


/// working directories
std::vector<WorkDir*> workdirs;


/// writing thread class.
class WritingThread : public StoppableThread
{
public:
    WritingThread() : stopping_(false) {}
    ~WritingThread() { join(); }


    void execute()
    {
        unsigned pass = 0;
        printf("wthread started\n");
        while ( !stopping_ ) {
            const msectime_type now = currentTimeMillis();
            const msectime_type till = cfg.consumeQuant(now);
            if ( till ) {
                // have to wait
                MutexGuard mg(mon_);
                mon_.wait(till);
                continue;
            }
            const size_t id = (now + getRandom()) % cfg.nfiles;
            for ( unsigned i = 0; i < cfg.bunchsize; ++i ) {
                workdirs[id]->addRecord();
            }
            if ( ++pass % cfg.fsyncrate == 0 ) {
                workdirs[id]->fsync();
            }
        }
        printf("wthread finished\n");
    }


    void stop() {
        MutexGuard mg(mon_);
        stopping_ = true;
        mon_.notify();
    }

private:
    EventMonitor mon_;
    bool         stopping_;
};


/// reading (checking) thread class.
class ReadingThread : public StoppableThread
{
public:
    ReadingThread( ReadingQueue& rq ) : rq_(rq) {}
    ~ReadingThread() {
        join(); 
    }

    void execute() {
        printf("rthread started\n");
        FileId fi;
        while ( rq_.pop(fi) ) {
            WorkDir* wd = workdirs[fi.dirId];
            const size_t recs = wd->readTheFile(fi.startTime,0,true);
            std::string fn;
            printf("file %s checked records=%llu\n",
                   wd->makeFilePath(fn,fi.startTime).c_str(),
                   ulonglong(recs));
        }
        printf("rthread finished\n");
    }

    void stop() {}

private:
    ReadingQueue& rq_;
};


/// intermediate reading thread class.
class InterReadThread : public StoppableThread
{
public:
    InterReadThread() : stopping_(false) {}

    ~InterReadThread() { join(); }

    void execute()
    {
        printf("interread started\n");
        while ( !stopping_ ) {
            const msectime_type now = currentTimeMillis();
            {
                MutexGuard mg(mon_);
                mon_.wait(now+200);
            }
            const size_t id = (now + getRandom()) % cfg.nfiles;
            workdirs[id]->interRead();
        }
        printf("interread finished\n");
    }


    void stop()
    {
        printf("stopping interread\n");
        {
            MutexGuard mg(mon_);
            stopping_ = true;
            mon_.notify();
        }
        cancel();
    }


private:
    EventMonitor mon_;
    bool stopping_;
};


/// program stop primitives:
EventMonitor stopmon;       // a monitor to wait for
bool stopping = false;      // the flag which is set from signal handler

// The stopping file name.
// If this file appears in the current directory the program stops.
// I needed the approach because valgrind on **** macos coredumps on any signals.
const char* stopfile = "testiobug.stop";

/// the signal handler which sets the stopping flag.
extern "C" void sigHandler(int signo)
{
    printf("signal received %d\n",signo);
    MutexGuard mg(stopmon);
    stopping = true;
    stopmon.notify();
}


void registerSignalHandler()
{
    sigset_t st;
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigfillset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, NULL);

    sigfillset(&st);
    sigdelset(&st, SIGBUS);
    sigdelset(&st, SIGFPE);
    sigdelset(&st, SIGILL);
    sigdelset(&st, SIGSEGV);
    sigdelset(&st, SIGINT);
    sigdelset(&st, SIGHUP);  
    sigprocmask(SIG_SETMASK, &st, NULL);

    sigdelset(&st, SIGUSR2);
    sigdelset(&st, SIGALRM);
    sigdelset(&st, SIGABRT);
    sigdelset(&st, SIGTERM);

    sigset(SIGUSR2, sigHandler);
    sigset(SIGFPE,  sigHandler);
    sigset(SIGILL,  sigHandler);
    signal(SIGABRT, sigHandler);
    sigset(SIGALRM, sigHandler);
    sigset(SIGTERM, sigHandler);
    sigset(SIGINT,  sigHandler);
}


int main( int argc, const char** argv )
{
    // --- parsing options
    for ( const char** p = argv+1; *p; ++p ) {
        const std::string arg(*p);
        if ( arg == "--wthreads" ) {
            cfg.wthreads = atoi(*++p);
            if ( cfg.wthreads < 1 ) cfg.wthreads = 1;
            else if ( cfg.wthreads > 200 ) cfg.wthreads = 200;
        } else if ( arg == "--rthreads" ) {
            cfg.rthreads = atoi(*++p);
            if ( cfg.rthreads < 1 ) cfg.rthreads = 1;
            else if ( cfg.rthreads > 200 ) cfg.rthreads = 200;
        } else if ( arg == "--files" ) {
            cfg.nfiles = atoi(*++p);
            if ( cfg.nfiles < 10 ) cfg.nfiles = 10;
            else if ( cfg.nfiles > 100000 ) cfg.nfiles = 100000;
        } else if ( arg == "--recsize" ) {
            cfg.recsize = atoi(*++p);
            if ( cfg.recsize < 50 ) cfg.recsize = 50;
            else if ( cfg.recsize > 10000 ) cfg.recsize = 10000;
        } else if ( arg == "--speed" ) {
            cfg.speed = atoi(*++p);
            if ( cfg.speed < 1 ) cfg.speed = 1;
            else if ( cfg.speed > 1000000 ) cfg.speed = 1000000;
        } else if ( arg == "--rolltime" ) {
            cfg.rolltime = atoi(*++p);
            if ( cfg.rolltime < 30 ) cfg.rolltime = 30;
            else if ( cfg.rolltime > 2000 ) cfg.rolltime = 2000;
        } else if ( arg == "--interread" ) {
            cfg.interspeed = atoi(*++p);
            if ( cfg.interspeed > 1000000 ) cfg.interspeed = 1000000;
        } else if ( arg == "--bunchsize" ) {
            cfg.bunchsize = atoi(*++p);
            if ( cfg.bunchsize < 1 ) cfg.bunchsize = 1;
        } else if ( arg == "--fsyncrate" ) {
            cfg.fsyncrate = atoi(*++p);
            if ( cfg.fsyncrate < 1 ) cfg.fsyncrate = 1;
        } else if ( arg == "--help" || arg == "-h" ) {
            fprintf(stderr,"Options:\n"
                    "  --wthreads  Number of writing threads [%u]\n"
                    "  --rthreads  Number of reading threads [%u]\n"
                    "  --files     Number of files [%u]\n"
                    "  --recsize   Size of the record, bytes [%u]\n"
                    "  --speed     Speed per thread, bunch/sec [%u]\n"
                    "  --rolltime  Time between file rolling, sec [%u]\n"
                    "  --interread Intermediate read speed [%u]\n"
                    "  --bunchsize Number of records per bunch [%u]\n"
                    "  --fsyncrate Number of bunches per fsync [%u]\n",
                    cfg.wthreads, cfg.rthreads, cfg.nfiles,
                    cfg.recsize, cfg.speed, cfg.rolltime,
                    cfg.interspeed, cfg.bunchsize, cfg.fsyncrate
                    );
            exit(0);
        } else {
            fprintf(stderr,"unknown option %s",arg.c_str());
            exit(-1);
        }
    }

    printf("rthreads=%u wthreads=%u files=%u recsize=%u speed=%u rolltime=%u interread=%u bunchsize=%u fsyncrate=%u\n",
           cfg.rthreads, cfg.wthreads, cfg.nfiles, cfg.recsize, cfg.speed,
           cfg.rolltime, cfg.interspeed, cfg.bunchsize, cfg.fsyncrate );

    // check the working directory
    stat_type st;
    if ( 0 == stat(cfg.store.c_str(),&st) ) {
        // the directory already exists
        printf("deleting directory %s\n",cfg.store.c_str());
        if ( !rmdirs(cfg.store.c_str()) ) {
            return -1;
        }
    }

    printf("creating directory %s\n",cfg.store.c_str());
    if ( -1 == mkdir(cfg.store.c_str(),0755) ) {
        fprintf(stderr,"cannot create testing path %s, errno=%u\n",cfg.store.c_str(),errno);
        return -1;
    }

    registerSignalHandler();

    ReadingQueue readingQueue;

    // --- creating all workdirs
    workdirs.reserve(cfg.nfiles);
    for ( unsigned i = 0; i < cfg.nfiles; ++i ) {
        workdirs.push_back( new WorkDir(readingQueue,i) );
    }

    ThreadGroup rtg;

    // --- creating reading threads
    for ( unsigned i = 0; i < cfg.rthreads; ++i ) {
        rtg.startThread( new ReadingThread(readingQueue) );
    }

    ThreadGroup wtg;
    // --- creating writing threads
    for ( unsigned i = 0; i < cfg.wthreads; ++i ) {
        wtg.startThread( new WritingThread() );
    }
    if ( cfg.interspeed ) {
        wtg.startThread( new InterReadThread() );
    }

    printf("entering wait loop\n");

    // waiting for stop
    while (!stopping) {
        MutexGuard mg(stopmon);
        stopmon.wait(currentTimeMillis()+2000);
        if ( 0 == stat(stopfile,&st) ) {
            unlink(stopfile);
            stopping = true;
            break;
        }
    }

    printf("stopping write group\n");
    wtg.stop();

    printf("stopping workdirs\n");
    for ( unsigned i = 0; i < cfg.nfiles; ++i ) {
        workdirs[i]->stop();
    }

    printf("stopping read queue\n");
    readingQueue.stop();

    printf("stopping read group\n");
    rtg.stop();

    for( std::vector< WorkDir* >::iterator i = workdirs.begin(),
         ie = workdirs.end(); i != ie; ++i ) {
        delete *i;
    }
    printf("main finished\n");
    return 0;
}
