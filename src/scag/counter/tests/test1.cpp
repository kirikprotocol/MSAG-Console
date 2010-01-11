#include <cassert>
#include <iostream>
#include "scag/counter/impl/HashCountManager.h"
#include "scag/counter/Accumulator.h"
#include "logger/Logger.h"
#include "scag/util/io/Drndm.h"

using namespace scag2::counter;
using scag2::util::Drndm;

std::auto_ptr<impl::HashCountManager> mgr;

CounterPtr< Accumulator > getAccumulator( const char* name, counttime_type delayTime )
{
    CounterPtr< Accumulator > ptr = mgr->getCounter< Accumulator >(name);
    if ( ! ptr.get() ) {
        try {
            ptr = mgr->registerCounter( new Accumulator(name,delayTime) );
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

    mgr.reset( new impl::HashCountManager() );
    mgr->start();

    Drndm& rnd = Drndm::getRnd();
    const counttime_type delay = 3;

    for ( unsigned i = 0; i < 1000; ++i ) {

        const char* name = names[Drndm::uniform(sizeof(names)/sizeof(names[0]),rnd.get())];
        CounterPtr< Accumulator > ptr = getAccumulator(name,delay);
        if ( !ptr.get() ) continue;

        ptr->accumulate(10);

        smsc_log_debug(logger,"counter name='%s' type=%u integral=%llu",
                       ptr->getName().c_str(),
                       unsigned(ptr->getType()),
                       ptr->getIntegral());
    }
    return 0;
}
