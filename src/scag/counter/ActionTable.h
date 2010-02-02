#ifndef _SCAG_COUNTER_ACTIONTABLE_H
#define _SCAG_COUNTER_ACTIONTABLE_H

#include <list>
#include "Counter.h"
#include "Observer.h"

namespace scag2 {
namespace counter {

class ActionTable : public Observer
{
protected:
    static smsc::logger::Logger* log_;
    virtual ~ActionTable();
public:
    ActionTable( ActionList* list = 0 );
    virtual void modified( const char*  cname,
                           CntSeverity& sev,
                           int64_t      value,
                           unsigned     maxval );

    /// NOTE: do not fiddle with this method
    virtual void ref(bool add);

    /// setting a new actions
    virtual void setNewActions( ActionList* newlist );

private:
    // void notify( Counter& c, int64_t value, const ActionParams& params );

private:
    smsc::core::synchronization::Mutex    lock_;
    unsigned                              ref_;
    smsc::core::synchronization::Mutex    actlock_;
    ActionList*                           actions_; // owned
    typedef std::pair<time_t,ActionList*> OldList;
    std::list< OldList >                  oldlists_;
};

}
}


#endif /* !_SCAG_COUNTER_ACTIONTABLE_H */
