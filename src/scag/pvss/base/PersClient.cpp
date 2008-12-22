#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "PersClient.h"

using namespace scag2::pvss;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< PersClient, OuterCreation > SingleSM;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( PersClient* ) { return 251; }

}

namespace scag2 {
namespace pvss {

PersClient& PersClient::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("PersClient not inited");
    }
    return SingleSM::Instance();
}

PersClient::PersClient()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    SingleSM::setInstance( this );
    inited = true;
}

PersClient::~PersClient()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace pvss
} // namespace scag2
