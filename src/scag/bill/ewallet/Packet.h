#ifndef SCAG_BILL_EWALLET_PACKET_H
#define SCAG_BILL_EWALLET_PACKET_H

#include <string>
#include "util/int.h"
#include "util/TypeInfo.h"

namespace scag2 {
namespace bill {
namespace ewallet {

/// a packet
class Packet
{
public:
    virtual ~Packet() {
        CHECKMAGTC;
    }
    virtual bool isRequest() const = 0;
    virtual uint32_t getSeqNum() const {
        CHECKMAGTC;
        return seqNum_; 
    }
    virtual void setSeqNum( uint32_t seq ) {
        CHECKMAGTC;
        seqNum_ = seq;
    }
    virtual std::string toString() const {
        CHECKMAGTC;
        char buf[40];
        snprintf(buf,sizeof(buf),"%s seqNum=%u",typeToString(),seqNum_);
        return buf;
    }
    virtual const char* typeToString() const = 0;
    virtual bool isValid() const = 0;

protected:
    DECLMAGTC(Packet);
private:
    uint32_t seqNum_;
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_PACKET_H */
