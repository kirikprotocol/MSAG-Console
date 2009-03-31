#ifndef _SCAG_PVSS_CORE_PACKETREADER_H
#define _SCAG_PVSS_CORE_PACKETREADER_H

#include "IOTask.h"
#include "PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {

class PacketWriter;

class PacketReader : public IOTask
{
public:
    PacketReader( Config& theconfig,
                  Core& thecore,
                  const char* taskname = "pvss.rtask" ) :
    IOTask(theconfig,thecore,taskname) {}

    virtual void shutdown() {
        IOTask::shutdown();
        waitUntilReleased();
    }

protected:
    virtual void attachToSocket( PvssSocket& socket ) {
        socket.registerReader(this);
    }
    virtual void detachFromSocket( PvssSocket& socket ) {
        socket.registerReader(0);
    }

    virtual bool setupSocket( PvssSocket& conn, util::msectime_type currentTime );
    virtual bool hasEvents() { return mul_.canRead(ready_, error_, 200 /* FIXME: timeout from config */); }
    virtual void process( PvssSocket& con ) {
        con.processInput();
    }
    virtual void postProcess() {}
};

} // namespace core
} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {
namespace core {

using scag2::pvss::core::PacketReader;

}
}
}

#endif /* !_SCAG_PVSS_CORE_PACKETREADER_H */
