#include <stdexcept>
#include "scag/util/singleton/Singleton2.h"
#include "core/synchronization/Mutex.hpp"
#include "ConfigManager2.h"

using namespace scag2::config;
using namespace scag2::util::singleton;
using namespace smsc::core::synchronization;

namespace {

typedef SingletonHolder< ConfigManager, OuterCreation > Single;
bool inited = false;
Mutex mtx;

inline unsigned GetLongevity( ConfigManager* ) { return 256; }

}


namespace scag2 {
namespace config {

ConfigManager& ConfigManager::Instance()
{
    if ( ! inited ) {
        MutexGuard mg(mtx);
        if ( ! inited )
            throw std::runtime_error("ConfigManager not inited");
    }
    return Single::Instance();
}


ConfigManager::~ConfigManager()
{
    MutexGuard mg(mtx);
    inited = false;
}


ConfigManager::ConfigManager()
{
    MutexGuard mg(mtx);
    assert( ! inited );
    inited = true;
    Single::setInstance( this );
}

} // namespace config
} // namespace scag2
