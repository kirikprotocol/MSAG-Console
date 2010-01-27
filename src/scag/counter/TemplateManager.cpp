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


CounterTemplate* CounterTemplate::create( const char* type,
                                          Observer*   observer,
                                          int64_t     param0,
                                          int64_t     param1 )
{
    if (!type) return 0;
    Counter* ptr = 0;
    if ( 0 == strcmp(type,"accumulator") ) {
        ptr = new Accumulator(type,observer,0);
    } else if ( 0 == strcmp(type,"average") ) {
        ptr = new Average(type,param0,observer,0);
    } else if ( 0 == strcmp(type,"timesnapshot") ) {
        ptr = new TimeSnapshot(type,unsigned(param0),unsigned(param1),observer,0);
    } else {
        return 0;
    }
    return new CounterTemplate(ptr);
}

}
}
