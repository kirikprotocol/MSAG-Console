#ifndef _INFOSME_V3_RECEIPTRECEIVER_H
#define _INFOSME_V3_RECEIPTRECEIVER_H

namespace smsc {
namespace infosme {

class ReceiptReceiver
{
public:
    virtual void receiptReceived( msgtime_type       currentTime,
                                  const DlvRegMsgId& msgId,
                                  uint8_t            state,
                                  int                smppStatus ) = 0;
};

} // infosme
} // smsc

#endif
