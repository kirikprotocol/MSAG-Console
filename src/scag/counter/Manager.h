#ifndef _SCAG_COUNTER_COUNTMANAGER_H
#define _SCAG_COUNTER_COUNTMANAGER_H

#include "Counter.h"
#include "util/TypeInfo.h"
#include "util/Exception.hpp"
#include "TimeSliceManager.h"
#include "TemplateManager.h"

namespace scag2 {
namespace counter {

class Accumulator;

class Manager : public Disposer
{
public:
    static Manager& getInstance();

protected:
    Manager();

public:
    virtual ~Manager();

    template < class T > CounterPtr< T > getCounter( const char* name ) {
        CounterPtrAny ptr = getAnyCounter(name);
        CounterPtr< T > res;
        if (ptr.get() && ptr->getType() == smsc::util::TypeInfo<T>::typeValue() ) {
            res.reset( static_cast<T*>(ptr.get()) );
        }
        return res;
    }

    /// the same notes as for registerAnyCounter
    template <class T> CounterPtr< T > registerCounter( T* c )
    {
        // if (!c) throw smsc::util::Exception("registering null counter");
        // setDisposer(*c);
        CounterPtr< T > res( static_cast< T* >(registerAnyCounter(c).get()) );
        return res;
    }

    inline CounterPtrAny registerAnyCounter( Counter* c ) {
        if (!c) throw smsc::util::Exception("registering null counter");
        c->disposer_ = this;
        bool wasReg;
        CounterPtrAny ptr = doRegisterAnyCounter(c,wasReg);
        if (wasReg) c->postRegister(*this);
        return ptr;
    }

    /// retrieve the counter of type T with name 'name'
    /// it may return 0 if the counter is not there.
    virtual CounterPtrAny getAnyCounter( const char* name ) = 0; // throw smsc::util::Exception

    /// return an instance of time slice manager
    virtual TimeSliceManager& getTimeManager() = 0;
    virtual TemplateManager* getTemplateManager() = 0;

protected:
    /// register counter and return a ptr to it, or to existing counter of this name.
    /// it may throw exception if counter types are not the same.
    /// otherwise it always return a ptr to a good counter.
    /// NOTE: don't use 'c' pointer after the call, the object under it may be destroyed!
    virtual CounterPtrAny doRegisterAnyCounter( Counter* c, bool& wasReg ) = 0;
    /*
    void registerTimeItem( TimeSliceItem* ptr ) {
        if (ptr) {
            TimeSliceManager& m = getTimeManager();
            m.addItem(*ptr,m.roundSlice(ptr->getTimeSliceWidth()));
        }
    }
     */
    inline void destroy( Counter* ptr ) {
        if (ptr) {
            ptr->preDestroy(*this);
            delete ptr;
        }
    }

private:
    static Manager* manager_;
};

}
}

#endif
