#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "HttpProcessor.h"

using namespace scag2::transport::http;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< HttpProcessor, OuterCreation > Single;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( HttpProcessor* ) { return 5; }

}

namespace scag2 {
namespace transport {
namespace http {

HttpProcessor& HttpProcessor::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("HttpProcessor not inited");
    }
    return Single::Instance();
}

HttpProcessor::HttpProcessor()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}

HttpProcessor::~HttpProcessor()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace http
} // namespace transport
} // namespace scag2
