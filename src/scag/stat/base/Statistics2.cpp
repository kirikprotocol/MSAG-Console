#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "Statistics2.h"

using namespace scag2::stat;
using namespace scag2::util::singleton;

namespace {

typedef SingletonHolder< Statistics, OuterCreation > Single;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( Statistics* ) { return 7; }

}

namespace scag2 {
namespace stat {

Statistics& Statistics::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("Statistics not inited");
    }
    return Single::Instance();
}

Statistics::Statistics()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}

Statistics::~Statistics()
{
    MutexGuard mg(mtx);
    inited = false;
}

} // namespace stat
} // namespace scag2
