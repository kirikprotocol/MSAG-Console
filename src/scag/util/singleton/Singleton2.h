#ifndef SCAG_UTIL_SINGLETON2
#define SCAG_UTIL_SINGLETON2

#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <new>

#include <core/synchronization/Mutex.hpp>
#include "Singleton.h"

namespace scag2 {
namespace util {
namespace singleton {

using namespace smsc::core::synchronization;

typedef scag::util::singleton::atexit_pfn_t atexit_pfn_t;
namespace Private = scag::util::singleton::Private;
    
    ////////////////////////////////////////////////////////////////////////////////
    // function template SetLongevity
    // Assigns an object a longevity; ensures ordered destructions of objects 
    //     registered thusly during the exit sequence of the application
    ////////////////////////////////////////////////////////////////////////////////
    template <typename T, typename Destroyer>
    void SetLongevity(T* pDynObject, unsigned int longevity, 
                      Destroyer d = Private::Deleter<T>::Delete)
    {
        using namespace Private;
        
        TrackerArray pNewArray = static_cast<TrackerArray>(
                std::realloc(pTrackerArray, sizeof(*pTrackerArray) * (elements + 1)));
        if (!pNewArray) throw std::bad_alloc();
        
        // Delayed assignment for exception safety
        pTrackerArray = pNewArray;
        
        LifetimeTracker* p = new ConcreteLifetimeTracker<T, Destroyer>(pDynObject, longevity, d);
        
        // Insert a pointer to the object into the queue
        TrackerArray pos = std::upper_bound(pTrackerArray, pTrackerArray + elements, 
                                            p, LifetimeTracker::Compare);
        std::copy_backward(pos, pTrackerArray + elements, pTrackerArray + elements + 1);
        *pos = p; ++elements;
        
        // Register a call to AtExitFn
        std::atexit(Private::AtExitFn);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // class template CreateUsingNew
    // Implementation of the CreationPolicy used by SingletonHolder
    // Creates objects using a straight call to the new operator 
    ////////////////////////////////////////////////////////////////////////////////
    template <class T> struct CreateUsingNew
    {
        static T* Create() { return new T; }
        static void Destroy(T* p) { delete p; }
    };
    
    ////////////////////////////////////////////////////////////////////////////////
    // class template CreateUsingNew
    // Implementation of the CreationPolicy used by SingletonHolder
    // Creates objects using a call to std::malloc, followed by a call to the 
    //     placement new operator
    ////////////////////////////////////////////////////////////////////////////////
    template <class T> struct CreateUsingMalloc
    {
        static T* Create()
        {
            void* p = std::malloc(sizeof(T));
            if (!p) return 0;
            return new(p) T;
        }
        
        static void Destroy(T* p)
        {
            p->~T();
            std::free(p);
        }
    };
    
    ////////////////////////////////////////////////////////////////////////////////
    // class template CreateStatic
    // Implementation of the CreationPolicy used by SingletonHolder
    // Creates an object in static memory
    // Implementation is slightly nonportable because it uses the MaxAlign trick 
    //     (an union of all types to ensure proper memory alignment). This trick is 
    //     nonportable in theory but highly portable in practice.
    ////////////////////////////////////////////////////////////////////////////////
    template <class T> struct CreateStatic
    {
#if defined(_MSC_VER) && _MSC_VER >= 1300
#pragma warning( push ) 
 // alignment of a member was sensitive to packing
#pragma warning( disable : 4121 )
#endif // _MSC_VER
        union MaxAlign
        {
            char t_[sizeof(T)];
            short int shortInt_;
            int int_;
            long int longInt_;
            float float_;
            double double_;
            long double longDouble_;
            struct Test;
            int Test::* pMember_;
            int (Test::*pMemberFn_)(int);
        };
#if defined(_MSC_VER) && _MSC_VER >= 1300
#pragma warning( pop )
#endif // _MSC_VER
        
        static T* Create()
        {
            static MaxAlign staticMemory_;
            return new(&staticMemory_) T;
        }
        
        static void Destroy(T* p)
        {
            p->~T();
        }
    };
    
    ////////////////////////////////////////////////////////////////////////////////
    // class template DefaultLifetime
    // Implementation of the LifetimePolicy used by SingletonHolder
    // Schedules an object's destruction as per C++ rules
    // Forwards to std::atexit
    ////////////////////////////////////////////////////////////////////////////////
    template <class T>
    struct DefaultLifetime
    {
        static void ScheduleDestruction(T*, atexit_pfn_t pFun) { std::atexit(pFun); }
        static void OnDeadReference() { throw std::logic_error("Dead Reference Detected"); }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // class template PhoenixSingleton
    // Implementation of the LifetimePolicy used by SingletonHolder
    // Schedules an object's destruction as per C++ rules, and it allows object 
    //    recreation by not throwing an exception from OnDeadReference
    ////////////////////////////////////////////////////////////////////////////////
    template <class T>
    class PhoenixSingleton
    {
    public:
        static void ScheduleDestruction(T*, atexit_pfn_t pFun)
        {
#ifndef ATEXIT_FIXED
            if (!destroyedOnce_)
#endif
                std::atexit(pFun);
        }
        
        static void OnDeadReference()
        {
#ifndef ATEXIT_FIXED
            destroyedOnce_ = true;
#endif
        }
        
    private:
#ifndef ATEXIT_FIXED
        static bool destroyedOnce_;
#endif
    };
    
#ifndef ATEXIT_FIXED
    template <class T> bool PhoenixSingleton<T>::destroyedOnce_ = false;
#endif
        
    ////////////////////////////////////////////////////////////////////////////////
    // class template SingletonWithLongevity
    // Implementation of the LifetimePolicy used by SingletonHolder
    // Schedules an object's destruction in order of their longevities
    // Assumes a visible function GetLongevity(T*) that returns the longevity of the
    // object
    ////////////////////////////////////////////////////////////////////////////////
    template <class T>
    class SingletonWithLongevity
    {
    protected:
        static void ScheduleDestruction(T* pObj, atexit_pfn_t pFun)
        {
            Private::Adapter<T> adapter = { pFun };
            scag2::util::singleton::SetLongevity(pObj, GetLongevity(pObj), adapter);
        }
        static void OnDeadReference() { throw std::logic_error("Dead Reference Detected"); }
    };

    ////////////////////////////////////////////////////////////////////////////////
    // class template NoDestroy
    // Implementation of the LifetimePolicy used by SingletonHolder
    // Never destroys the object
    ////////////////////////////////////////////////////////////////////////////////
    template <class T> 
    struct NoDestroy
    {
        static void ScheduleDestruction(T*, atexit_pfn_t pFun) {}
        static void OnDeadReference() {}
    };

    template <class T>
    class Volatile {
    public:
        typedef volatile T VolatileType;
    };
    

////////////////////////////////////////////////////////////////////////////////
// class template OuterCreation (implementation of CreationPolicy for SingletonHolder)
//
////////////////////////////////////////////////////////////////////////////////
template < class T >
    class OuterCreation
{
public:
    /// you have to make sure of synchronized access to this method
    static void setInstance( T* inst ) {
        if ( inst_ ) {
            throw std::logic_error("Multiple invocation of OuterCreation::setInstance()");
        }
        inst_ = inst;
    }
protected:
    static T* Create() {
        if ( ! inst_ ) {
            throw std::logic_error("Have you forgot to call OuterCreation::setInstance()");
        }
        return inst_;
    }
    static void Destroy(T* p) {
        if ( p != inst_ ) {
            throw std::logic_error("different pointer used in OuterCreation::Destroy()");
        }
        inst_ = 0;
        delete p;
    }

private:
    static T* inst_;
};

template < class T > T* OuterCreation< T >::inst_ = 0;

////////////////////////////////////////////////////////////////////////////////
// class template SingletonHolder
// Provides Singleton amenities for a type T
// To protect that type from spurious instantiations, you have to protect it
// yourself.
////////////////////////////////////////////////////////////////////////////////
template <
    typename T,
    template <class> class CreationPolicy = CreateUsingNew,
    template <class> class LifetimePolicy = SingletonWithLongevity >
    class SingletonHolder : public CreationPolicy< T >, public LifetimePolicy< T >
{
    public:
        
        static T& Instance() {
            if (!pInstance_) MakeInstance();
            return *pInstance_;
        }
        
    private:
        // Helpers
        static void MakeInstance()
        {
            MutexGuard guard(singletonHolderLock_);

            if (!pInstance_)
            {
                if (destroyed_)
                {
                    LifetimePolicy< T >::OnDeadReference();
                    destroyed_ = false;
                }
                pInstance_ = CreationPolicy< T >::Create();
                LifetimePolicy< T >::ScheduleDestruction(pInstance_, &DestroySingleton);
            }
        }
        static void DestroySingleton()
        {
            assert(!destroyed_);
            CreationPolicy< T >::Destroy(pInstance_);
            pInstance_ = 0;
            destroyed_ = true;
        }
        
        // Protection
        SingletonHolder();
        
        // Data
        static Mutex singletonHolderLock_;
        typedef typename Volatile<T*>::VolatileType PtrInstanceType;
        static PtrInstanceType pInstance_;
        static bool destroyed_;
    };
    
    ////////////////////////////////////////////////////////////////////////////////
    // SingletonHolder's data
    ////////////////////////////////////////////////////////////////////////////////
template < class T,
    template <class> class CreationPolicy,
    template <class> class LifetimePolicy >
    Mutex SingletonHolder<T, CreationPolicy, LifetimePolicy >::singletonHolderLock_;

template < class T,
    template <class> class CreationPolicy,
    template <class> class LifetimePolicy >
    typename SingletonHolder<T, CreationPolicy, LifetimePolicy >::PtrInstanceType 
    SingletonHolder<T, CreationPolicy, LifetimePolicy >::pInstance_ = 0;

template < class T,
    template <class> class CreationPolicy,
    template <class> class LifetimePolicy >
    bool SingletonHolder<T, CreationPolicy, LifetimePolicy >::destroyed_ = false;
    
}}}

#endif // SCAG_UTIL_SINGLETON


