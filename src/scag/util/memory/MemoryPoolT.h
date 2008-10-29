#ifndef _SCAG_UTIL_MEMORY_MEMORYPOOLT_H
#define _SCAG_UTIL_MEMORY_MEMORYPOOLT_H

#include "MemoryPool.h"
#include "scag/util/singleton/Singleton2.h"

namespace scag {
namespace util {
namespace memory {

/// This memory manager may be used for providing a local pool of objects
/// for a particular type.
template < typename T >
    class MemoryPoolT : public MemoryPool
{
public:
    static MemoryPool& Instance() {
        return util::singleton::SingletonHolder< MemoryPoolT< T > >::Instance();
    }
protected:
    friend class util::singleton::CreateUsingNew< MemoryPoolT< T > >;
    MemoryPoolT() {}
    ~MemoryPoolT() {}
};

}
}
}

template < class T >
    unsigned GetLongevity( scag::util::memory::MemoryPoolT< T >* ) {
        return 0xfffffff0;
    }

#endif /* !_SCAG_UTIL_MEMORY_MEMORYPOOLT_H */
