#include <cassert>
#include <iostream>
#include "scag/counter/impl/HashCountManager.h"
#include "scag/counter/Average.h"
#include "logger/Logger.h"
#include "scag/util/io/Drndm.h"
#include "core/synchronization/EventMonitor.hpp"

using namespace scag2::counter;
using scag2::util::Drndm;

std::auto_ptr<impl::HashCountManager> mgr;

const char* groupname = "sys.performance";

CounterPtr< Average > getCounter( const char* name, counttime_type delayTime )
{
    CounterPtr< Average > ptr = mgr->getCounter< Average >(name);
    if ( ! ptr.get() ) {
        try {
            CounterPtr< AveragingGroup > grp( mgr->getCounter< AveragingGroup >(groupname) );
            if ( !grp.get() ) {
                grp = mgr->registerCounter
                    ( new AveragingGroup( groupname,
                                          mgr->getAvgManager(),
                                          1000,
                                          delayTime ) );
            }
            ptr = mgr->registerCounter
                ( new Average(name,
                              grp,
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
        CounterPtr< Average > ptr = getCounter(fullname,delay);
        if ( !ptr.get() ) continue;

        // ptr->accumulate(time(0));
        const uint64_t r0 = rnd.get();
        const uint64_t r1 = Drndm::uniform(100,r0);
        const int64_t r2 = int64_t(r1) - 50;
        smsc_log_debug(logger,"r0=%llu r1=%llu r2=%lld",r0,r1,r2);
        ptr->increment(r2);

        const Average::Stat& total = ptr->total();
        const Average::Stat& last = ptr->last();
        smsc_log_debug(logger,"counter name='%s' type=%u total=(cnt:%lld,sum:%lld,sm2:%lld), last=(cnt:%lld,sum:%lld,sm2:%lld)",
                       ptr->getName().c_str(),
                       unsigned(ptr->getType()),
                       total.count, total.sum, total.sum2,
                       last.count, last.sum, last.sum2 );
        mainmon.wait(20);
    }
    return 0;
}
