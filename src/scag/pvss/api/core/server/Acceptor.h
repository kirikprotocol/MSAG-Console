#ifndef _SCAG_PVSS_CORE_SERVER_ACCEPTOR_H
#define _SCAG_PVSS_CORE_SERVER_ACCEPTOR_H

#include "scag/util/WatchedThreadedTask.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

/// interface
class Acceptor : public util::WatchedThreadedTask
{
public:
    Acceptor( const std::string& host, short port );

    virtual const char* taskName() { return "pvss.atask"; };

    virtual ~Acceptor();

    virtual int Execute();

private:
    std::string host_;
    short       port_;
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_ACCEPTOR_H */
