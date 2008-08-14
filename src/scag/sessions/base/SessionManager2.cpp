#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "SessionManager2.h"


using namespace scag2::sessions;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< SessionManager, OuterCreation > SingleSM;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( SessionManager* ) { return 6; }

}

namespace scag2 {
namespace sessions {

SessionManager& SessionManager::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("SessionManager not inited");
    }
    return SingleSM::Instance();
}

SessionManager::SessionManager()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    SingleSM::setInstance( this );
}

SessionManager::~SessionManager()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace sessions
} // namespace scag2
