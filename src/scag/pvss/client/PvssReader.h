#ifndef _SCAG_PVSS_CLIENT_PVSSREADER_H
#define _SCAG_PVSS_CLIENT_PVSSREADER_H

#include "PvssIOTask.h"
#include "PvssConnection.h"
#include "scag/util/MsecTime.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssReader : public PvssIOTask
{
public:
    PvssReader( PvssStreamClient& pers ) : PvssIOTask(pers), lastCheck_(0) {}
    virtual const char* taskName() { return "PvssReader"; }

protected:
    virtual bool setupSocket( PvssConnection& conn );
    // { mul_.add(conn.socket()); return true; }
    virtual bool hasEvents() { return mul_.canRead(ready_, error_, 200); }
    virtual void process( PvssConnection& con ) {con.processInput();}
    virtual void postProcess();

private:
    util::MsecTime            time0_;
    util::MsecTime::time_type lastCheck_;
};

} // namespace client
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CLIENT_PVSSREADER_H */
