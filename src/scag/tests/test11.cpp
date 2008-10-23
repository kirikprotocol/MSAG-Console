#include <cstdio>
#include <vector>
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"

using namespace smsc::core::threads;
using namespace smsc::core::synchronization;

class A : public Thread
{
public:
    A() : stopped_(false) {
        printf( "created %p\n", this );
    }
    virtual ~A() {
        printf( "destroyed %p\n", this );
    }
    virtual int Execute()
    {
        printf( "executing %p\n", this );
        EventMonitor mon;
        MutexGuard mg(mon);
        while ( ! stopped_ ) {
            mon.wait(1000);
        }
        printf("exited %p\n", this );
        return 0;
    }

    void Stop() {
        stopped_ = true;
    }

private:
    bool stopped_;
};

int main()
{
    std::vector< A* > as;
    for ( size_t i = 0; i < 10; ++i ) {
        as.push_back( new A() );
    }

    // starting
    for ( std::vector< A* >::const_iterator i = as.begin(); i != as.end(); ++i ) {
        (*i)->Start();
    }
    
    {
        EventMonitor mon;
        MutexGuard mg(mon);
        mon.wait(2000);
    }

    printf( "send stopping signal to threads\n" );

    for ( std::vector< A* >::const_iterator i = as.begin(); i != as.end(); ++i ) {
        (*i)->Stop();
    }

    for ( std::vector< A* >::const_iterator i = as.begin(); i != as.end(); ++i ) {
        (*i)->WaitFor();
    }

    for ( std::vector< A* >::const_iterator i = as.begin(); i != as.end(); ++i ) {
        delete *i;
    }

    return 0;
}
