#ifndef _BILLING_MANAGER_H_
#define _BILLING_MANAGER_H_

#include "sms/sms.h"
#include <core/synchronization/EventMonitor.hpp>
#include "scag/config/bill/BillingManagerConfig.h"

#include "infrastruct/Infrastructure.h"

namespace scag { namespace bill {

using namespace smsc::sms;
using namespace smsc::core::synchronization;
using namespace scag::config;
using namespace scag::bill::infrastruct;

struct CTransactionData
{
    Address OA;
    Address DA;

    std::string mediatype;
    std::string category;
};

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
    TRANSACTION_TIME_OUT        = 4,
    TRANSACTION_REFUSED         = 5
};
/*
1-open
2-commit
3-rollback
4-rollback_by_timeout */


class BillingManager
{
    BillingManager(const BillingManager& bm);
    BillingManager& operator=(const BillingManager& bm);
protected:
    virtual ~BillingManager() {};
    BillingManager() {};

public:
    virtual int ChargeBill(CTransactionData& data) = 0;  //¬озвращ€ет billId
    virtual TransactionStatus CheckBill(int billId, EventMonitor * eventMonitor) = 0; //”станавливает eventMonitor и возвращ€ет статус транзакции
    virtual TransactionStatus GetStatus(int billId) = 0; //¬озвращ€ет статус транзакции

    virtual void commit(int billId) = 0;
    virtual void rollback(int billId) = 0;

    static BillingManager& Instance();
    static void Init(BillingManagerConfig& cfg);
    //static const ActionFactory * getActionFactory() {return &factory;}

    virtual Infrastructure& getInfrastructure() = 0;
    virtual CTransactionData getTransactionData(int billId) = 0;
};


}}

#endif
