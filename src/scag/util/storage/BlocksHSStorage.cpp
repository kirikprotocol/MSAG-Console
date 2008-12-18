#include "BlocksHSStorage.h"

namespace scag {
namespace util {
namespace storage {

// this f**king SunOS's CC is a shit.
#ifdef __GNUC__
uint32_t DescriptionFile::size_ = 0;
#endif

}
}
}
