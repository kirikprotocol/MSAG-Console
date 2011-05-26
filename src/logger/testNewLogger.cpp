#include <time.h>
#include <vector>
#include "Logger.h"
#include "util/TimeSource.h"
#include "core/threads/Thread.hpp"
#include "core/buffers/TmpBuf.hpp"


class TestThread;

void testFun1( smsc::logger::Logger* thelog, TestThread& )
{
    smsc_log_debug(thelog,"simple call");
}

typedef void (testFunType) ( smsc::logger::Logger* thelog, TestThread& );

class TestThread : public smsc::core::threads::Thread
{
public:
    TestThread( smsc::logger::Logger* thelog, testFunType* fun ) :
        isStopping_(false), count_(0), log_(thelog), fun_(fun) {}

    ~TestThread() {
        stop();
    }

    int Execute()
    {
        while ( ! isStopping_ ) {
            (*fun_)(log_,*this);
            ++count_;
        }
        return 0;
    }

    unsigned stop() {
        isStopping_ = true;
        WaitFor();
        return count_;
    }

    inline unsigned getCount() const { return count_; }

private:
    bool isStopping_;
    unsigned count_;
    smsc::logger::Logger* log_;
    testFunType* fun_;
};


void testFun2( smsc::logger::Logger* thelog, TestThread& t )
{
    smsc_log_debug(thelog,"call with a few arguments %u",t.getCount());
}

void testFun3( smsc::logger::Logger* thelog, TestThread& t )
{
    smsc_log_debug(thelog,"rather big: log@%p logName='%s' thread@%p count=%u",
                   thelog,thelog->getName(),&t,t.getCount());
}

namespace {
smsc::core::buffers::TmpBuf<char,1024> verylongdump;
}

void testFun4( smsc::logger::Logger* thelog, TestThread& t )
{
    smsc_log_debug(thelog,"log@%p thread@%p count=%u dump:%s",thelog,&t,t.getCount(),verylongdump.get());
}


int readArg( const char* arg, int minval, int maxval )
{
    if (!arg) throw smsc::util::Exception("option requires an argument");
    const int res = atoi(arg);
    if ( res < minval || res > maxval ) {
        throw smsc::util::Exception("invalid value of argument '%s' -> %d, must be [%d,%d]",
                                    arg, res, minval, maxval );
    }
    return res;
}


typedef smsc::util::TimeSourceSetup::AbsUSec USec;

#include "core/synchronization/MutexReportContentionRealization.h"

int main( int argc, char** argv)
{
    // smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );
    smsc::logger::Logger::Init();

    char** arg = argv;
    int threads = 10;
    int fun = 1;
    int waitTime = 30;
    testFunType* funPtr = &testFun1;
    while ( *++arg ) {
        const std::string a(*arg);
        if ( a == "--threads" ) {
            threads = readArg(*++arg,1,100);

            if ( !*++arg ) throw smsc::util::Exception("--threads requires an arg");
            threads = atoi(*arg);
            if ( threads < 1 || threads > 100 ) {
                throw smsc::util::Exception("wrong number of threads %u",threads);
            }
        } else if ( a == "--fun" ) {
            fun = readArg(*++arg,1,4);
            switch (fun) {
            case 1 : funPtr = &testFun1; break;
            case 2 : funPtr = &testFun2; break;
            case 3 : funPtr = &testFun3; break;
            case 4 : funPtr = &testFun4; break;
            default: throw smsc::util::Exception("invalid function %u",fun); break;
            }
        } else if ( a == "--sleep" ) {
            waitTime = readArg(*++arg,1,100);
        } else {
            throw smsc::util::Exception("invalid argument %s",*arg);
        }
    }

    if (4==fun) {
        // preparing a dump
        for ( int i = 0; i < 1000; ++i ) {
            const USec::usec_type t = USec::getUSec();
            char buf[20];
            sprintf(buf," %02x %02x",int(t>>8)&255,int(t)&255);
            verylongdump.Append(buf,strlen(buf));
        }
        verylongdump.Append("",1);
        // printf("dump: %s\n",verylongdump.get());
    }

    typedef std::vector<TestThread*> TList;
    TList threadList;
    threadList.reserve(threads);
    for ( int i = 0; i < threads; ++i ) {
        char buf[20];
        sprintf(buf,"thr%03u",i);
        threadList.push_back( new TestThread( smsc::logger::Logger::getInstance(buf),
                                              funPtr ));
    }

    // sleeping a number of seconds
    const USec::usec_type start = USec::getUSec();

    for ( TList::iterator i = threadList.begin(), ie = threadList.end(); i != ie; ++i ) {
        (*i)->Start();
    }

    const timespec sleepTime = { waitTime, 0 };
    nanosleep( &sleepTime, 0 );
    
    unsigned total = 0;
    for ( TList::iterator i = threadList.begin(), ie = threadList.end(); i != ie; ++i ) {
        total += (*i)->stop();
        delete *i;
    }

    const USec::usec_type stop = USec::getUSec();
    const double elapsed = (stop - start)/1000000.;

    printf("Threads: %u\n",threads);
    printf("Type of test: %u\n",fun);
    printf("Interval of measurements: %.3f\n",elapsed);
    printf("Number of logger calls: %u\n", total);
    printf("Calls/second: %.3f\n", total / elapsed );
    return 0;
}
