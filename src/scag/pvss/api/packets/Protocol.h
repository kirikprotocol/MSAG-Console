#ifndef _SCAG_PVSS_PACKETS_PROTOCOL_H
#define _SCAG_PVSS_PACKETS_PROTOCOL_H

#include "core/buffers/TmpBuf.hpp"
#include "scag/pvss/common/PvapException.h"

namespace scag2 {
namespace pvss {

class Packet;

class Protocol
{
public:
    typedef smsc::core::buffers::TmpBuf< char, 32 > Buffer;

    virtual ~Protocol() {}

    /**
     * Serialize packet into a buffer and return 
     */
    virtual void serialize( const Packet& packet, Buffer& buf ) /* throw(PvapException) */  = 0;

    /**
     * NOTE: buf will be used only for reading, so it is safe to pass an extBuf.
     */
    virtual Packet* deserialize( Buffer& buf ) /* throw(PvapException) */  = 0;

};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_PACKETS_PROTOCOL_H */
