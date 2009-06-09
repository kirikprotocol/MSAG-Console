#ifndef _SCAG_UTIL_STORAGE_ENDIANCONVERTER_H
#define _SCAG_UTIL_STORAGE_ENDIANCONVERTER_H

#include "util/int.h"
#include "util/byteorder.h"
#include <netinet/in.h>
#include "string.h"

namespace scag {
namespace util {
namespace io {

/// NOTE: code in this class is moved into .cpp as we would like to
/// compile it with high optimization level.
struct EndianConverter
{
    // methods for inplace conversion
    static uint16_t get16( const void* buf );
    static uint32_t get32( const void* buf );
    static uint64_t get64( const void* buf );

    static void set16( void* buf, uint16_t i );
    static void set32( void* buf, uint32_t i );
    static void set64( void* buf, uint64_t i );
};

}
}
}

namespace scag2 {
namespace util {
namespace io {
using namespace scag::util::io;
}
}
}

#endif /* !_SCAG_UTIL_STORAGE_ENDIANCONVERTER_H */
