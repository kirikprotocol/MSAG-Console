#include <stdexcept>
#include "Manager.h"
#include "TemplateManager.h"
#include "scag/util/singleton/Singleton2.h"

namespace {
using namespace scag2::counter;
using namespace scag2::util::singleton;

typedef SingletonHolder< Manager, OuterCreation > Single;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( Manager* ) { return 253; }

}

namespace scag2 {
namespace counter {

Manager& Manager::getInstance()
{
    if (!inited) {
        MutexGuard mg(mtx);
        if ( !inited ) throw std::runtime_error("counter::Manager is not inited");
    }
    return Single::Instance();
}


Manager::Manager()
{
    MutexGuard mg(mtx);
    assert(!inited);
    Single::setInstance(this);
    inited = true;
}


Manager::~Manager()
{
    MutexGuard mg(mtx);
    inited = false;
}


CounterPtrAny Manager::createCounter( const char* tmplid,
                                      const char* name,
                                      unsigned    lifetime,
                                      unsigned    maxval )
{
    Counter* c = getTemplateManager()->createCounter(tmplid,name,lifetime,maxval);
    if (!c) return CounterPtrAny();
    return registerAnyCounter(c);
}


}
}
