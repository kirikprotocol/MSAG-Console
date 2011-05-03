#ifndef _SCAG_COUNTER_COUNTMANAGER_H
#define _SCAG_COUNTER_COUNTMANAGER_H

#include "Counter.h"
#include "util/Exception.hpp"
#include "scag/snmp/MsagCounterTableElement.h"

namespace scag2 {
namespace counter {

class TemplateManager;
class TimeSliceManager;

class Manager
{
    friend class Counter;
public:
    static Manager& getInstance();

protected:
    Manager();

public:
    virtual ~Manager();

    virtual MsagCounterTableElement* updateSnmpCounterList( MsagCounterTableElement* list ) = 0;

    inline CounterPtrAny registerAnyCounter( Counter* c )
    {
        if (!c) throw smsc::util::Exception("registering null counter");
        // c->disposer_ = this;
        bool wasReg;
        CounterPtrAny ptr = doRegisterAnyCounter(c,wasReg);
        if (wasReg) c->postRegister(*this);
        return ptr;
    }

    /// retrieve the counter of type T with name 'name'
    /// it may return 0 if the counter is not there.
    virtual CounterPtrAny getAnyCounter( const char* name ) = 0; // throw smsc::util::Exception

    // helper method to create a counter from template
    // CounterPtrAny createCounter( const char* tmplid,
    // const char* name,
    // unsigned    lifetime = counttime_max );

    /// helper method to get observer
    ObserverPtr getObserver( const char* name );

    /// return an instance of time slice manager
    virtual TimeSliceManager& getTimeManager() = 0;
    virtual TemplateManager* getTemplateManager() = 0;

    /// make a notification for the counter
    virtual void notify( const char* cname,
                         CntSeverity& sev,   // severity for backtrace
                         int64_t value,      // newval
                         const ActionLimit& limit ) = 0;

    virtual void stop() = 0;

    /// load observer/template from config and replace them in template manager.
    virtual void reloadObserver( const char* id ) = 0; // throw
    virtual void reloadTemplate( const char* id ) = 0; // throw

protected:
    /// register counter and return a ptr to it, or to existing counter of this name.
    /// it may throw exception if counter types are not the same.
    /// otherwise it always return a ptr to a good counter.
    /// NOTE: don't use 'c' pointer after the call, the object under it may be destroyed!
    virtual CounterPtrAny doRegisterAnyCounter( Counter* c, bool& wasReg ) = 0;
    inline void destroy( Counter* ptr )
    {
        if (ptr) {
            ptr->preDestroy(*this);
            delete ptr;
        }
    }

    virtual counttime_type getWakeTime() const = 0;
    virtual void scheduleDisposal( Counter& c ) = 0;

    inline bool checkDisposal( Counter* c, counttime_type now ) const
    {
        if ( !c || c->usage_ ) return false;
        {
            smsc::core::synchronization::MutexGuard mg(c->usageMutex_);
            if ( c->usage_ || c->disposeTime_ > now ) return false;
        }
        return true;
    }
    
private:
    static Manager* manager_;
};

}
}

#endif
