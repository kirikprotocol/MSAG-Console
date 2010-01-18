#include <cassert>
#include <iostream>
#include "scag/counter/impl/HashCountManager.h"
#include "scag/counter/TimeSnapshot.h"
#include "logger/Logger.h"
#include "scag/util/io/Drndm.h"
#include "core/synchronization/EventMonitor.hpp"

using namespace scag2::counter;
using scag2::util::Drndm;

std::auto_ptr<impl::HashCountManager> mgr;

CounterPtr< TimeSnapshot > getSnapshot( const char* name, counttime_type delayTime )
{
    CounterPtr< TimeSnapshot > ptr = mgr->getCounter< TimeSnapshot >(name);
    if ( ! ptr.get() ) {
        try {
            ptr = mgr->registerCounter
                ( new TimeSnapshot(name,
                                   usecFactor*2,10,
                                   delayTime) );
        } catch ( std::exception& e ) {
        }
    }
    return ptr;
}


const char* names[] = {
        "sys.test.hello",
        "sys.test.world",
        "what.the.f**k",
        "sys.perf.diskio",
        "msag.operator.1.reqs"
};


int main()
{
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("main");
    smsc::core::synchronization::EventMonitor mainmon;
    MutexGuard mg(mainmon);

    mgr.reset( new impl::HashCountManager() );
    mgr->start();

    Drndm& rnd = Drndm::getRnd();
    const counttime_type delay = 3;
    const unsigned indices = 6;

    for ( unsigned i = 0; i < 1000; ++i ) {

        const char* name = names[Drndm::uniform(sizeof(names)/sizeof(names[0]),rnd.get())];
        const unsigned idx = unsigned(Drndm::uniform(indices,rnd.get()));
        char fullname[100];
        snprintf(fullname,sizeof(fullname),"%s.%u",name,idx);
        CounterPtr< TimeSnapshot > ptr = getSnapshot(fullname,delay);
        if ( !ptr.get() ) continue;

        // ptr->accumulate(time(0));
        ptr->increment();

        smsc_log_debug(logger,"counter name='%s' type=%u integral=%llu",
                       ptr->getName().c_str(),
                       unsigned(ptr->getType()),
                       ptr->getValue());
        mainmon.wait(20);
    }
    return 0;
}
