#ifndef _INFORMER_RECEIPTPROCESSOR_H
#define _INFORMER_RECEIPTPROCESSOR_H

namespace eyeline {
namespace informer {

class CommonSettings;
struct DlvRegMsgId;

class ReceiptProcessor
{
public:
    virtual ~ReceiptProcessor() {}

    virtual const CommonSettings& getCS() const = 0;

    virtual void receiveReceipt( const DlvRegMsgId& drm,
                                 int                smppStatus,
                                 bool               retry ) = 0;
    virtual bool receiveResponse( const DlvRegMsgId& drm ) = 0;

    virtual void incIncoming() = 0;
    virtual void incOutgoing( unsigned nchunks ) = 0;
};

} // informer
} // smsc

#endif
