#ifndef _INFORMER_RECEIPTPROCESSOR_H
#define _INFORMER_RECEIPTPROCESSOR_H

namespace eyeline {
namespace informer {

struct DlvRegMsgId;
class RetryPolicy;

class ReceiptProcessor
{
public:
    virtual ~ReceiptProcessor() {}

    virtual void receiveReceipt( const DlvRegMsgId& drm,
                                 const RetryPolicy& policy,
                                 int                smppStatus,
                                 bool               retry,
                                 unsigned           nchunks,
                                 bool               fixTransactional = false ) = 0;
    virtual bool receiveResponse( const DlvRegMsgId& drm ) = 0;

    virtual void incIncoming() = 0;
    virtual void incOutgoing( unsigned nchunks ) = 0;
};

} // informer
} // smsc

#endif
