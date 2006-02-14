#ifndef _BILLING_MANAGER_H_
#define _BILLING_MANAGER_H_

#include "sms/sms.h"
#include <core/synchronization/EventMonitor.hpp>


namespace scag { namespace bill {

using namespace smsc::sms;
using namespace smsc::core::synchronization;

struct CTransportId
{
    Address OA;
    Address DA;
};

enum TransactionStatus
{
    TRANSACTION_NOT_STARTED = 0,
    TRANSACTION_WAIT_ANSWER = 1,
    TRANSACTION_VALID       = 2,
    TRANSACTION_INVALID     = 3
};


class BillingManager
{
    BillingManager(const BillingManager& bm);
    BillingManager& operator=(const BillingManager& bm);
protected:
    virtual ~BillingManager() {};
    BillingManager() {};

public:
    virtual int ChargeBill(CTransportId& transportId) = 0;  //¬озвращ€ет billId
    virtual TransactionStatus CheckBill(int billId, EventMonitor * eventMonitor) = 0; //”станавливает eventMonitor и возвращ€ет статус транзакции
    virtual TransactionStatus GetStatus(int billId) = 0; //¬озвращ€ет статус транзакции

    virtual void commit(int billId) = 0;
    virtual void rollback(int billId) = 0;

    static BillingManager& Instance();
    static void Init(int maxMonitors);




};


}}

#endif
