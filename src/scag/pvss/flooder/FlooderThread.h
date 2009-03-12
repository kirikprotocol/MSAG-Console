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
    FlooderThread( FlooderStat& flooderStat);

public:
    virtual const char* taskName() { return "pvss.fldr"; }

protected:
    /// generate the next request
    std::auto_ptr<Request> generate();

protected:
    smsc::logger::Logger*                           log_;
    FlooderStat&                                    stat_;
    unsigned                                        pattern_;
    ProfileKey                                      profileKey_;
};


class AsyncFlooderThread : public FlooderThread
{
public:
    AsyncFlooderThread( FlooderStat&      flooderStat ) :
    FlooderThread(flooderStat) {}
    virtual int Execute();
};

class SyncFlooderThread : public FlooderThread
{
public:
    SyncFlooderThread( FlooderStat&          flooderStat) :
    FlooderThread(flooderStat) {}
    virtual int Execute();
};

} // namespace flooder
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_FLOODER_FLOODERTHREAD_H */
