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
    IOTask(theconfig,thecore,taskname), writePending_(false) {}

    /// serialize the packet
    void serialize( const Packet& packet,
                    Protocol::Buffer& buffer) /* throw (PvssException) */ ;

    /// notify writer that it should write all pending contexts,
    /// wait until all contexts are written.  Typically it should be invoked before shutdown.
    void writePending();

protected:
    virtual void attachToSocket( PvssSocket& socket ) {
        socket.registerWriter(this);
    }
    virtual void detachFromSocket( PvssSocket& socket ) {
        socket.registerWriter(0);
    }

    virtual bool setupSocket( PvssSocket& conn, util::msectime_type currentTime ) {
        if ( ! conn.wantToSend(currentTime) ) return false;
        mul_.add( conn.getSocket() );
        return true;
    }
    virtual bool hasEvents() { return mul_.canWrite(ready_, error_, 200); }
    virtual void process( PvssSocket& con ) { con.sendData(); }

    virtual int setupFailed( int tmo );

private:
    smsc::core::synchronization::EventMonitor pmon_;
    bool  writePending_;
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
