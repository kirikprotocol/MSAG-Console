#ifndef _SCAG_PVSS_CLIENT_PVSSCONNECTOR_H
#define _SCAG_PVSS_CLIENT_PVSSCONNECTOR_H

#include "PvssConnTask.h"
#include "PvssConnection.h"

namespace scag2 {
namespace pvss {
namespace client {

class PvssStreamClient;

class PvssConnector : public PvssConnTask
{
public:
    PvssConnector( PvssStreamClient& pers ) :
    PvssConnTask(pers, smsc::logger::Logger::getInstance("pvss.ctask")) {}
    virtual const char* taskName() { return "PvssConnector"; }

protected:
    virtual bool setupSockets();
    virtual bool hasEvents();
    virtual void processEvents();
    void removeConnected();

protected:
    smsc::core::buffers::Array< PvssConnection* > worksock_;
};

} // namespace client
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CLIENT_PVSSCONNECTOR_H */
