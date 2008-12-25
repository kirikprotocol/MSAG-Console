#ifndef _SCAG_UTIL_HEXDUMP_H
#define _SCAG_UTIL_HEXDUMP_H

#include <memory>

namespace scag2 {
namespace util {

class HexDump
{
public:
    // requested size of the buffer
    unsigned hexdumpsize( unsigned insize ) const { return insize*3; }
    char* hexdump( char* outbuf, const char* inbuf, unsigned insize );
    unsigned strdumpsize( unsigned insize ) const { return insize; }
    char* strdump( char* outbuf, const char* inbuf, unsigned insize );

    // NOTE: w/o termination null-byte
    char* addstr( char* outbuf, const char* cstring );

private:
    static std::auto_ptr<char> digits_;
};

} // namespace util
} // namespace scag2


namespace scag {
namespace util {
using scag2::util::HexDump;
}
}

#endif /* !_SCAG_UTIL_HEXDUMP_H */
