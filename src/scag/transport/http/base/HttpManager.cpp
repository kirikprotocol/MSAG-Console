#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "HttpManager.h"

using namespace scag2::transport::http;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< HttpManager, OuterCreation > Single;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( HttpManager* ) { return 5; }

}

namespace scag2 {
namespace transport {
namespace http {

HttpManager& HttpManager::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("HttpManager not inited");
    }
    return Single::Instance();
}

HttpManager::HttpManager()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}

HttpManager::~HttpManager()
{
    MutexGuard mg(mtx);
    inited = false;
}

}
}
}
