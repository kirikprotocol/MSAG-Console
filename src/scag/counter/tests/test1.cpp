#include <cassert>
#include <iostream>
#include "scag/counter/impl/HashCountManager.h"
#include "scag/counter/Accumulator.h"
#include "logger/Logger.h"
#include "scag/util/io/Drndm.h"
#include "core/synchronization/EventMonitor.hpp"

using namespace scag2::counter;
using scag2::util::Drndm;

CounterPtr< Accumulator > getAccumulator( const char* name, counttime_type delayTime )
{
    Manager& mgr = Manager::getInstance();
    CounterPtr< Accumulator > ptr = mgr.getCounter< Accumulator >(name);
    if ( ! ptr.get() ) {
        try {
            ptr = mgr.registerCounter( new Accumulator(name,0,delayTime) );
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

    {
        impl::HashCountManager* mgr = new impl::HashCountManager();
        mgr->start();
    }

    Drndm& rnd = Drndm::getRnd();
    const counttime_type delay = 3;
    const unsigned indices = 10;

    for ( unsigned i = 0; i < 1000; ++i ) {

        const char* name = names[Drndm::uniform(sizeof(names)/sizeof(names[0]),rnd.get())];
        const unsigned idx = unsigned(Drndm::uniform(indices,rnd.get()));
        char fullname[100];
        snprintf(fullname,sizeof(fullname),"%s.%u",name,idx);
        CounterPtr< Accumulator > ptr = getAccumulator(fullname,delay);
        if ( !ptr.get() ) continue;

        ptr->increment(10);

        smsc_log_debug(logger,"counter name='%s' type=%u integral=%llu",
                       ptr->getName().c_str(),
                       unsigned(ptr->getType()),
                       ptr->getValue());
        mainmon.wait(1);
    }
    return 0;
}
