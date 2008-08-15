#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "core/synchronization/Mutex.hpp"
#include "LongCallManager2.h"

using namespace scag2::lcm;
using namespace scag2::util::singleton;
using namespace smsc::core::synchronization;

namespace {

typedef SingletonHolder< LongCallManager, OuterCreation > Single;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( LongCallManager* ) { return 5; }

}

namespace scag2 {
namespace lcm {

LongCallManager& LongCallManager::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("LongCallManager not inited");
    }
    return Single::Instance();
}

LongCallManager::LongCallManager()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}


LongCallManager::~LongCallManager()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace lcm
} // namespace scag2
