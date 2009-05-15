#ifndef _SCAG_PVSS_BASE_PACKET_H
#define _SCAG_PVSS_BASE_PACKET_H

#include <string>
#include "util/int.h"
#include "logger/Logger.h"
#include "scag/util/HRTimer.h"

namespace scag2 {
namespace pvss {

class PvssException;

///
/// abstract class Packet
///
class Packet
{
protected:
    static smsc::logger::Logger* log_;
    static smsc::logger::Logger* logtm_;

public:
    struct Timing {
        std::string    result;
        util::HRTiming timing;
        unsigned       total;
        Timing() : total(0) {
            timing.reset(result);
            result.reserve(400);
        }
        /// correlated constructor
        Timing( const Timing& t ) : timing(t.timing), total(0) {
            result.reserve(400);
        }

        /// merge timing info back
        void merge( const Timing& t ) {
            if (!t.result.empty()) {
                result.push_back(' ');
                result.append(t.result);
            }
            timing.setTimer(t.timing);
            total += t.total;
        }
    };

    enum {
            CONNECT_RESPONSE_OK = 0x4f4b,         // "OK"
            CONNECT_RESPONSE_SERVER_BUSY = 0x5342 // "SB"
    };

public:
    virtual ~Packet() {}

    virtual bool isRequest() const = 0;
    /// if false exc will contain details
    virtual bool isValid( PvssException* exc = 0 ) const = 0;
    virtual std::string toString() const = 0;
    virtual uint32_t  getSeqNum() const = 0;
    virtual void setSeqNum( uint32_t seqNum ) = 0;
    virtual Packet* clone() const = 0;
    virtual const char* typeToString() const = 0;

    /// report timing in different parts of program
    virtual bool hasTiming() const { return false; }
    /// NOTE: that timing information is not a part of packet, so
    /// const is quite reasonable here.
    virtual void timingMark( const char* where ) const {}
    virtual void timingComment( const char* comment ) const {}
    /// this method is necessary to bind timing measurements in response
    /// to the corresponding request.
    virtual const Timing* getTiming() const { return 0; }

protected:
    void initLog();
    void logDtor();
};

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PACKET_H */
