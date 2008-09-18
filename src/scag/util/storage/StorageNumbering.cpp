#include <cassert>
#include "StorageNumbering.h"

namespace {
    scag::util::storage::StorageNumbering* inst_ = 0;
}

namespace scag {
namespace util {
namespace storage {

/// right now the storage numbering could be set only once
/// at the beginning of the program.
/// if you want to change the number of nodes,
/// please use some kind of locking here.

const StorageNumbering& StorageNumbering::instance()
{
    assert( inst_ );
    return *inst_;
}

void StorageNumbering::setInstance( unsigned nodes,
                                    unsigned stores )
{
    assert( !inst_ );
    inst_ = new StorageNumbering( nodes, stores );
}

void StorageNumbering::setNodes( unsigned nodes )
{
    nodes_ = nodes;
}

} // namespace storage
} // namespace util
} // namespace scag
