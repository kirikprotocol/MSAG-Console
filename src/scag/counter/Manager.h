#ifndef _SCAG_COUNTER_COUNTMANAGER_H
#define _SCAG_COUNTER_COUNTMANAGER_H

#include "Counter.h"
#include "util/TypeInfo.h"
#include "util/Exception.hpp"
#include "AveragingManager.h"

namespace scag2 {
namespace counter {

class Manager : public Disposer
{
public:
    virtual ~Manager() {}

    template < class T > CounterPtr< T > getCounter( const char* name ) {
        CounterPtrAny ptr = getAnyCounter(name);
        CounterPtr< T > res;
        if (ptr.get() && ptr->getType() == smsc::util::TypeInfo<T>::typeValue() ) {
            res.reset( static_cast<T*>(ptr.get()) );
        }
        return res;
    }

    /// the same notes as for registerAnyCounter
    template <class T> CounterPtr< T > registerCounter( T* c ) {
        if (!c) throw smsc::util::Exception("registering null counter");
        setDisposer(*c);
        CounterPtr< T > res( static_cast< T* >(registerAnyCounter(c).get()) );
        return res;
    }

    /// retrieve the counter of type T with name 'name'
    /// it may return 0 if the counter is not there.
    virtual CounterPtrAny getAnyCounter( const char* name ) = 0; // throw smsc::util::Exception

    /// return an instance of averaging manager
    virtual AveragingManager& getAvgManager() = 0;

protected:
    /// register counter and return a ptr to it, or to existing counter of this name.
    /// it may throw exception if counter types are not the same.
    /// otherwise it always return a ptr to a good counter.
    /// NOTE: don't use 'c' pointer after the call, the object under it may be destroyed!
    virtual CounterPtrAny registerAnyCounter( Counter* c ) = 0;
};

}
}

#endif
