#ifndef _SCAG_PVSS_CORE_CHANNELINACTIVITYLISTENER_H
#define _SCAG_PVSS_CORE_CHANNELINACTIVITYLISTENER_H

namespace scag2 {
namespace pvss {
namespace core {

class PvssSocket;

class ChannelInactivityListener
{
public:
    virtual ~ChannelInactivityListener() {}
    virtual void inactivityTimeoutExpired( PvssSocket& channel ) = 0;
};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CHANNELACTIVITYLISTENER_H */
