#ifndef _SCAG_PVSS_BASE_PACKET_H
#define _SCAG_PVSS_BASE_PACKET_H

#include <string>
#include "util/int.h"
#include "logger/Logger.h"

namespace scag2 {
namespace pvss {

///
/// abstract class Packet
///
class Packet
{
protected:
    static smsc::logger::Logger* log_;

public:
    enum {
            CONNECT_RESPONSE_OK = 0x4f4b,         // "OK"
            CONNECT_RESPONSE_SERVER_BUSY = 0x5342 // "SB"
    };

public:
    virtual ~Packet() {}

    virtual bool isRequest() const = 0;
    virtual bool isValid() const = 0;
    virtual std::string toString() const = 0;
    virtual uint32_t  getSeqNum() const = 0;
    virtual void setSeqNum( uint32_t seqNum ) = 0;
    virtual Packet* clone() const = 0;

protected:
    void initLog();
    void logDtor();
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PACKET_H */
