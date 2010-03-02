#ifndef _SCAG_UTIL_STORAGE_HEAPALLOCATOR_H
#define _SCAG_UTIL_STORAGE_HEAPALLOCATOR_H

namespace scag2 {
namespace util {
namespace storage {

template < class Key, class Val > class HeapAllocator
{
protected:
    ~HeapAllocator() {}
    inline Val* alloc( const Key& k ) const { return new Val(k); }
};

}
}
}

namespace scag {
namespace util {
namespace storage {
using scag2::util::storage::HeapAllocator;
}
}
}

#endif
