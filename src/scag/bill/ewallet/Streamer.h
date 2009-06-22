#ifndef SCAG_BILL_EWALLET_STREAMER_H
#define SCAG_BILL_EWALLET_STREAMER_H

#include <string>
#include "core/buffers/TmpBuf.hpp"
#include "scag/util/io/HexDump.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class Packet;

/// an interface to a ewallet protocol
class Streamer
{
public:
    typedef smsc::core::buffers::TmpBuf<char, 32> Buffer;

public:
    virtual ~Streamer() {}
    virtual void serialize( const Packet& packet, Buffer& buf ) /* throw */ = 0;
    virtual Packet* deserialize( Buffer& buf ) /* throw */ = 0;

    static std::string bufferDump( Buffer& buf ) {
        util::HexDump hd;
        util::HexDump::string_type s;
        hd.hexdump( s, buf.get(), buf.GetPos() );
        return hd.c_str(s);
    }
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_STREAMER_H */
