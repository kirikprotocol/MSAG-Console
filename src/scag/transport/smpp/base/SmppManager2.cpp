#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "SmppManager2.h"

using namespace scag2::transport::smpp;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< SmppManager, OuterCreation > Single;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( SmppManager* ) { return 5; }

}


namespace scag2 {
namespace transport {
namespace smpp {

SmppManager& SmppManager::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("SmppManager not inited");
    }
    return Single::Instance();
}

SmppManager::SmppManager()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}

SmppManager::~SmppManager()
{
    MutexGuard mg(mtx);
    inited = false;
}

}
}
}
