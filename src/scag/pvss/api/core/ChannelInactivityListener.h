#ifndef _SCAG_PVSS_CORE_CHANNELINACTIVITYLISTENER_H
#define _SCAG_PVSS_CORE_CHANNELINACTIVITYLISTENER_H

namespace smsc {
namespace core {
namespace network {
class Socket;
}
}
}


namespace scag2 {
namespace pvss {
namespace core {

class ChannelInactivityListener
{
public:
    virtual ~ChannelInactivityListener() {}
    virtual void inactivityTimeoutExpired( smsc::core::network::Socket& channel ) = 0;
};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CHANNELACTIVITYLISTENER_H */
