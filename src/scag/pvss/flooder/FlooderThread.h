#ifndef _SCAG_PVSS_FLOODER_FLOODERTHREAD_H
#define _SCAG_PVSS_FLOODER_FLOODERTHREAD_H

#include <memory>
#include <vector>
#include "logger/Logger.h"
#include "scag/util/WatchedThreadedTask.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/data/ProfileKey.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace client {
class Client;
}
}
}
}

namespace scag2 {
namespace pvss {

class AbstractCommand;

namespace flooder {

class FlooderConfig;
class FlooderStat;
class RequestGenerator;

class FlooderThread : public util::WatchedThreadedTask
{

protected:
    FlooderThread( FlooderStat& flooderStat, bool oneCommandPerAbonent = false );

public:
    virtual const char* taskName() { return "pvss.fldr"; }

protected:
    /// generate the next request
    std::auto_ptr<Request> generate();
    virtual int doExecute();
    virtual void doProcessRequest( std::auto_ptr< Request >& req ) /* throw (PvssException) */  = 0;

protected:
    smsc::logger::Logger*                           log_;
    FlooderStat&                                    stat_;
    unsigned                                        pattern_;
    ProfileKey                                      profileKey_;
    bool                                            oneCommandPerAbonent_;
};


class AsyncFlooderThread : public FlooderThread
{
public:
    AsyncFlooderThread( FlooderStat& flooderStat, bool oneCommandPerAbonent = false ) :
    FlooderThread(flooderStat,oneCommandPerAbonent) {}
protected:
    virtual void doProcessRequest( std::auto_ptr< Request >& req ) /* throw (PvssException) */ ;
};

class SyncFlooderThread : public FlooderThread
{
public:
    SyncFlooderThread( FlooderStat& flooderStat, bool oneCommandPerAbonent = false) :
    FlooderThread(flooderStat,oneCommandPerAbonent) {}
protected:
    virtual void doProcessRequest( std::auto_ptr< Request >& req ) /* throw (PvssException) */ ;
};

} // namespace flooder
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_FLOODER_FLOODERTHREAD_H */
