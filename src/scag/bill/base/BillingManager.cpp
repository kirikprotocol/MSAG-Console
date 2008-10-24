#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "BillingManager.h"

using namespace scag2::bill;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< BillingManager, OuterCreation > Single;
bool inited = false;
Mutex mtx;

/// BillingManager is required for sessionmanager.
inline unsigned GetLongevity( BillingManager* ) { return 251; }

}

namespace scag2 {
namespace bill {

BillingManager& BillingManager::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("BillingManager not inited");
    }
    return Single::Instance();
}

BillingManager::BillingManager()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}

BillingManager::~BillingManager()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace bill
} // namespace scag2
