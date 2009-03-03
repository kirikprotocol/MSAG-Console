#ifndef _SCAG_PVSS_CORE_SERVER_DISPATCHER_H
#define _SCAG_PVSS_CORE_SERVER_DISPATCHER_H

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

/// interface
class Dispatcher
{
public:
    virtual ~Dispatcher() {}
    virtual unsigned getIndex(Request& request, PvssSocket& channel) = 0;
    virtual Server::SyncLogic* getLogic(unsigned idx) = 0;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_DISPATCHER_H */
