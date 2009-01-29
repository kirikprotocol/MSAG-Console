#include "TypeId.h"
#include "core/synchronization/Mutex.hpp"

namespace scag {
namespace pvss {
namespace pvap {

int TypeIdFactory::getId()
{
    static int currentId = 0;
    return ++currentId;
}

} // namespace pvap
} // namespace pvss
} // namespace scag
