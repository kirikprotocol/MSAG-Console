#include <cassert>
#include <iostream>
#include "scag/counter/impl/HashCountManager.h"
#include "scag/counter/Accumulator.h"
#include "logger/Logger.h"

using namespace scag2::counter;

int main()
{
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );

    impl::HashCountManager mgr;
    CounterPtr< Accumulator > ptr;
    {
        const char* pname = "sys.sessions.activeCount";
        ptr = mgr.getCounter< Accumulator >(pname);
        if ( ptr.get() ) {
            assert( ptr->getType() == Accumulator::getStaticType() );
        }
        if (!ptr.get()) {
            ptr = mgr.registerCounter(new Accumulator(pname));
        }
    }
    std::cout << "counter name=" << ptr->getName() << std::endl;
    std::cout << "counter type=" << ptr->getType() << std::endl;
    std::cout << "counter count=" << ptr->getCount() << std::endl;

    const int64_t inc = ptr->accumulate( time(0) );
    std::cout << "after increment=" << inc << std::endl;
    assert( inc == ptr->getCount() );
    ptr->reset();
    assert( ptr->getCount() == 0 );
    return 0;
}
