#ifndef _SCAG_BILL_BASE_BILLINGINFOSTRUCT_H
#define _SCAG_BILL_BASE_BILLINGINFOSTRUCT_H

#include "scag/util/Time.h"

namespace scag2 {
namespace bill {

typedef util::msectime_type billid_type;

enum TransactionStatus
{
    TRANSACTION_NOT_STARTED = 0,
    TRANSACTION_WAIT_ANSWER = 1,
    TRANSACTION_VALID       = 2,
    TRANSACTION_INVALID     = 3
};

enum BillingTransactionEvent
{
    TRANSACTION_OPEN            = 1,
    TRANSACTION_COMMITED        = 2,
    TRANSACTION_CALL_ROLLBACK   = 3,
    TRANSACTION_TIME_OUT        = 4
    //TRANSACTION_REFUSED         = 5
};

enum BillingCommandStatus
{
    COMMAND_SUCCESSFULL         =0,
    SERVER_NOT_RESPONSE         =1,
    REJECTED_BY_SERVER          =2,
    EXTERNAL_ERROR              =3,
    INVALID_TRANSACTION         =4
};
/*
1-open
2-commit
3-rollback
4-rollback_by_timeout */


struct BillingInfoStruct
{
    billid_type billId; // unique: filled by BillingManager, value of 0 means 'not filled'

    std::string mediaType, category;
    std::string AbonentNumber;
    int serviceId;
    int protocol;
    int providerId;
    int operatorId;
    int msgRef;
    // wallet type is taken from tariffRec (the currency field)
    // std::string walletType;  // subdivision (empty -- not used)
    int timeout;             // 0 -- not used
    std::string description; // description (empty -- not used)
    std::string externalId;  // external id (empty -- not used)

    timeval SessionBornMicrotime;

    BillingInfoStruct() :
    billId(0),
    timeout(0)
    {}

    const BillingInfoStruct& operator=(const BillingInfoStruct& cp)
    {
        if ( this == &cp ) return *this;
        billId = cp.billId;
        mediaType = cp.mediaType;
        category = cp.category;
        AbonentNumber = cp.AbonentNumber;
        serviceId = cp.serviceId;
        protocol = cp.protocol;
        providerId = cp.providerId;
        operatorId = cp.operatorId;
        msgRef = cp.msgRef;
        timeout = cp.timeout;
        description = cp.description;
        externalId = cp.externalId;
        SessionBornMicrotime = cp.SessionBornMicrotime;
        return *this;
    }
};

}
}

#endif /* ! _SCAG_BILL_BASE_BILLINGINFOSTRUCT_H */
