#ifndef _SCAG_COUNTER_ACTIONTABLE_H
#define _SCAG_COUNTER_ACTIONTABLE_H

#include "Counter.h"
#include "Observer.h"

namespace scag2 {
namespace counter {

class ActionTablePtr;

class ActionTable : public Observer
{
protected:
    static smsc::logger::Logger* log_;
    virtual ~ActionTable();
public:
    ActionTable();
    virtual void modified( Counter& counter, int64_t value );
    /// NOTE: do not fiddle with this method
    virtual void ref(bool add);
private:
    smsc::core::synchronization::Mutex lock_;
    unsigned ref_;
};

}
}


#endif /* !_SCAG_COUNTER_ACTIONTABLE_H */
