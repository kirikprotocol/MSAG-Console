#ifndef _INFORMER_DELIVERYACTIVATOR_H
#define _INFORMER_DELIVERYACTIVATOR_H

#include <vector>
#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

class RegionFinder;
class InputTransferTask;
class ResendTransferTask;
class Delivery;
class BindSignal;
class Message;

/// an interface for applying some activities on deliveries.
class DeliveryActivator
{
public:
    virtual ~DeliveryActivator() {}

    virtual bool isStopping() const = 0;

    /// get region finder
    virtual RegionFinder& getRegionFinder() = 0;

    /// NOTE: vector regs will be emptied!
    /// @param bind: bind dlv and regs if true, unbind otherwise.
    virtual void deliveryRegions( dlvid_type dlvId,
                                  std::vector<regionid_type>& regs,
                                  bool bind ) = 0;

    /// start a task transferring messages from inputstore into opstore.
    virtual void startInputTransfer( InputTransferTask* ) = 0;
    virtual void startResendTransfer( ResendTransferTask* ) = 0;

    /// finish state change.
    /// NOTE: should be invoked from delivery itself only!
    virtual void finishStateChange( msgtime_type       currentTime,
                                    ulonglong          ymdTime,
                                    BindSignal&        bs,
                                    const Delivery&    dlv ) = 0;

    struct PvssNotifyee
    {
        virtual ~PvssNotifyee() {}
        virtual void notify() = 0;
    };

    /// start checking of message via pvss.
    /// NOTE: msg.timeLeft should be set to -1 on input (waiting for pvss reply),
    /// and it will be set to some result value after pvss check is finished:
    ///  0 - message is blocked by pvss;
    ///  1 - message is allowed by pvss.
    virtual void startPvssCheck( PvssNotifyee& notifyee, Message& msg ) = 0;
};

} // informer
} // smsc

#endif
