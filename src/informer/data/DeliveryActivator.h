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
    virtual void finishStateChange( ulonglong    ymdTime,
                                    Delivery&    dlv,
                                    DlvState     oldState ) = 0;

    // log delivery state change
    /*
     // virtual void logStateChange( ulonglong    ymd,
                                 dlvid_type   dlvId,
                                 const char*  userId,
                                 DlvState     newState,
                                 msgtime_type planTime ) = 0;
     */
};

} // informer
} // smsc

#endif
