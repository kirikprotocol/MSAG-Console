#ident "$Id$"
#ifndef __SMSC_INMAN_COMMON_OBSERVABLE__
#define __SMSC_INMAN_COMMON_OBSERVABLE__

#include <list>
#include <algorithm>
#include <functional>

namespace smsc  {
namespace inman {
namespace common  {

// Этот функтор удаляет переданный ему указатель
// Полезен при работе с алгоритмами STL, например
// std::for_each( listeners.begin(), listeners.end(), DeletePtr() );
// удаляет все обьекты из контейнера
struct DeletePtr : std::unary_function< void*, void > 
{
    result_type operator( ) ( argument_type ptr )
    {
        delete ptr;
    }
};

template < class Listener >
class ObservableT {
public:
    typedef std::list<Listener*> ListenerList;

    ObservableT() { }
    ~ObservableT() { clearListeners(); }

    inline void addListener(Listener* pListener)    { listeners.push_back(pListener); }
    inline void removeListener(Listener* pListener) { listeners.remove(pListener); }
    inline bool hasListeners(void) const            { return !listeners.empty(); }
    inline void clearListeners(void)                { listeners.clear(); }

    inline void deleteListeners(void)
    {
        std::for_each( listeners.begin(), listeners.end(), DeletePtr() );
        listeners.clear();
    }
    
protected:
    ListenerList listeners;
};

} //common
} //inman
} //smsc

#endif /* __SMSC_INMAN_COMMON_OBSERVABLE__ */

