#ifndef _SCAG_UTIL_HEXDUMP_H
#define _SCAG_UTIL_HEXDUMP_H

#include <memory>
#include <string>

namespace scag2 {
namespace util {

class HexDump
{
public:
    // requested size of the buffer
    size_t hexdumpsize( size_t insize ) const { return insize*3; }
    char* hexdump( char* outbuf, const void* inbuf, size_t insize );
    size_t strdumpsize( size_t insize ) const { return insize; }
    char* strdump( char* outbuf, const void* inbuf, size_t insize );

    // NOTE: w/o termination null-byte
    char* addstr( char* outbuf, const char* cstring );

    void hexdump( std::string& out, const void* inbuf, size_t insize );
    void strdump( std::string& out, const void* inbuf, size_t insize );

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
