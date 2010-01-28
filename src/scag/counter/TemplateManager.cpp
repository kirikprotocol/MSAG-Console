#include "TemplateManager.h"
#include "Counter.h"
#include "Accumulator.h"
#include "Average.h"
#include "TimeSnapshot.h"

namespace scag2 {
namespace counter {

CounterTemplate::~CounterTemplate()
{
    delete prototype_;
}


CounterTemplate* CounterTemplate::create( const char* tname,
                                          Observer*   observer,
                                          int64_t     param0,
                                          int64_t     param1 )
{
    const Counter::CountType type = Counter::stringToCountType(tname);
    Counter* proto;
    switch (type) {
    case Counter::TYPEUNKNOWN: return 0;
    case Counter::TYPEACCUMULATOR:
        proto = new Accumulator("",observer,0);
        break;
    case Counter::TYPEAVERAGE:
        proto = new Average("",param0,observer,0);
        break;
    case Counter::TYPETIMESNAPSHOT:
        proto = new TimeSnapshot("",unsigned(param0),unsigned(param1),observer,0);
    }
    return new CounterTemplate(proto);
}

}
}
