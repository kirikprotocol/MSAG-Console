#include "logger/Logger.h"
#include "core/threads/ThreadPool.hpp"
#include "core/buffers/PerThreadData.hpp"

using namespace smsc::logger;
using namespace smsc::core::threads;
using namespace smsc::core::buffers;

class TestTask : public ThreadedTask
{
private:
    class TestObj
    {
    public:
        TestObj( Logger* tlog, int id ) : id_(id) {
            smsc_log_debug(tlog,"entering testObj ctor id=%d",id);
            timespec ts = {1,0};
            nanosleep(&ts,0);
            smsc_log_debug(tlog,"leaving  testObj ctor id=%d",id_);
        }

        int getId() const { return id_; }

    private:
        int id_;
    };

    class TestObj2
    {
    public:
        TestObj2( int id ) : id_(id) {
            fprintf(stderr,"%lx obj2 @ %p ctor id=%d\n",
                    long(pthread_self()), this, id);
        }
            
        inline int getId() const { return id_; }

        ~TestObj2() {
            fprintf(stderr,"%lx obj2 @ %p dtor id=%d\n",
                    long(pthread_self()), this, id_);
        }

    private:
        int id_;
    };

public:
    TestTask( int id ) : log_(Logger::getInstance("task")), id_(id) {}

    virtual int Execute()
    {
        smsc_log_info(log_,"task %d started",id_);
        static PerThreadData< TestObj2 > data;
        data.reset( new TestObj2(id_) );

        static TestObj testObj(log_,id_);

        smsc_log_info( log_,"task %d finished, objId=%d, data=%d",
                       id_, testObj.getId(), data.get()->getId() );
        if (id_ % 2) {
            smsc_log_info(log_,"destroying test obj");
            delete data.release();
        }
        return 0;
    }

    virtual const char* taskName() 
    {
        return "testTask";
    }

private:
    Logger* log_;
    int     id_;
};


int main()
{
    Logger::initForTest( Logger::LEVEL_DEBUG );
    ThreadPool tp;
    for ( int i = 0; i < 10; ++i ) {
        tp.startTask( new TestTask(i) );
    }
    timespec ts = {5,0};
    nanosleep(&ts,0);
    tp.stopNotify();
    tp.shutdown(0);
    return 0;
}
