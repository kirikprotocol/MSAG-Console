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


CounterTemplate* CounterTemplate::create( CountType   type,
                                          Observer*   observer,
                                          unsigned    param0,
                                          unsigned    param1 )
{
    Counter* proto;
    switch (type) {
    case TYPEUNKNOWN: return 0;
    case TYPEACCUMULATOR:
        proto = new Accumulator("",observer,0);
        break;
    case TYPEAVERAGE:
        proto = new Average("",param0,observer,0);
        break;
    case TYPETIMESNAPSHOT:
        proto = new TimeSnapshot("",param0,param1,observer,0);
    }
    return new CounterTemplate(proto);
}

}
}
