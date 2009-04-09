#include "BlocksHSStorage2.h"
#include "core/synchronization/Mutex.hpp"

using namespace smsc::core::synchronization;

namespace {
Mutex sizeLock;
}

namespace scag2 {
namespace util {
namespace storage {

size_t BlocksHSStorage2::BlockNavigation::persistentSize_ = 0;

void BlocksHSStorage2::BlockNavigation::calculatePersistentSize()
{
    MutexGuard mg(sizeLock);
    if (persistentSize_) return;
    buffer_type buf;
    Serializer ser(buf);
    BlockNavigation bn;
    bn.save(ser);
    persistentSize_ = ser.wpos();
}

} // namespace storage
} // namespace util
} // namespace scag2
