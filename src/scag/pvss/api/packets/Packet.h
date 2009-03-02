#ifndef _SCAG_PVSS_BASE_PACKET_H
#define _SCAG_PVSS_BASE_PACKET_H

#include <string>
#include "util/int.h"

namespace scag2 {
namespace pvss {

///
/// abstract class Packet
///
class Packet
{
public:
    virtual ~Packet() {}

    virtual bool isRequest() const = 0;
    virtual bool isValid() const = 0;
    virtual uint32_t  getSeqNum() const = 0;
    virtual std::string toString() const = 0;
    virtual void setSeqNum( uint32_t seqNum ) = 0;
    virtual Packet* clone() const = 0;
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PACKET_H */
