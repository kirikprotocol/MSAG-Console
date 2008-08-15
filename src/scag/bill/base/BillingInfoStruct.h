#ifndef _SCAG_BILL_BASE_BILLINGINFOSTRUCT_H
#define _SCAG_BILL_BASE_BILLINGINFOSTRUCT_H

namespace scag2 {
namespace bill {

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
    std::string mediaType, category;
    std::string AbonentNumber;
    int serviceId;
    int protocol;
    int providerId;
    int operatorId;
    int msgRef;

    timeval SessionBornMicrotime;

    const BillingInfoStruct& operator=(const BillingInfoStruct& cp)
    {
        mediaType = cp.mediaType;
        category = cp.category;
        AbonentNumber = cp.AbonentNumber;
        serviceId = cp.serviceId;
        protocol = cp.protocol;
        providerId = cp.providerId;
        operatorId = cp.operatorId;
        msgRef = cp.msgRef;
        SessionBornMicrotime = cp.SessionBornMicrotime;
        return *this;
    }
};

}
}

#endif /* ! _SCAG_BILL_BASE_BILLINGINFOSTRUCT_H */
