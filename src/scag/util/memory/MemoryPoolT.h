#ifndef _SCAG_UTIL_MEMORY_MEMORYPOOLT_H
#define _SCAG_UTIL_MEMORY_MEMORYPOOLT_H

#include "MemoryPool.h"
#include "scag/util/singleton/SingletonHolder.h"

namespace scag {
namespace util {
namespace memory {

/// This memory manager may be used for providing a local pool of objects
/// for a particular type.
template < typename T >
    class MemoryPoolT : public MemoryPool
{
public:
    MemoryPool& Instance() {
        return util::singleton::SingletonHolder< MemoryPoolT< T > >::Instance();
    }
protected:
    friend class util::singleton::SingletonHolder< MemoryPoolT< T > >;
    MemoryPoolT() {}
    ~MemoryPoolT() {}
};

}
}
}

#endif /* !_SCAG_UTIL_MEMORY_MEMORYPOOLT_H */
