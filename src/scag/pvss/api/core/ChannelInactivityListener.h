#ifndef _SCAG_PVSS_CORE_CHANNELINACTIVITYLISTENER_H
#define _SCAG_PVSS_CORE_CHANNELINACTIVITYLISTENER_H

#include "PvssSocket.h"

namespace scag2 {
namespace pvss {
namespace core {

class ChannelInactivityListener
{
public:
    virtual ~ChannelInactivityListener() {}
    virtual void inactivityTimeoutExpired( PvssSocketBase& channel ) = 0;
};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CHANNELACTIVITYLISTENER_H */
