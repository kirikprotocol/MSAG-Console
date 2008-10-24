#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "RuleEngine2.h"

using namespace scag2::re;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< RuleEngine, OuterCreation > Single;
bool inited = false;
Mutex mtx;

/// RE is required for sessionmanager.
inline unsigned GetLongevity( RuleEngine* ) { return 252; }

}

namespace scag2 {
namespace re {

RuleEngine& RuleEngine::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("RuleEngine not inited");
    }
    return Single::Instance();
}

RuleEngine::RuleEngine()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    Single::setInstance( this );
    inited = true;
}

RuleEngine::~RuleEngine()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace re
} // namespace scag2
