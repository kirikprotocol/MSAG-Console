#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "HttpRouter.h"

using namespace scag2::transport::http;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< HttpTraceRouter, OuterCreation > Single;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( HttpTraceRouter* ) { return 5; }

}

namespace scag2 {
namespace transport {
namespace http {

HttpTraceRouter& HttpTraceRouter::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("HttpTraceRouter not inited");
    }
    return Single::Instance();
}

HttpTraceRouter::HttpTraceRouter()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}

HttpTraceRouter::~HttpTraceRouter()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace http
} // namespace transport
} // namespace scag2
