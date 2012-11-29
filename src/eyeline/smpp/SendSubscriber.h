#ifndef _EYELINE_SMPP_SENDSUBSCRIBER_H
#define _EYELINE_SMPP_SENDSUBSCRIBER_H

#include <exception>

namespace eyeline {
namespace smpp {

/// An interface to be notified on the results of Socket::send().
/// It is useful for example for resp registry arrangement.
class SendSubscriber
{
public:
    virtual void wasSent( Socket& socket,
                          uint32_t seqNum,
                          bool isRequest ) = 0;
    /// pdu is failed to be sent
    /// @param exc details of failure (may be missing)
    virtual void failedToSend( Socket& socket,
                               uint32_t seqNum,
                               bool isRequest,
                               const std::exception* exc ) = 0;
protected:
    virtual ~SendSubscriber() {}
};

}
}

#endif
