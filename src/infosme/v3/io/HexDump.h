#ifndef _INFORMER_HEXDUMP_H
#define _INFORMER_HEXDUMP_H

#include <memory>
#include <vector>

namespace eyeline {
namespace informer {
namespace io {

class HexDump
{
public:
    typedef std::vector< char > string_type;

    // requested size of the buffer
    size_t hexdumpsize( size_t insize ) const { return insize*3; }

    // hex dump insize bytes of inbuf to outbuf and return the position after last dumped
    char* hexdump( char* outbuf, const void* inbuf, size_t insize );
    size_t strdumpsize( size_t insize ) const { return insize; }
    char* strdump( char* outbuf, const void* inbuf, size_t insize );
    size_t utfdumpsize( size_t insize ) const { return insize; }
    char* utfdump( char* outbuf, const void* inbuf, size_t insize );

    // NOTE: w/o termination null-byte
    char* addstr( char* outbuf, const char* cstring );

    void hexdump( string_type& out, const void* inbuf, size_t insize );
    void strdump( string_type& out, const void* inbuf, size_t insize );
    void utfdump( string_type& out, const void* inbuf, size_t insize );
    void addstr( string_type& out, const char* cstring );

    // used for printing
    const char* c_str( const string_type& str ) const { return str.empty() ? "" : &str[0]; }

private:
    static std::auto_ptr<char> digits_;
};

}
}
}

#endif /* !_SCAG_UTIL_HEXDUMP_H */
