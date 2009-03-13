//
// A test of concurrent increment of int and reading that int value.
// Are those values are monotonic?
//

#include <iostream>
#include "scag/util/WatchedThreadedTask.h"
#include "core/threads/ThreadPool.hpp"
#include "core/threads/Thread.hpp"
#include "logger/Logger.h"
// #include "core/synchronization/EventMonitor.hpp"

using namespace scag2::util;
using namespace smsc::core::synchronization;
using namespace smsc::core::threads;

smsc::logger::Logger* log_;

class Inc : public WatchedThreadedTask
{
public:
    Inc( volatile int& value ) : value_(value) {}
    
    virtual const char* taskName() { return "inc"; }
    virtual int Execute() {
        while ( ! isStopping ) {
            ++value_;
            // smsc_log_debug(log_,"inc %d", value_);
            if ( value_ < 0 ) break;
            Thread::Yield();
        }
        return 0;
    }

private:
    volatile int& value_;
};


std::auto_ptr<Inc> fun1( volatile int& i )
{
    return std::auto_ptr<Inc>(new Inc(i));
}

int main()
{
    smsc::logger::Logger::Init();
    log_ = smsc::logger::Logger::getInstance("test15");

    ThreadPool tp;
    volatile int value = 0;
    int prevValue = value;
    tp.startTask( fun1(value).release() );
    while ( true ) {
        int cur = value;
        if ( cur != prevValue ) smsc_log_debug(log_,"get %d", cur);
        if ( cur < 0 ) break;
        if ( cur < prevValue ) {
            std::cout << "discrepancy: prev=" << prevValue << " cur=" << cur << std::endl;
        }
        prevValue = cur;
    }
    return 0;
}
