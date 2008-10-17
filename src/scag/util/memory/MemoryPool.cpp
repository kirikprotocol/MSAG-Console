#include "MemoryPool.h"
#include "scag/util/singleton/Singleton.h"

unsigned GetLongevity( scag::util::memory::MemoryPool* ) { return 0xffffffff; }

namespace scag {
namespace util {
namespace memory {

struct SingleMemoryPoolInstance : public MemoryPool
{
    SingleMemoryPoolInstance() {}
    ~SingleMemoryPoolInstance() {}
};


MemoryPool& MemoryPool::Instance() {
    return scag::util::singleton::SingletonHolder< SingleMemoryPoolInstance >::Instance();
}

}
}
}
