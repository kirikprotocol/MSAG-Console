#ifndef _SCAG_PVSS_CORE_PACKETWRITER_H
#define _SCAG_PVSS_CORE_PACKETWRITER_H

#include <memory>
#include "IOTask.h"
#include "Context.h"
#include "PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {

class PacketWriter : public IOTask
{
public:
    PacketWriter( Config& theconfig,
                  Core& thecore,
                  const char* taskname = "pvss.wtask" ) :
    IOTask(theconfig,thecore,taskname) {}

    /// serialize the packet
    void serialize(const Packet& packet,Protocol::Buffer& buffer) throw (PvssException);

protected:
    virtual void attachToSocket( PvssSocket& socket ) {
        socket.registerWriter(this);
    }
    virtual void detachFromSocket( PvssSocket& socket ) {
        socket.registerWriter(0);
    }

    virtual bool setupSocket( PvssSocket& conn ) {
        // FIXME: was used for sync protocol
        // if ( ! pers_->async && conn.isReading() ) return false;
        // hasAvailable_ = true;
        if ( ! conn.wantToSend() ) return false;
        mul_.add( conn.socket() );
        return true;
    }
    virtual bool hasEvents() { return mul_.canWrite(ready_, error_, 200); }
    virtual void process( PvssSocket& con ) { con.sendData(*core_); }

};

} // namespace core
} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {
namespace core {

using scag2::pvss::core::PacketWriter;

} // namespace core
} // namespace pvss
} // namespace scag

#endif /* !_SCAG_PVSS_CORE_PACKETWRITER_H */
