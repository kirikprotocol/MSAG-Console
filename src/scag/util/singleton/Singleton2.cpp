
#include <stdio.h>

#include "Singleton2.h"

using namespace scag2::util::singleton;
using namespace scag2::util::singleton::Private;

scag2::util::singleton::Private::TrackerArray scag2::util::singleton::Private::pTrackerArray = 0;
unsigned int scag2::util::singleton::Private::elements = 0;

////////////////////////////////////////////////////////////////////////////////
// function AtExitFn
// Ensures proper destruction of objects with longevity
////////////////////////////////////////////////////////////////////////////////
extern "C" void scag2::util::singleton::Private::AtExitFn()
{
    assert(elements > 0 && pTrackerArray != 0);
    
    // Pick the element at the top of the stack
    LifetimeTracker* pTop = pTrackerArray[elements - 1];
    // Remove that object off the stack
    // Don't check errors - realloc with less memory can't fail
    pTrackerArray = static_cast<TrackerArray>(std::realloc(pTrackerArray, 
                                                           sizeof(*pTrackerArray) * --elements));
    // Destroy the element
    delete pTop;
}

/*
// Following code is for test reasons only !!!!

struct A { 
     A() { printf("A()\n");  }
    ~A() { printf("~A()\n"); }
};
struct B { 
     B() { printf("B()\n");  }
    ~B() { printf("~B()\n"); }
};
struct C { 
     C() { printf("C()\n");  }
    ~C() { printf("~C()\n"); }
};
struct D { 
     D() { printf("D()\n");  }
    ~D() { printf("~D()\n"); }
};
struct S { 
     S() { printf("S()\n");  }
    ~S() { printf("~S()\n"); }
};

static S s;

inline unsigned GetLongevity(A*) { return 1; }
inline unsigned GetLongevity(B*) { return 2; }
inline unsigned GetLongevity(C*) { return 3; }
inline unsigned GetLongevity(D*) { return 2; }


// In Statistics.h file
struct Statistics
{
    static Statistics& Instance();

    virtual void aaa() = 0;

protected:
    
    Statistics() {};
    virtual ~Statistics() {};
};

// In StatisticsManager.h file
class StatisticsManager : public Statistics
{
friend class Statistics;
private:
    
    void _init(int param);
    
    static bool  inited;
    //static Mutex initLock;

public:

    static void init(int param);    

    virtual void aaa();

    StatisticsManager() : Statistics() { printf("SM()\n"); };
    virtual ~StatisticsManager() { printf("~SM()\n"); }
};

// In StatisticsManager.cpp file
inline unsigned GetLongevity(StatisticsManager*) { return 1; }
//typedef SingletonHolder<StatisticsManager, CreateUsingNew, SingletonWithLongevity> SingleSM;
typedef SingletonHolder<StatisticsManager> SingleSM;

bool  StatisticsManager::inited = false;
//Mutex StatisticsManager::initLock;

Statistics& Statistics::Instance()
{
    if (!StatisticsManager::inited) 
    {
        //MutexGuard guard(Statistics::initLock);
        if (!StatisticsManager::inited) throw std::runtime_error("Statistics not inited!");
    }
    return SingleSM::Instance();
}

void StatisticsManager::init(int param)
{
    if (!StatisticsManager::inited)
    {
        //MutexGuard guard(Statistics::initLock);
        if (!StatisticsManager::inited) {
            StatisticsManager& sm = SingleSM::Instance();
            sm._init(param);
            StatisticsManager::inited = true;
        }
    }
}
void StatisticsManager::_init(int param)
{
    printf("_init()\n");
}
void StatisticsManager::aaa() {
    printf("aaa() called\n");
}


int main(void)
{
    printf("Start\n");
    
    StatisticsManager::init(10);

    Statistics& stat = Statistics::Instance();
    stat.aaa();

    Statistics::Instance().aaa();

    typedef SingletonHolder<A, CreateUsingNew, SingletonWithLongevity> SingleA;
    A& a = SingleA::Instance();
    typedef SingletonHolder<D, CreateUsingNew, SingletonWithLongevity> SingleD;
    D& d = SingleD::Instance();
    typedef SingletonHolder<B, CreateUsingNew, SingletonWithLongevity> SingleB;
    B& b = SingleB::Instance();

    C c;
    typedef SingletonHolder<D, CreateUsingNew, SingletonWithLongevity> SingleDD;

    printf("Exit\n");
    return 0;
}
*/
