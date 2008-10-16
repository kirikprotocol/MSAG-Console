#ifndef _SCAG_UTIL_MEMORY_MEMORYMANAGERT_H
#define _SCAG_UTIL_MEMORY_MEMORYMANAGERT_H

#include "MemoryManager.h"
#include "scag/util/singleton/SingletonHolder.h"

namespace scag {
namespace util {
namespace memory {

/// This memory manager may be used for providing a local pool of objects
/// for a particular type.
template < typename T >
    class MemoryManagerT : public MemoryManager
{
public:
    MemoryManager& Instance() {
        return util::singleton::SingletonHolder< MemoryManagerT< T > >::Instance();
    }
protected:
    friend class util::singleton::SingletonHolder< MemoryManagerT< T > >;
    MemoryManagerT() {}
    ~MemoryManagerT() {}
};

}
}
}

#endif /* !_SCAG_UTIL_MEMORY_MEMORYMANAGERT_H */
