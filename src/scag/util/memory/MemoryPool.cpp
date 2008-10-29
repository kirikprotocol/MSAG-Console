#include "MemoryPool.h"
#include "scag/util/singleton/Singleton2.h"

unsigned GetLongevity( scag::util::memory::MemoryPool* ) { return 0xfffffff0; }

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
