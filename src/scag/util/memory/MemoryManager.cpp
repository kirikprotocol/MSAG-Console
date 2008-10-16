#include "MemoryManager.h"
#include "scag/util/singleton/Singleton.h"

unsigned GetLongevity( scag::util::memory::MemoryManager* ) { return 0xffffffff; }

namespace scag {
namespace util {
namespace memory {

struct SingleMemoryManagerInstance : public MemoryManager
{
    SingleMemoryManagerInstance() {}
    ~SingleMemoryManagerInstance() {}
};


MemoryManager& MemoryManager::Instance() {
    return scag::util::singleton::SingletonHolder< SingleMemoryManagerInstance >::Instance();
}

}
}
}
