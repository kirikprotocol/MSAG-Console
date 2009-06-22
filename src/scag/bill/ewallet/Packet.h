#ifndef SCAG_BILL_EWALLET_PACKET_H
#define SCAG_BILL_EWALLET_PACKET_H

#include <string>
#include "util/int.h"

namespace scag2 {
namespace bill {
namespace ewallet {

/// a packet
class Packet
{
public:
    virtual ~Packet() {}
    virtual bool isRequest() const = 0;
    virtual uint32_t getSeqNum() const { return seqNum_; }
    virtual void setSeqNum( uint32_t seq ) { seqNum_ = seq; }
    virtual std::string toString() const = 0;
    virtual const char* typeToString() const = 0;
    virtual bool isValid() const = 0;

private:
    uint32_t seqNum_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PACKET_H */
