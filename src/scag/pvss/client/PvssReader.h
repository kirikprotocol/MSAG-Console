#ifndef _SCAG_PVSS_CLIENT_PVSSREADER_H
#define _SCAG_PVSS_CLIENT_PVSSREADER_H

#include "PvssIOTask.h"
#include "PvssConnection.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssReader : public PvssIOTask
{
public:
    PvssReader( PvssStreamClient& pers ) : PvssIOTask(pers) {}
    virtual const char* taskName() { return "PvssReader"; }

protected:
    virtual bool setupSocket( PvssConnection& conn );
    // { mul_.add(conn.socket()); return true; }
    virtual bool hasEvents() { return mul_.canRead(ready_, error_, 200); }
    virtual void process( PvssConnection& con ) {
        con.setReady(true);
        con.processInput();
    }
    virtual void processEvents();
    virtual void processNoEvents();
};

} // namespace client
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CLIENT_PVSSREADER_H */
