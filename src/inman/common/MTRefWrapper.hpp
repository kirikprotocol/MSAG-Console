/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __MT_REFWRAPPER_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __MT_REFWRAPPER_HPP__

#include <pthread.h>

namespace smsc {
namespace core {
namespace synchronization {

// ********************************************************************
// 
// NOTE: Requires an external sync object.
// ********************************************************************
template <class _TArg>
class MTRefWrapper_T {
protected:
    _TArg   * first;
    _TArg   * second;
    pthread_t thrId;

public:
    MTRefWrapper_T(_TArg * use_ref = NULL)
        : thrId((pthread_t)(-1))
    {
        first = use_ref;  second = 0;
    }
    ~MTRefWrapper_T()
    { }

    void Reset(_TArg * use_ref)
    {
        thrId = (pthread_t)(-1);
        first = use_ref;  second = 0;
    }

    _TArg * get(void) const { return first; }
    _TArg * operator->() const { return first; }

    _TArg * Lock(void)
    {
        if (first)
            thrId = pthread_self();
        return (second = first);
    }
    _TArg * UnLock(void)
    {
        first = second;
        second = 0;
        thrId = (pthread_t)(-1);
        return first;
    }
    bool Unref(void)
    {
        if (!second) { //not locked
            first = 0;
            return true;
        }
        if (thrId == pthread_self()) {  //locked by same thread,
            second = 0;                 //next Unlock() will perform Unref()
            return true;
        }
        return false;
    }
};


}//namespace synchronization
}//namespace core
}//namespace smsc

#endif /* __MT_REFWRAPPER_HPP__ */

